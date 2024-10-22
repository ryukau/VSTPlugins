// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample, size_t length> class ParallelSVF {
private:
  std::array<Sample, length> ic1eq{};
  std::array<Sample, length> ic2eq{};

  static constexpr Sample k
    = Sample(1) / Sample(halfSqrt2 - std::numeric_limits<Sample>::epsilon());

public:
  void reset()
  {
    ic1eq.fill({});
    ic2eq.fill({});
  }

  void lowpass(std::array<Sample, length> &v0, std::array<Sample, length> &g)
  {
    for (size_t n = 0; n < length; ++n) {
      auto v1 = (ic1eq[n] + g[n] * (v0[n] - ic2eq[n])) / (Sample(1) + g[n] * (g[n] + k));
      auto v2 = ic2eq[n] + g[n] * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] = v2;
    }
  }

  void highpass(std::array<Sample, length> &v0, std::array<Sample, length> &g)
  {
    for (size_t n = 0; n < length; ++n) {
      auto v1 = (ic1eq[n] + g[n] * (v0[n] - ic2eq[n])) / (Sample(1) + g[n] * (g[n] + k));
      auto v2 = ic2eq[n] + g[n] * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] -= k * v1 + v2;
    }
  }
};

template<typename Sample, size_t length> class ParallelPitchShiftDelay {
private:
  static constexpr size_t minSize = 4;

  std::array<size_t, length> wptr{};
  std::array<Sample, length> phase{};
  std::array<std::vector<Sample>, length> buffer;

public:
  ParallelPitchShiftDelay()
  {
    for (auto &bf : buffer) bf.resize(minSize);
  }

  std::array<Sample, length> &getPhase() { return phase; }

  // bufferSize must be less than 2^24 for single precision float.
  void setup(size_t bufferSize)
  {
    for (auto &bf : buffer) bf.resize(bufferSize < minSize ? minSize : bufferSize);
    reset();
  }

  void reset()
  {
    wptr.fill(0);
    phase.fill({});
    for (auto &bf : buffer) std::fill(bf.begin(), bf.end(), Sample(0));
  }

  void syncPhase(const std::array<Sample, length> &target, Sample kp)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto d1 = target[idx] - phase[idx];
      if (d1 < 0) {
        auto d2 = d1 + Sample(1);
        phase[idx] += kp * (d2 < -d1 ? d2 : d1);
      } else {
        auto d2 = d1 - Sample(1);
        phase[idx] += kp * (-d2 < d1 ? d2 : d1);
      }
    }
  }

  // `pitch` is a multiplier relative to input pitch. It can be negative number.
  void process(
    std::array<Sample, length> &input,
    std::array<Sample, length> &pitch,
    std::array<Sample, length> &timeInSample,
    Sample modPitch,
    Sample modTime)
  {
    for (size_t idx = 0; idx < length; ++idx) {
      auto &buf = buffer[idx];
      auto &phi = phase[idx];

      // Write to buffer.
      buf[wptr[idx]] = input[idx];
      if (++wptr[idx] >= buf.size()) wptr[idx] -= buf.size();

      // Read from buffer.
      Sample bufSize = Sample(buf.size());

      auto delayTime = std::clamp(
        modTime * timeInSample[idx], std::numeric_limits<Sample>::epsilon(),
        Sample(buf.size() - 1));

      phi -= (modPitch * pitch[idx] - Sample(1)) / delayTime;
      phi -= std::floor(phi);
      auto rptr0 = std::fmod(Sample(wptr[idx]) - delayTime * phi, bufSize);
      if (rptr0 < 0) rptr0 += bufSize;

      auto ph1 = phi + Sample(0.5);
      ph1 -= std::floor(ph1);
      auto rptr1 = std::fmod(Sample(wptr[idx]) - delayTime * ph1, bufSize);
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

      auto amp = Sample(2) * (phi <= Sample(0.5) ? phi : Sample(1) - phi);

      input[idx] = v1 + amp * (v0 - v1);
    }
  }
};

} // namespace SomeDSP
