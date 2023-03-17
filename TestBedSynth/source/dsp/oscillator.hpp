// (c) 2021 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/fftw3/fftw3.h"
#include "modulationenum.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <mutex>
#include <numeric>

namespace SomeDSP {

template<typename T> inline T lerp(T y0, T y1, T t) { return y0 + t * (y1 - y0); }

template<typename T> inline T signedPower(T x, T y)
{
  return std::copysign(std::pow(std::abs(x), y), x);
}

// Range of t is in [0, 1]. Interpolates between y1 and y2.
template<typename T> inline T cubicInterp(T y0, T y1, T y2, T y3, T t)
{
  auto t2 = t * t;
  auto c0 = y1 - y2;
  auto c1 = (y2 - y0) / T(2);
  auto c2 = c0 + c1;
  auto c3 = c0 + c2 + (y3 - y1) / T(2);
  return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y1;
}

/**
Range of t is in [0, 1]. Interpoltes between y1 and y2.
y0 is current, y3 is earlier sample.
*/
template<typename T> inline T lagrange3Interp(T y0, T y1, T y2, T y3, T t)
{
  T u = 1 + t;
  T d0 = y0 - y1;
  T d1 = d0 - (y1 - y2);
  T d2 = d1 - ((y1 - y2) - (y2 - y3));
  return y0 - u * (d0 + (T(1) - u) / T(2) * (d1 + (T(2) - u) / T(3) * d2));
}

struct WavetableParameter {
  float baseNyquistHz = 0;

  ExpSmoother<float> oscPitch;
  ExpSmoother<float> sumMix;
  ExpSmoother<float> feedbackLowpassKp;
  ExpSmoother<float> sumToImmediatePm;
  ExpSmoother<float> sumToAccumulatePm;
  ExpSmoother<float> sumToFm;

  float hardSync = 1;
  float spectralSpread = 1;
  float phaseSlope = 0;
  float spectralLowpass = 1;
  float spectralHighpass = 1;

  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> pitch;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> immedaitePm;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> accumulatePm;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> fm;

  std::array<float, ModID::MODID_ENUM_LENGTH> modHardSync{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modSpectralSpread{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modPhaseSlope{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modSpectralLowpass{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modSpectralHighpass{};
};

inline float mixModulation(
  const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
  const std::array<float, ModID::MODID_ENUM_LENGTH> &amount)
{
  return std::inner_product(mod.begin(), mod.end(), amount.begin(), float(0));
}

template<size_t tableSize> struct WaveForm {
  float *table;

  void init()
  {
    table = (float *)fftwf_malloc(sizeof(float) * tableSize);
    std::fill(table, table + tableSize, 0.0f);
  }

  void free() { fftwf_free(table); }

  void draw(
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    const std::array<float, nOscWavetable> &wavetable,
    WavetableParameter &param)
  {
    auto modHardSync = mixModulation(mod, param.modHardSync);

    auto hardSync = std::exp2(modHardSync) * param.hardSync;

    for (size_t idx = 0; idx < tableSize; ++idx) {
      auto phase = hardSync * float(idx) / float(tableSize);
      phase -= std::floor(phase);

      auto pos = nOscWavetable * phase;
      size_t i0 = size_t(pos);
      size_t i1 = (i0 + 1) & (nOscWavetable - 1);
      table[idx] = lerp(wavetable[i0], wavetable[i1], pos - float(i0));
    }
  }
};

template<size_t tableSize> struct Spectrum {
  static constexpr size_t spectrumSize = tableSize / 2 + 1;
  static constexpr float bendRange = 1.7320508075688772f; // sqrt(3).

  std::complex<float> *src; // source.
  std::complex<float> *dst; // destination.

  void init()
  {
    src = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spectrumSize);
    dst = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spectrumSize);
  }

  void free()
  {
    fftwf_free(src);
    fftwf_free(dst);
  }

  void prepare(
    float noteHz,
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    WavetableParameter &param)
  {
    std::fill(dst, dst + spectrumSize, std::complex<float>{0, 0});

    // Spectral modulation.
    auto modSpectralSpread = mixModulation(mod, param.modSpectralSpread);
    auto modPhaseSlope = mixModulation(mod, param.modPhaseSlope);
    auto modSpectralLowpass = mixModulation(mod, param.modSpectralLowpass);
    auto modSpectralHighpass = mixModulation(mod, param.modSpectralHighpass);

    auto spectralSpread = std::exp2(modSpectralSpread) * param.spectralSpread;
    auto phaseSlope = modPhaseSlope + param.phaseSlope;
    auto spcLpPitch = std::exp2(modSpectralLowpass) * param.spectralLowpass;
    auto spcHpPitch = std::exp2(modSpectralHighpass) * param.spectralHighpass;
    size_t end = std::clamp(size_t(spcLpPitch * spectrumSize), size_t(2), spectrumSize);
    size_t start = std::max(size_t(spcHpPitch * spectrumSize), size_t(1));

    for (size_t idx = start; idx < end; ++idx) {
      auto target = spectralSpread * float(idx);
      size_t index = size_t(target);
      if (index >= spectrumSize) break;

      auto input = src[index] / float(tableSize);

      auto len = std::abs(input);
      auto arg = std::arg(input);

      auto moddedPhase = arg + phaseSlope * len;
      dst[idx].real(len * std::cos(moddedPhase));
      dst[idx].imag(len * std::sin(moddedPhase));

      auto frac = target - float(index);
      auto gain = spectralSpread * std::abs(float(1) - float(2) * frac);
      dst[idx] *= gain;
    }
  }
};

struct VariableWaveTableOscillator {
  // `fftwMutex` is used to lock FFTW3 calls except `fftw*_execute`.
  static std::mutex fftwMutex;

  static constexpr size_t tableSize = 8192;
  static constexpr size_t paddedSize = tableSize + 3; // +1 for linear, +3 for cubic.
  static constexpr size_t spectrumSize = tableSize / 2 + 1;

  WaveForm<tableSize> waveform;
  Spectrum<tableSize> spectrum;

  fftwf_plan forwardPlan;
  std::array<fftwf_plan, 2> inversePlan;

  std::array<float *, 2> table;
  size_t backIndex = 1;

  float phase = 0.0f;
  size_t fadeSamples = 256;
  size_t fadeCounter = 0;

  EMAFilter<float> feedbackLowpass;

  VariableWaveTableOscillator()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    waveform.init();
    spectrum.init();

    forwardPlan = fftwf_plan_dft_r2c_1d(
      tableSize, waveform.table, reinterpret_cast<fftwf_complex *>(spectrum.src),
      FFTW_ESTIMATE);

    for (size_t idx = 0; idx < table.size(); ++idx) {
      table[idx] = (float *)fftwf_malloc(sizeof(float) * paddedSize);
      std::fill(table[idx], table[idx] + paddedSize, 0.0f);

      inversePlan[idx] = fftwf_plan_dft_c2r_1d(
        tableSize, reinterpret_cast<fftwf_complex *>(spectrum.dst), table[idx] + 1,
        FFTW_ESTIMATE);
    }
  }

  ~VariableWaveTableOscillator()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    for (auto &pln : inversePlan) fftwf_destroy_plan(pln);
    for (auto &tbl : table) fftwf_free(tbl);
    fftwf_destroy_plan(forwardPlan);

    spectrum.free();
    waveform.free();
  }

