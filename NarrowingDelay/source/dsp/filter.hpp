// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample> class AMFrequencyShifter {
private:
  constexpr static std::array<Sample, 4> coRe{
    Sample(0.16175849836770106), Sample(0.7330289323414905), Sample(0.9453497003291133),
    Sample(0.9905991566845292)};
  constexpr static std::array<Sample, 4> coIm{
    Sample(0.47940086558884), Sample(0.8762184935393101), Sample(0.9765975895081993),
    Sample(0.9974992559355491)};

  std::array<Sample, coRe.size()> x1Re{};
  std::array<Sample, coRe.size()> x2Re{};
  std::array<Sample, coRe.size()> y1Re{};
  std::array<Sample, coRe.size()> y2Re{};

  std::array<Sample, coIm.size()> x1Im{};
  std::array<Sample, coIm.size()> x2Im{};
  std::array<Sample, coIm.size()> y1Im{};
  std::array<Sample, coIm.size()> y2Im{};

  Sample phase = 0;
  Sample delayedIm = 0;

public:
  void reset()
  {
    x1Re.fill(0);
    x2Re.fill(0);
    y1Re.fill(0);
    y2Re.fill(0);

    x1Im.fill(0);
    x2Im.fill(0);
    y1Im.fill(0);
    y2Im.fill(0);

    phase = 0;
    delayedIm = 0;
  }

  //
  // `shiftFreqeuncy` is normalized frequency in [0, 0.5). Negative and beyond Nyquist
  // frequency can be used.
  //
  // Note: output may exceed the amplitude of input.
  //
  Sample process(const Sample input, Sample shiftFreqeuncy)
  {
    auto sigRe = input;
    for (size_t i = 0; i < coRe.size(); ++i) {
      auto y0 = coRe[i] * (sigRe + y2Re[i]) - x2Re[i];
      x2Re[i] = x1Re[i];
      x1Re[i] = sigRe;
      y2Re[i] = y1Re[i];
      y1Re[i] = y0;
      sigRe = y0;
    }

    auto sigIm = input;
    for (size_t i = 0; i < coIm.size(); ++i) {
      auto y0 = coIm[i] * (sigIm + y2Im[i]) - x2Im[i];
      x2Im[i] = x1Im[i];
      x1Im[i] = sigIm;
      y2Im[i] = y1Im[i];
      y1Im[i] = y0;
      sigIm = y0;
    }

    const auto norm = std::sqrt(sigRe * sigRe + delayedIm * delayedIm);
    const auto theta = std::atan2(delayedIm, sigRe);
    delayedIm = sigIm; // 1 sample delay.

    Sample output = norm * std::cos(theta + Sample(twopi) * phase);

    phase += shiftFreqeuncy;
    phase -= std::floor(phase);

    return output;
  }
};

template<typename Sample> class SVF {
private:
  static constexpr Sample minCutoff = Sample(0.00001);
  static constexpr Sample nyquist = Sample(0.49998);

  Sample ic1eq = 0;
  Sample ic2eq = 0;

  ExpSmoother<Sample> g;
  ExpSmoother<Sample> k;

public:
  void pushCutoff(Sample normalizedFreq, Sample Q)
  {
    g.push(std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k.push(Sample(1) / Q);
  }

  void resetCutoff(Sample normalizedFreq, Sample Q)
  {
    g.reset(std::tan(std::clamp(normalizedFreq, minCutoff, nyquist) * Sample(pi)));
    k.reset(Sample(1) / Q);
  }

  void reset()
  {
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample lowpass(Sample v0)
  {
    auto gn = g.process();
    auto kn = k.process();
    auto v1 = (ic1eq + gn * (v0 - ic2eq)) / (Sample(1) + gn * (gn + kn));
    auto v2 = ic2eq + gn * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v2;
  }

  Sample highpass(Sample v0)
  {
    auto gn = g.process();
    auto kn = k.process();
    auto v1 = (ic1eq + gn * (v0 - ic2eq)) / (Sample(1) + gn * (gn + kn));
    auto v2 = ic2eq + gn * v1;
    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;
    return v0 - kn * v1 - v2;
  }
};

template<typename Sample> class PitchShiftDelay {
private:
  static constexpr size_t minSize = 4;

  size_t wptr = 0;
  Sample phase = 0;
  std::vector<Sample> buf;

public:
  PitchShiftDelay() : buf(minSize) {}

  // bufferSize must be less than 2^24 for single precision float.
  void setup(size_t bufferSize)
  {
    buf.resize(bufferSize < minSize ? minSize : bufferSize);
    reset();
  }

  void reset()
  {
    wptr = 0;
    phase = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  void syncPhase(Sample target, Sample kp)
  {
    auto d1 = target - phase;
    if (d1 < 0) {
      auto d2 = d1 + Sample(1);
      phase += kp * (d2 < -d1 ? d2 : d1);
    } else {
      auto d2 = d1 - Sample(1);
      phase += kp * (-d2 < d1 ? d2 : d1);
    }
  }

  // `pitch` is a multiplier relative to input pitch. It can be negative number.
  Sample process(Sample input, Sample pitch, Sample timeInSample)
  {
    // Write to buffer.
    buf[wptr] = input;
    if (++wptr >= buf.size()) wptr -= buf.size();

    // Read from buffer.
    Sample bufSize = Sample(buf.size());

    auto delayTime = std::clamp(
      timeInSample, std::numeric_limits<Sample>::epsilon(), Sample(buf.size() - 1));

    phase -= (pitch - Sample(1)) / delayTime;
    phase -= std::floor(phase);
    auto rptr0 = std::fmod(Sample(wptr) - delayTime * phase, bufSize);
    if (rptr0 < 0) rptr0 += bufSize;

    auto ph1 = phase + Sample(0.5);
    ph1 -= std::floor(ph1);
    auto rptr1 = std::fmod(Sample(wptr) - delayTime * ph1, bufSize);
    if (rptr1 < 0) rptr1 += bufSize;

    auto i0 = size_t(rptr0);
    auto i1 = size_t(rptr1);

    size_t j0 = i0 - 1;
    size_t j1 = i1 - 1;

    if (j0 >= buf.size()) j0 += buf.size(); // Unsigned negative overflow case.
    if (j1 >= buf.size()) j1 += buf.size(); // Unsigned negative overflow case.

    auto frac0 = rptr0 - Sample(i0);
    auto frac1 = rptr1 - Sample(i1);

    auto v0 = buf[j0] + frac0 * (buf[i0] - buf[j0]);
    auto v1 = buf[j1] + frac1 * (buf[i1] - buf[j1]);

    auto amp = Sample(2) * (phase <= Sample(0.5) ? phase : Sample(1) - phase);

    return v1 + amp * (v0 - v1);
  }
};

} // namespace SomeDSP
