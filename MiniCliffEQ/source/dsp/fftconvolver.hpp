// (c) 2022 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/fftw3/fftw3.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <numeric>

namespace SomeDSP {

template<size_t nTap> class FFTConvolver {
private:
  static constexpr size_t half = nTap;
  static constexpr size_t bufSize = 2 * half;
  static constexpr size_t spcSize = nTap + 1; // spc = spectrum.

  float *buf;
  std::complex<float> *spc;

  float *coefficient;
  std::complex<float> *fir;

  std::array<float *, 2> flt; // filtered.

  fftwf_plan firPlan;
  fftwf_plan forwardPlan;
  std::array<fftwf_plan, 2> inversePlan;

  size_t front = 0;
  size_t wptr = 0;
  std::array<size_t, 2> rptr{};

public:
  FFTConvolver()
  {
    static_assert(
      nTap && ((nTap & (nTap - 1)) == 0), "FFTConvolver: nTap must be power of 2.");

    buf = (float *)fftwf_malloc(sizeof(float) * bufSize);
    spc = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    for (size_t idx = 0; idx < flt.size(); ++idx) {
      flt[idx] = (float *)fftwf_malloc(sizeof(float) * bufSize);
    }

    coefficient = (float *)fftwf_malloc(sizeof(float) * bufSize);
    std::fill(coefficient, coefficient + bufSize, float(0));

    fir = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spcSize);
    firPlan = fftwf_plan_dft_r2c_1d(
      bufSize, coefficient, reinterpret_cast<fftwf_complex *>(fir), FFTW_ESTIMATE);

    forwardPlan = fftwf_plan_dft_r2c_1d(
      bufSize, buf, reinterpret_cast<fftwf_complex *>(spc), FFTW_ESTIMATE);
    for (size_t idx = 0; idx < inversePlan.size(); ++idx) {
      inversePlan[idx] = fftwf_plan_dft_c2r_1d(
        bufSize, reinterpret_cast<fftwf_complex *>(spc), flt[idx], FFTW_ESTIMATE);
    }

    reset();
  }

  ~FFTConvolver()
  {
    fftwf_destroy_plan(firPlan);
    fftwf_destroy_plan(forwardPlan);
    for (auto &ip : inversePlan) fftwf_destroy_plan(ip);

    fftwf_free(buf);
    fftwf_free(spc);

    fftwf_free(coefficient);
    fftwf_free(fir);

    for (auto &fl : flt) fftwf_free(fl);
  }

  inline size_t latency()
  {
    // `(half - 1)` is the latency of FFT buffering.
    // `(half / 2 - 1)` is the latency of FIR filter specific to `fillHighpass()`.
    return (half - 1) + (half / 2 - 1);
  }

  void refreshFir(float sampleRate, float cutoffHz, bool isHighpass)
  {
    const auto nyquist = sampleRate / float(2);
    if (cutoffHz > nyquist) cutoffHz = nyquist;

    bool isEven = (half / 2 & 1) == 0;
    size_t end = half;
    if (isEven) --end; // Always use odd length FIR.

    auto mid = float(end - 1) / float(2);
    auto cutoff = float(twopi) * cutoffHz / sampleRate;
    for (size_t idx = 0; idx < end; ++idx) {
      float m = float(idx) - mid;
      float x = cutoff * m;
      coefficient[idx] = x == 0 ? float(1) : std::sin(x) / (x);
    }

    // Apply Nuttall window.
    float tpN = float(twopi) / float(end - 1);
    for (size_t n = 0; n < end; ++n) {
      auto c0 = float(0.3635819);
      auto c1 = float(0.4891775) * std::cos(tpN * n);
      auto c2 = float(0.1365995) * std::cos(tpN * n * float(2));
      auto c3 = float(0.0106411) * std::cos(tpN * n * float(3));
      coefficient[n] *= c0 - c1 + c2 - c3;
    }

    // Normalize to fix FIR scaling.
    float sum = std::accumulate(coefficient, coefficient + half, float(0));
    for (size_t idx = 0; idx < end; ++idx) coefficient[idx] /= sum;

    if (isHighpass) {
      for (size_t idx = 0; idx < end; ++idx) coefficient[idx] = -coefficient[idx];
      coefficient[size_t(mid)] += float(1);
    }

    // Normalize for FFT scaling.
    for (size_t idx = 0; idx < end; ++idx) coefficient[idx] /= float(bufSize);

    fftwf_execute(firPlan);
    reset();
  }

  void reset()
  {
    front = 0;
    wptr = 0;
    rptr[0] = 1;
    rptr[1] = 1 + half;

    std::fill(buf, buf + bufSize, float(0));
    std::fill(spc, spc + spcSize, std::complex<float>(0, 0));
    for (size_t idx = 0; idx < flt.size(); ++idx) {
      std::fill(flt[idx], flt[idx] + bufSize, float(0));
    }
  }

  float process(float input)
  {
    buf[wptr] = input;

    if (++wptr >= half) {
      wptr = 0;
      front ^= 1;

      fftwf_execute(forwardPlan);
      for (size_t i = 0; i < spcSize; ++i) spc[i] *= fir[i];
      fftwf_execute(inversePlan[front]);
    }

    float output = flt[0][rptr[0]] + flt[1][rptr[1]];
    for (auto &r : rptr) {
      if (++r >= bufSize) r = 0;
    }
    return output;
  }
};

} // namespace SomeDSP
