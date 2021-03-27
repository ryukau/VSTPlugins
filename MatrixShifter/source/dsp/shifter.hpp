// (c) 2019-2020 Takamitsu Endo
//
// This file is part of MatrixShifter.
//
// MatrixShifter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixShifter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixShifter.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"

#include <array>

namespace SomeDSP {

/**
Translation of SVF in Faust filter.lib.
https://faustlibraries.grame.fr/libs/filters/#svf-filters

Arguments of `process`.
- fs: Sampling rate.
- v0: Input.
- F: Normalized frequency. In theory, range is in [0, 1].
- Q: Quality factor, or resonance.
- G: Gain for bell and shelving filters.

List of `type`.
- 0: LP
- 1: BP
- 2: HP
- 3: Notch
- 4: Peak
- 5: AP
- 6: Bell
- 7: Low-shelf
- 8: High-shelf
*/
template<typename Sample, size_t type> class SVF {
private:
  Sample ic1eq = Sample(0);
  Sample ic2eq = Sample(0);

public:
  SVF() { static_assert(type <= 8, "SVF type must be less than or equal to 8."); }

  void reset() { ic1eq = ic2eq = Sample(0); }

  Sample process(Sample fs, Sample v0, Sample F, Sample Q, Sample G = Sample(0))
  {
    auto A = Sample(1);
    if constexpr (type >= 6) A = std::pow(Sample(10), G / Sample(40));

    auto g = std::tan(F * Sample(pi) / fs);
    if constexpr (type == 7) {
      g /= std::sqrt(A);
    } else if (type == 8) {
      g *= std::sqrt(A);
    }

    auto k = Sample(1) / Q;
    if constexpr (type == 6) k /= A;

    // tick.
    auto v1 = (ic1eq + g * (v0 - ic2eq)) / (Sample(1) + g * (g + k));
    auto v2 = ic2eq + g * v1;

    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;

    // Mix.
    if constexpr (type == 0) {
      return v2;
    } else if (type == 1) {
      return v1;
    } else if (type == 2) {
      return v0 - k * v1 - v2;
    } else if (type == 3) {
      return v0 - k * v1;
    } else if (type == 4) {
      return v0 - k * v1 - Sample(2) * v2;
    } else if (type == 5) {
      return v0 - Sample(2) * k * v1;
    } else if (type == 6) {
      return v0 + v1 * k * (A * A - Sample(1));
    } else if (type == 7) {
      return v0 + v1 * k * (A - Sample(1)) + v2 * (A * A - Sample(1));
    } else if (type == 8) {
      return v0 * A * A + v1 * k * (A - Sample(1)) * A + v2 * (Sample(1) - A * A);
    }
    return Sample(0); // Shouldn't reach here.
  }
};

template<typename Sample> class LFO {
public:
  Sample phase = 0;

  void reset() { phase = 0; }

  Sample process(Sample sampleRate, Sample hz, Sample skew, Sample phaseOffset)
  {
    phase += hz / sampleRate;
    phase -= std::floor(phase);
    const auto ph = phaseOffset + phase - skew * (phase * phase - phase);
    return 0.5f + 0.5f * std::sin(Sample(twopi) * ph);
  }
};

// 2x oversampled delay.
template<typename Sample> class Delay {
public:
  Sample w1 = 0;
  Sample rFraction = 0.0;
  size_t wptr = 0;
  size_t rptr = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample maxTime)
  {
    auto size = size_t(Sample(2) * sampleRate * maxTime) + 1;
    if (size < 4) size = 4;
    buf.resize(size);

    reset();
  }

