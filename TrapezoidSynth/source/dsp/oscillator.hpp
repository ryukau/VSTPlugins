// (c) 2019 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>

#include "../../../lib/juce_FastMathApproximations.h"
#include "constants.hpp"
#include "noise.hpp"
#include "somemath.hpp"

namespace SomeDSP {

enum class LFOType { sin, saw, pulse, noise };

template<typename Sample> class LFO {
public:
  const static int32_t brownSeed = 871341;

  LFOType type = LFOType::sin;
  Sample sampleRate = 44100;
  Sample phaseRad = 0;
  Sample tickRad = 0;
  Sample phaseNorm = 0;
  Sample tickNorm = 0;
  Sample pw = 0; // [0.0, 1.0]
  Brown<Sample> brown{brownSeed};

  void setup(Sample sampleRate) { this->sampleRate = sampleRate; }

  void setFreq(Sample hz)
  {
    tickNorm = hz / sampleRate;
    tickRad = Sample(twopi) * tickNorm;
  }

  void reset()
  {
    phaseRad = 0;
    phaseNorm = 0;
    brown.seed = brownSeed;
  }

  inline Sample sin()
  {
    phaseRad += tickRad;
    if (phaseRad > Sample(pi)) phaseRad -= Sample(twopi);

    auto sign = (Sample(pi) < phaseRad) - (phaseRad < Sample(pi));
    auto sinSig = somefabs<Sample>(juce::dsp::FastMathApproximations::sin(phaseRad));
    return Sample(0.5) + Sample(0.5) * sign * somepow<Sample>(sinSig, Sample(2) * pw);
  }

  inline Sample saw()
  {
    phaseNorm += tickNorm;
    if (phaseNorm >= pw) phaseNorm -= Sample(1);

    return phaseNorm < 0 ? -phaseNorm / (Sample(1) - pw) : phaseNorm / pw;
  }

  inline Sample pulse()
  {
    phaseNorm += tickNorm;
    if (phaseNorm >= pw) phaseNorm -= Sample(1);

    return phaseNorm < 0 ? Sample(1) : Sample(0);
  }

  Sample process()
  {
    switch (type) {
      case LFOType::saw:
        return saw();

      case LFOType::pulse:
        return pulse();

      case LFOType::noise:
        brown.drift = pw;
        return brown.process();

      default:
      case LFOType::sin:
        break;
    }
    return sin();
  }
};

class PTRTrapezoidOsc {
public:
  PTRTrapezoidOsc(float sampleRate, float frequencyHz) : sampleRate(sampleRate)
  {
    setFreq(frequencyHz);
  }

  void setFreq(float hz)
  {
    if (hz >= 0) tick = hz / sampleRate;
  }

  void setPhase(float phase) { this->phase = phase; }
  void addPhase(float phase) { this->phase += phase; }
  void setSlope(float slope) { this->slope = slope; }
  void setPulseWidth(float pw) { this->pw = pw; }
  void reset() { phase = 0; }

  float process()
  {
    if (tick <= 0) return 0;
    phase += tick;
    phase -= somefloor<float>(phase);
    return ptrTpz5(phase, tick, slope, pw);
  }

  float sampleRate;
  float phase = 0;
  float tick = 0;
  float slope = 8;
  float pw = float(0.5); // Pulse width.

protected:
  // tick must be greater than 0.
  static float ptrTpz5(float phase, float tick, float slope, float pw)
  {
    uint32_t order = 5;
    if (order > float(0.25) / tick) order = int(float(0.25) / tick);

    const float ptrLen = order * tick;

    const float maxSlope = float(0.25) / ptrLen;
    if (slope > maxSlope) {
      slope = maxSlope;
    } else {
      const float minSlope = float(1);
      if (slope < minSlope) slope = minSlope;
    }

    const float maxPw = float(1) - float(1) / slope;
    if (pw > maxPw) pw = std::max<float>(float(0), maxPw);

    const float y = float(1) - float(2) * slope * ptrLen;
    const float dc = (y * y + pw * slope * y) / (float(2) * y + slope - float(1));
    if (order == 5)
      return branch<PTRTrapezoidOsc::ptrRamp5>(slope, pw, y, phase, tick) - dc;
    else if (order == 4)
      return branch<PTRTrapezoidOsc::ptrRamp4>(slope, pw, y, phase, tick) - dc;
    else if (order == 3)
      return branch<PTRTrapezoidOsc::ptrRamp3>(slope, pw, y, phase, tick) - dc;
    return branch<PTRTrapezoidOsc::ptrRamp2>(slope, pw, y, phase, tick) - dc;
  }

