// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#define POCKETFFT_NO_MULTITHREADING
#include "../../../lib/pocketfft/pocketfft_hdronly.h"

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/somemath.hpp"

#include <algorithm>
#include <cstring>
#include <deque>
#include <random>
#include <vector>

namespace SomeDSP {

// Range of t is in [0, 1]. Interpoltes between y1 and y2.
inline float cubicInterp(float y0, float y1, float y2, float y3, float t)
{
  auto t2 = t * t;
  auto c0 = y1 - y2;
  auto c1 = (y2 - y0) * 0.5f;
  auto c2 = c0 + c1;
  auto c3 = c0 + c2 + (y3 - y1) * 0.5f;
  return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y1;
}

template<typename Sample> struct PeakInfo {
  Sample frequency = 0;
  Sample gain = 0;
  Sample phase = 0;
  Sample bandWidth = 1;
};

template<typename T> class PocketFFT {
public:
  pocketfft::shape_t shape;
  pocketfft::stride_t strideR;
  pocketfft::stride_t strideC;
  pocketfft::shape_t axes;
  size_t ndata = 1;

  void setShape(pocketfft::shape_t shape)
  {
    this->shape = shape;

    strideR.resize(shape.size());
    strideC.resize(shape.size());

    size_t tmpR = sizeof(T);
    size_t tmpC = sizeof(std::complex<T>);
    for (int i = shape.size() - 1; i >= 0; --i) {
      strideR[i] = tmpR;
      tmpR *= shape[i];
      strideC[i] = tmpC;
      tmpC *= shape[i];
    }

    ndata = 1;
    for (const auto &shp : shape) ndata *= shp;

    axes.resize(shape.size());
    std::iota(axes.begin(), axes.end(), 0);
  }

  void r2c(const T *data_in, std::complex<T> *data_out, bool forward = true, T scale = 1)
  {
    pocketfft::r2c(shape, strideR, strideC, axes, forward, data_in, data_out, scale);
  }

  void c2r(const std::complex<T> *data_in, T *data_out, bool forward = false, T scale = 1)
  {
    pocketfft::c2r(
      shape, strideC, strideR, axes, forward, data_in, data_out, scale / ndata);
  }
};

/**
Last element of table is padded for linear interpolation.
For example, consider following table:

```
tableRaw = [11, 22, 33, 44].
```

which will be padded as following:

```
tablePadded = [11, 22, 33, 44, 11].
```
 */
template<size_t tableSize> struct WaveTable {
  std::vector<std::complex<float>> spectrum;
  std::vector<std::complex<float>> tmpSpec;
  float tableBaseFreq = 20.0f;
  PocketFFT<float> fft;

  WaveTable()
  {
    size_t spectrumSize = tableSize / 2 + 1;
    spectrum.resize(spectrumSize);
    tmpSpec.resize(spectrumSize);

    pocketfft::shape_t shape{tableSize};
    fft.setShape(shape);
  }

  size_t getTableSize() { return tableSize; }

  inline float profile(float fi, float bwi, float shape)
  {
    if (bwi < 1e-5) bwi = 1e-5;
    auto x = fi / bwi;
    return powf(expf(-x * x) / bwi, shape);
  }

  void padsynth(
    float sampleRate,
    float tableBaseFreq,
    std::vector<PeakInfo<float>> &peakInfos,
    uint32_t seed,
    float expand,
    int32_t shift,
    uint32_t profileSkip, // 1 or greater.
    float profileShape,
    bool uniformPhaseProfile)
  {
    if (profileSkip < 1) profileSkip = 1;

    this->tableBaseFreq = tableBaseFreq;

    for (int32_t bin = 1; bin < spectrum.size(); ++bin) spectrum[bin] = 0.0f;

    std::minstd_rand rng(seed);
    for (const auto &peak : peakInfos) {
      float bandHz = (powf(2.0f, peak.bandWidth / 1200.0f) - 1.0f) * peak.frequency;
      float bandIdx = bandHz / (2.0f * sampleRate);

      float sigma = sqrtf(bandIdx * bandIdx / float(twopi));
      int32_t profileHalf = std::max<int32_t>(1, int32_t(spectrum.size() * 5 * sigma));

      float freqIdx = peak.frequency * 2.0f / sampleRate;

      int32_t center = int32_t(freqIdx * spectrum.size());
      int32_t start = std::max<int32_t>(center - profileHalf, 0);
      int32_t end = std::min<int32_t>(center + profileHalf, spectrum.size());

      std::uniform_real_distribution<float> distPhase(0.0f, peak.phase);
      auto phase = distPhase(rng);
      for (int32_t bin = start; bin < end; bin += profileSkip) {
        auto radius = peak.gain
          * profile(bin / float(spectrum.size()) - freqIdx, bandIdx, profileShape);
        if (!uniformPhaseProfile) phase = distPhase(rng);
        spectrum[bin] += std::complex<float>(radius * cosf(phase), radius * sinf(phase));
      }
    }

    if (expand != 1.0f || shift != 0) {
      auto sa = abs(shift);
      if (sa >= spectrum.size()) {
        tmpSpec = spectrum;
      } else if (shift >= 0) {
        std::fill_n(tmpSpec.begin(), shift, 0);
        std::copy(tmpSpec.begin() + shift, tmpSpec.end(), spectrum.begin());
      } else { // shift < 0
        std::copy(tmpSpec.begin(), tmpSpec.end() - sa, spectrum.begin() + sa);
        std::fill(tmpSpec.end() - sa, tmpSpec.end(), 0);
      }

      size_t bin = 1;
      for (; bin < spectrum.size(); ++bin) {
        float tmpIdx = (bin - 1) / expand;
        int32_t low = int32_t(tmpIdx) + 1;
        if (low >= spectrum.size()) break;
        size_t high = low + 1;
        float frac = tmpIdx - floorf(tmpIdx);
        spectrum[bin] = tmpSpec[low] + frac * (tmpSpec[high] - tmpSpec[low]);
      }

      if (bin < spectrum.size()) std::fill(spectrum.begin() + bin, spectrum.end(), 0);
    }

    // Remove DC offset.
    spectrum[0] = 0.0f;

    // Normalize spectrum. Reference: https://dsp.stackexchange.com/a/3470
    float sum = 0;
    for (const auto &bin : spectrum) sum += abs(bin);
    if (sum != 0) {
      sum = 0.5f * sum / tableSize;
      for (auto &bin : spectrum) bin /= sum;
    }
  }

  void refreshTable(float frequency, std::vector<float> &table)
  {
    size_t bandIdx = size_t(spectrum.size() * tableBaseFreq / frequency);
    bandIdx = std::clamp<size_t>(bandIdx, 1, spectrum.size());

    std::copy_n(spectrum.begin(), bandIdx, tmpSpec.begin());
    std::fill(tmpSpec.begin() + bandIdx, tmpSpec.end(), 0);

    fft.c2r(tmpSpec.data(), table.data());

    // Fill padded elements.
    table[table.size() - 1] = table[0];
  }
};

template<size_t tableSize> struct TableOsc {
  std::vector<float> table;
  float phase = 0;
  float tick = 0;

  TableOsc() { table.resize(tableSize + 1); }

  void setFrequency(float frequency, float tableBaseFreq)
  {
    tick = frequency / tableBaseFreq;
    if (tick >= tableSize) tick = 0;
  }

  // Input phase is normalized in [0, 1], member phase is in [0, tableSize].
  void setPhase(float phase) { this->phase = (phase - floorf(phase)) * tableSize; }

  void reset() { phase = 0; }

  float process()
  {
    phase += tick;
    if (phase >= tableSize) phase -= tableSize;

    size_t x0 = phase;
    return table[x0] + (phase - floorf(phase)) * (table[x0 + 1] - table[x0]);
  }
};

template<size_t tableSize> struct LfoWavetable {
  std::array<float, tableSize + 1> table;

  enum InterpType : int32_t { interpStep, interpLinear, interpCubic };

  void refreshTable(std::vector<float> &uiTable, int interpType)
  {
    const size_t last = table.size() - 1;
    switch (interpType) {
      case interpStep: {
        for (size_t idx = 0; idx < last; ++idx) {
          size_t uiIdx = float(uiTable.size() * idx / float(last));
          table[idx] = uiTable[uiIdx];
        }
      } break;

      case interpLinear: {
        uiTable.push_back(uiTable[0]);
        const size_t uiTableLast = uiTable.size() - 1;
        for (size_t idx = 0; idx < last; ++idx) {
          float targetIdx = float(uiTableLast * idx / float(last));
          float frac = targetIdx - floorf(targetIdx);
          size_t uiIdx = size_t(targetIdx);
          table[idx] = uiTable[uiIdx] + frac * (uiTable[uiIdx + 1] - uiTable[uiIdx]);
        }
      } break;

      case interpCubic:
      default: {
        uiTable.insert(uiTable.begin(), uiTable.back());
        uiTable.push_back(uiTable[1]);
        uiTable.push_back(uiTable[2]);
        const size_t uiTableLast = uiTable.size() - 2;
        for (size_t idx = 0; idx < last; ++idx) {
          float targetIdx = 1 + float(uiTableLast * idx / float(last));
          float frac = targetIdx - floorf(targetIdx);
          size_t uiIdx = size_t(targetIdx);
          table[idx] = cubicInterp(
            uiTable[uiIdx - 1], uiTable[uiIdx], uiTable[uiIdx + 1], uiTable[uiIdx + 2],
            frac);
        }
      } break;
    }
    table[last] = table[0];
  }
};

template<size_t tableSize> struct LfoTableOsc {
public:
  float phase = 0;

  void reset() { phase = 0; }

  float process(
    const std::array<float, tableSize + 1> &table, float sampleRate, float frequency)
  {
    float tick = frequency * tableSize / sampleRate;
    if (tick >= tableSize) tick = 0;

    phase += tick;
    if (phase >= tableSize) phase -= tableSize;

    float xFrac = phase - floor(phase);
    int ix0 = phase;
    return table[ix0] + xFrac * (table[ix0 + 1] - table[ix0]);
  }
};

template<typename Sample> class LP3 {
public:
  void reset() { acc = vel = pos = x1 = 0; }

  Sample process(const Sample x0, Sample sampleRate, Sample lowpassHz, Sample resonance)
  {
    // Map cutoff to filter coefficient `c`.
    Sample fc = lowpassHz / sampleRate;
    auto c = Sample(14.57922056987288) * fc * fc * fc
      + Sample(-15.50319149517482) * fc * fc + Sample(5.8725399228949335) * fc;

    auto k = resonance;

    // Process filter.
    acc = k * acc + c * vel;
    vel -= acc + x0 - x1;
    pos -= c / (1 - k) * vel;

    x1 = x0;
    return pos;
  }

private:
  Sample acc = 0;
  Sample vel = 0;
  Sample pos = 0;
  Sample x1 = 0;
};

} // namespace SomeDSP
