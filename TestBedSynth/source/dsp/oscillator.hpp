// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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

// `fftwMutex` is used to lock FFTW3 calls except `fftw*_execute`. In other words, FFTW3
// isn't thread safe except `fftw*_execute` call.
extern std::mutex fftwMutex;

// Range of t is in [0, 1]. Interpoltes between y1 and y2.
// y0 is current, y3 is earlier sample.
template<typename T> inline T lagrange3Interp(T y0, T y1, T y2, T y3, T t)
{
  T u = 1 + t;
  T d0 = y0 - y1;
  T d1 = d0 - (y1 - y2);
  T d2 = d1 - ((y1 - y2) - (y2 - y3));
  return y0 - u * (d0 + (T(1) - u) / T(2) * (d1 + (T(2) - u) / T(3) * d2));
}

struct WavetableParameter {
  float baseNyquistHz = 20000;
  float upNyquistHz = 20000;

  size_t waveInterpType = 2; // 0: Step, 1: Linear, 2: Cubic.

  ExpSmoother<float> oscPitch;
  ExpSmoother<float> sumMix;
  ExpSmoother<float> feedbackLowpassKp;
  ExpSmoother<float> sumToImmediatePm;
  ExpSmoother<float> sumToAccumulatePm;
  ExpSmoother<float> sumToFm;
  ExpSmoother<float> sumToAm;

  float hardSync = 1;
  float phaseSkew = 0;
  float distortion = 0;
  float spectralSpread = 1;
  float phaseSlope = 0;
  float spectralLowpass = 1;
  float spectralHighpass = 1;

  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> pitch;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> immedaitePm;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> accumulatePm;
  ParallelExpSmoother<float, ModID::MODID_ENUM_LENGTH> fm;

  std::array<float, ModID::MODID_ENUM_LENGTH> modHardSync{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modPhaseSkew{};
  std::array<float, ModID::MODID_ENUM_LENGTH> modDistortion{};
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
  float *table = nullptr;

  WaveForm()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    table = (float *)fftwf_malloc(sizeof(float) * tableSize);
    std::fill(table, table + tableSize, 0.0f);
  }

  ~WaveForm()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    if (table) fftwf_free(table);
  }

  inline float phaseSkewFunc(float x) { return x * x * x * x * x * x * x * x; }

  inline float distortionFunc(float x)
  {
    // (125 * x^3) / 32 - (825 * x^2) / 128 + (535 * x) / 512 + 1485 / 2048.

    auto y = (float(125) * x * x * x) / float(32) //
      - (float(825) * x * x) / float(128)         //
      + (float(535) * x) / float(512)             //
      + float(1485) / float(2048);

    return y < float(-2) ? x : y;
  }

  void draw(
    const std::array<float, ModID::MODID_ENUM_LENGTH> &mod,
    const std::array<float, nOscWavetable> &wavetable,
    WavetableParameter &param)
  {
    auto modHardSync = mixModulation(mod, param.modHardSync);
    auto modPhaseSkew = mixModulation(mod, param.modPhaseSkew);
    auto modDistortion = mixModulation(mod, param.modDistortion);

    auto hardSync = std::exp2(modHardSync) * param.hardSync;
    auto phaseSkew = std::clamp(modPhaseSkew + param.phaseSkew, float(0), float(1));
    auto distortion = std::clamp(modDistortion + param.distortion, float(0), float(1));

    for (size_t idx = 0; idx < tableSize; ++idx) {
      auto phase = hardSync * float(idx) / float(tableSize);
      phase -= std::floor(phase);

      phase = std::lerp(phase, phaseSkewFunc(phase), phaseSkew);

      if (param.waveInterpType == 0) { // Step interpolation.
        table[idx] = wavetable[size_t(nOscWavetable * phase)];
      } else if (param.waveInterpType == 1) { // Linear interpolation.
        auto pos = nOscWavetable * phase;
        size_t i0 = size_t(pos);
        size_t i1 = (i0 + 1) & (nOscWavetable - 1);
        table[idx] = std::lerp(wavetable[i0], wavetable[i1], pos - float(i0));
      } else { // Cubic interpolation.
        auto pos = nOscWavetable * phase;
        size_t i0 = size_t(pos);
        size_t i1 = (i0 + 1) & (nOscWavetable - 1);
        size_t i2 = (i0 + 2) & (nOscWavetable - 1);
        size_t i3 = (i0 + 3) & (nOscWavetable - 1);
        table[idx] = lagrange3Interp(
          wavetable[i0], wavetable[i1], wavetable[i2], wavetable[i3], pos - float(i0));
      }
    }

    for (size_t idx = 0; idx < tableSize; ++idx) {
      table[idx] = std::lerp(table[idx], distortionFunc(table[idx]), distortion);
    }
  }
};