  template<float (*ptrfunc)(float, float)>
  static float branch(float slope, float pw, float y, float phase, float tick)
  {
    if (phase <= float(0.25) / slope)
      return slope * ptrfunc(phase, tick);
    else if (phase <= float(0.5) / slope)
      return y - slope * ptrfunc(float(0.5) / slope - phase, tick);
    else if (phase <= float(0.5) / slope + pw)
      return y;
    else if (phase <= float(0.75) / slope + pw)
      return y - slope * ptrfunc(phase - float(0.5) / slope - pw, tick);
    else if (phase <= float(1) / slope + pw)
      return slope * ptrfunc(float(1) / slope + pw + -phase, tick);
    return float(0);
  }

  static float ptrRamp0(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(-1)) return float(2) * T * n - float(1);
    return 0.0; // Just in case.
  }

  static float ptrRamp1(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(0)) return float(2) * T * n - T;
    if (n < float(1)) return n * n + T;
    return 0.0; // Just in case.
  }

  static float ptrRamp2(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(1)) return float(2) * T * n - float(2) * T;
    if (n < float(1)) return (T * n * n * n) / float(3);
    if (n < float(2))
      return -(T * n * n * n) / float(3) + float(2) * T * n * n - float(2) * T * n
        + (float(2) * T) / float(3);
    return 0.0; // Just in case.
  }

  static float ptrRamp3(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(2)) return float(2) * T * n - float(3) * T;
    if (n < float(1)) return (T * n * n * n * n) / float(12);
    if (n < float(2))
      return -(T * n * n * n * n) / float(6) + T * n * n * n
        - (float(3) * T * n * n) / float(2) + T * n - T / float(4);
    if (n < float(3))
      return (T * n * n * n * n) / float(12) - T * n * n * n
        + (float(9) * T * n * n) / float(2) - float(7) * T * n
        + (float(15) * T) / float(4);
    return 0.0; // Just in case.
  }

  static float ptrRamp4(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(3)) return float(2) * T * n - float(4) * T;
    if (n < float(1)) return (T * n * n * n * n * n) / float(60);
    if (n < float(2))
      return -(T * n * n * n * n * n) / float(20) + (T * n * n * n * n) / float(3)
        - (float(2) * T * n * n * n) / float(3) + (float(2) * T * n * n) / float(3)
        - (T * n) / float(3) + T / float(15);
    if (n < float(3))
      return (T * n * n * n * n * n) / float(20)
        - (float(2) * T * n * n * n * n) / float(3)
        + (float(10) * T * n * n * n) / float(3) - (float(22) * T * n * n) / float(3)
        + (float(23) * T * n) / float(3) - (float(47) * T) / float(15);
    if (n < float(4))
      return -(T * n * n * n * n * n) / float(60) + (T * n * n * n * n) / float(3)
        - (float(8) * T * n * n * n) / float(3) + (float(32) * T * n * n) / float(3)
        - (float(58) * T * n) / float(3) + (float(196) * T) / float(15);
    return 0.0; // Just in case.
  }

  static float ptrRamp5(float phi, float T)
  {
    float n = phi / T;
    if (n >= float(4)) return float(2) * T * n - float(5) * T;
    if (n < float(1)) return (T * n * n * n * n * n * n) / float(360);
    if (n < float(2))
      return -(T * n * n * n * n * n * n) / float(90)
        + (T * n * n * n * n * n) / float(12) - (float(5) * T * n * n * n * n) / float(24)
        + (float(5) * T * n * n * n) / float(18) - (float(5) * T * n * n) / float(24)
        + (T * n) / float(12) - T / float(72);
    if (n < float(3))
      return (T * n * n * n * n * n * n) / float(60) - (T * n * n * n * n * n) / float(4)
        + (float(35) * T * n * n * n * n) / float(24)
        - (float(25) * T * n * n * n) / float(6) + (float(155) * T * n * n) / float(24)
        - (float(21) * T * n) / float(4) + (float(127) * T) / float(72);
    if (n < float(4))
      return -(T * n * n * n * n * n * n) / float(90) + (T * n * n * n * n * n) / float(4)
        - (float(55) * T * n * n * n * n) / float(24)
        + (float(65) * T * n * n * n) / float(6) - (float(655) * T * n * n) / float(24)
        + (float(141) * T * n) / float(4) - (float(1331) * T) / float(72);
    if (n < float(5))
      return (T * n * n * n * n * n * n) / float(360)
        - (T * n * n * n * n * n) / float(12)
        + (float(25) * T * n * n * n * n) / float(24)
        - (float(125) * T * n * n * n) / float(18) + (float(625) * T * n * n) / float(24)
        - (float(601) * T * n) / float(12) + (float(2765) * T) / float(72);
    return 0.0; // Just in case.
  }
};

} // namespace SomeDSP