  void reset()
  {
    w1 = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  Sample process(Sample sampleRate, Sample input, Sample seconds)
  {
    // Set delay time.
    Sample timeInSample = std::clamp<Sample>(
      Sample(2) * sampleRate * seconds, Sample(0), Sample(buf.size() - 1));

    size_t timeInt = size_t(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr >= buf.size()) rptr += buf.size(); // Unsigned negative overflow case.

    // Write to buffer.
    buf[wptr] = Sample(0.5) * (input + w1);
    if (++wptr >= buf.size()) wptr -= buf.size();

    buf[wptr] = input;
    if (++wptr >= buf.size()) wptr -= buf.size();

    w1 = input;

    // Read from buffer.
    const size_t i1 = rptr;
    if (++rptr >= buf.size()) rptr -= buf.size();

    const size_t i0 = rptr;
    if (++rptr >= buf.size()) rptr -= buf.size();

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

template<typename Sample, size_t nParallel> class AMPitchShiter {
public:
  void reset()
  {
    phase.fill(0);
    x0.fill(0);
    x1.fill(0);
    x2.fill(0);
    y0.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  // Note: output may exceed the amplitude of input.
  Sample process(
    Sample sampleRate,
    Sample input,
    Sample phaseOffset,
    std::array<Sample, nParallel> &shiftHz)
  {
    x0[7] = x0[6];
    x0[6] = x0[5];
    x0[5] = x0[4];
    x0[4] = input;
    x0[3] = x0[2];
    x0[2] = x0[1];
    x0[1] = x0[0];
    x0[0] = input;

    for (size_t i = 0; i < y0.size(); ++i) y0[i] = co[i] * (x0[i] + y2[i]) - x2[i];

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    const auto re = y0[3];
    const auto norm = std::sqrt(re * re + im * im);
    const auto theta = std::atan2(im, re);
    im = y0[7]; // 1 sample delay.

    Sample output = 0;
    for (size_t idx = 0; idx < nParallel; ++idx) {
      // TODO: Add parameter to modify cosine.
      output += norm * std::cos(theta + Sample(twopi) * (phase[idx] + phaseOffset));

      phase[idx] += std::clamp<Sample>(shiftHz[idx] / sampleRate, 0, 1);
      phase[idx] -= std::floor(phase[idx]);
    }

    return output / Sample(nParallel);
  }

private:
  constexpr static std::array<Sample, 8> co{
    Sample(0.16175849836770106), Sample(0.7330289323414905), Sample(0.9453497003291133),
    Sample(0.9905991566845292),  Sample(0.47940086558884),   Sample(0.8762184935393101),
    Sample(0.9765975895081993),  Sample(0.9974992559355491)};

  std::array<Sample, nParallel> phase{};
  Sample im = 0.0;

  std::array<Sample, 8> x0{};
  std::array<Sample, 8> x1{};
  std::array<Sample, 8> x2{};
  std::array<Sample, 8> y0{};
  std::array<Sample, 8> y1{};
  std::array<Sample, 8> y2{};
};

template<typename Sample, size_t nParallel> class AMPitchShiterFixed {
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

  std::array<Sample, nParallel> phase{};
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

    phase.fill(0);
    delayedIm = 0;
  }

  // Note: output may exceed the amplitude of input.
  Sample process(
    Sample sampleRate,
    const Sample input,
    Sample phaseOffset,
    std::array<Sample, nParallel> &shiftHz)
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

    Sample output = 0;
    for (size_t idx = 0; idx < nParallel; ++idx) {
      output += norm * std::cos(theta + Sample(twopi) * (phase[idx] + phaseOffset));

      phase[idx] += std::clamp<Sample>(shiftHz[idx] / sampleRate, 0, 1);
      phase[idx] -= std::floor(phase[idx]);
    }

    return output / Sample(nParallel);
  }
};

template<typename Sample, size_t nParallel, size_t nSerial> class MultiShifter {
public:
  std::array<AMPitchShiterFixed<Sample, nParallel>, nSerial> shifter;
  std::array<Delay<Sample>, nSerial> delay;
  SVF<Sample, 7> svf;
  Sample buf = 0;

  std::array<std::array<Sample, nParallel>, nSerial> hz;
  std::array<Sample, nSerial> seconds{};
  std::array<Sample, nSerial> gain{};
  Sample bypassGain = 0;

  void setup(Sample sampleRate, Sample maxSeconds)
  {
    for (auto &dly : delay) dly.setup(sampleRate, maxSeconds);
  }

  void reset()
  {
    for (auto &shf : shifter) shf.reset();
    for (auto &dly : delay) dly.reset();
    svf.reset();
    buf = 0;
  }

  Sample process(
    Sample sampleRate,
    Sample input,
    Sample phaseOffset,
    Sample feedbackGain,
    Sample feedbackCutoffHz,
    Sample sectionGain)
  {
    Sample output = bypassGain * input + (feedbackGain * buf);
    for (size_t idx = 0; idx < nSerial; ++idx) {
      input = sectionGain * shifter[idx].process(sampleRate, input, phaseOffset, hz[idx]);
      input = delay[idx].process(sampleRate, input, seconds[idx]);
      output += gain[idx] * input;
    }
    buf = svf.process(sampleRate, output, feedbackCutoffHz, Sample(0.1), Sample(-3));
    return output / Sample(nSerial);
  }
};

} // namespace SomeDSP