template<size_t tableSize> struct Spectrum {
  static constexpr size_t spectrumSize = tableSize / 2 + 1;
  static constexpr float bendRange = 1.7320508075688772f; // sqrt(3).

  std::complex<float> *src = nullptr; // source.
  std::complex<float> *dst = nullptr; // destination.

  Spectrum()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    src = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spectrumSize);
    dst = (std::complex<float> *)fftwf_malloc(sizeof(std::complex<float>) * spectrumSize);
  }

  ~Spectrum()
  {
    const std::lock_guard<std::mutex> fftwLock(fftwMutex);

    if (src) fftwf_free(src);
    if (dst) fftwf_free(dst);
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
    auto spcSpreadGain = std::sqrt(spectralSpread);
    if (spectralSpread > 1) spcSpreadGain *= spectralSpread;
    auto phaseSlope = modPhaseSlope + param.phaseSlope;
    auto spcLpPitch = std::exp2(modSpectralLowpass) * param.spectralLowpass;
    auto spcHpPitch = std::exp2(modSpectralHighpass) * param.spectralHighpass;
    size_t end = std::clamp(
      size_t(spcLpPitch * spectrumSize), size_t(2),
      size_t(float(spectrumSize) * noteHz / param.upNyquistHz));
    size_t start = std::max(size_t(spcHpPitch * spectrumSize), size_t(1));

    for (size_t idx = start; idx < end; ++idx) {
      auto target = spectralSpread * float(idx);
      size_t index = size_t(target);
      if (index >= spectrumSize) break;

      auto frac = target - float(index);
      auto gain = spcSpreadGain * std::abs(float(1) - float(2) * frac);
      dst[idx] = gain * src[index] / float(tableSize);

      auto len = std::abs(dst[idx]);
      if (len < float(tableSize) * std::numeric_limits<float>::epsilon()) continue;
      auto arg = std::arg(dst[idx]);
      auto moddedPhase = arg + phaseSlope * len;
      dst[idx].real(len * std::cos(moddedPhase));
      dst[idx].imag(len * std::sin(moddedPhase));
    }
  }
};

struct VariableWaveTableOscillator {
  static constexpr size_t tableSize = 4096;
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

  // Call this before `process()`.
  bool isRefreshing() { return fadeCounter <= 0; }

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
      std::lerp(feedback[0], feedback[1], param.sumMix.getValue()),
      param.feedbackLowpassKp.getValue());
    const auto modPitch = mixModulation(mod, param.pitch.value);
    const auto immediatePm
      = mixModulation(mod, param.immedaitePm.value) + param.sumToImmediatePm.getValue();
    const auto accumulatePm
      = mixModulation(mod, param.accumulatePm.value) + param.sumToAccumulatePm.getValue();
    const auto fm = mixModulation(mod, param.fm.value) + param.sumToFm.getValue();

    phase += accumulatePm * oscSum
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
    return std::lerp(1.0f, oscSum, param.sumToAm.getValue()) * (vf + fade * (vb - vf));
  }
};

} // namespace SomeDSP