  void reset()
  {
    phase = 0;
    fadeCounter = 0;
    feedbackLowpass.reset();
  }

  void noteOn(
    float note,
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    const std::array<float, nOscWavetable> &wavetable,
    WavetableParameter &param)
  {
    reset();
    for (size_t idx = 0; idx < table.size(); ++idx) {
      refreshTable(idx, note, mod, wavetable, param);
    }
  }

  void setup(float sampleRate)
  {
    fadeSamples = size_t(0.007f * sampleRate);
    fadeCounter = fadeSamples;
  }

  void refreshTable(
    size_t tableIndex,
    float noteHz,
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    const std::array<float, nOscWavetable> &wavetable,
    WavetableParameter &param)
  {
    waveform.draw(mod, wavetable, param);
    fftwf_execute(forwardPlan);
    spectrum.prepare(noteHz, mod, param);
    fftwf_execute(inversePlan[tableIndex]);

    table[tableIndex][0] = table[tableIndex][tableSize];
    table[tableIndex][paddedSize - 2] = table[tableIndex][1];
    table[tableIndex][paddedSize - 1] = table[tableIndex][2];
  }

  float process(
    float sampleRate,
    float noteHz,
    const std::array<float, nOscillator> &feedback,
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    const std::array<float, nOscWavetable> &wavetable,
    WavetableParameter &param)
  {
    noteHz *= param.oscPitch.getValue();

    --fadeCounter;
    if (fadeCounter >= fadeSamples) { // Unsigned negative overflow case.
      fadeCounter = fadeSamples;
      backIndex ^= 1;

      refreshTable(backIndex, noteHz, mod, wavetable, param);
    }

    const auto oscSum = feedbackLowpass.processKp(
      lerp(feedback[0], feedback[1], param.sumMix.getValue()),
      param.feedbackLowpassKp.getValue());
    const auto modPitch = mixModulation(mod, param.pitch.value);
    const auto immediatePm
      = mixModulation(mod, param.immedaitePm.value) + param.sumToImmediatePm.getValue();
    const auto accumulatePm
      = mixModulation(mod, param.accumulatePm.value) + param.sumToAccumulatePm.getValue();
    const auto fm = mixModulation(mod, param.fm.value) + param.sumToFm.getValue();

    phase += accumulatePm * oscSum
      // + std::fmod(std::exp2(fm * oscSum + modPitch) * noteHz / sampleRate, float(0.5));
      + std::min(std::exp2(fm * oscSum + modPitch) * noteHz / sampleRate, float(0.5));
    phase -= std::floor(phase);
    auto phs = phase + immediatePm * oscSum;
    phs -= std::floor(phs);

    float pos = float(tableSize) * phs;
    size_t idx = size_t(pos);
    float fraction = pos - float(idx);

    float vf = lagrange3Interp(
      table[0][idx], table[0][idx + 1], table[0][idx + 2], table[0][idx + 3], fraction);
    float vb = lagrange3Interp(
      table[1][idx], table[1][idx + 1], table[1][idx + 2], table[1][idx + 3], fraction);
    if (backIndex ^ 1) std::swap(vf, vb);

    float fade
      = 0.5f + 0.5f * std::cos(float(pi) * float(fadeCounter) / float(fadeSamples));
    return vf + fade * (vb - vf);
  }
};

//// TODO: Remove this if template is not required.
// template<size_t nModulation>
// std::mutex VariableWaveTableOscillator<nModulation>::fftwMutex;

} // namespace SomeDSP
