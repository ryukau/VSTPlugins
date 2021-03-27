// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/fftw3/fftw3.h"
#include "../../../lib/vcl/vectorclass.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <deque>
#include <random>

namespace SomeDSP {

constexpr size_t nTable = 136; // midi note nubmer 136 ~= 21096 Hz.
constexpr size_t nTablePadded = nTable + 4;
constexpr size_t notePitchUpperBound = nTable + 1;

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

// Range of t is in [0, 1]. Interpoltes between y1 and y2.
inline Vec16f cubicInterp(Vec16f y0, Vec16f y1, Vec16f y2, Vec16f y3, Vec16f t)
{
  auto t2 = t * t;
  auto c0 = y1 - y2;
  auto c1 = (y2 - y0) * float(0.5);
  auto c2 = c0 + c1;
  auto c3 = c0 + c2 + (y3 - y1) * float(0.5);
  return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y1;
}

/*
table is 2d array which has extra padding for interpolation.

    @              @  @
@   3  0  1  2  3  0  1
    3  0  1  2  3  0  1
   13 10 11 12 13 10 11
   23 20 21 22 23 20 21
   33 30 31 32 33 30 31
@   0  0  0  0  0  0  0
@   0  0  0  0  0  0  0
@   0  0  0  0  0  0  0

'@' in figure above represents padded array. Index is table[column][row].
- Padded first column has last element of original table.
- Padded last column has first element of original table.
- Padded first row is copy of first row of original table.
- Padded last 3 row is silence.
*/
template<size_t tableSize, size_t nPeak> struct WaveTable {
  static constexpr size_t spectrumSize = tableSize / 2 + 1;
  static constexpr size_t paddedSize = tableSize + 3;
  fftwf_complex *spectrum;
  fftwf_complex *bandLimited;
  fftwf_complex *tmpSpec;
  std::array<float *, nTablePadded> table;
  std::array<fftwf_plan, nTablePadded> plan;
  std::array<float, nTablePadded> frequency; // Must be sorted by ascending order.
  bool isRefreshing = true;
  float tableBaseFreq = 20.0f;

  WaveTable()
  {
    spectrum = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * spectrumSize);
    bandLimited = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * spectrumSize);
    tmpSpec = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * spectrumSize);

    for (size_t idx = 0; idx < nTablePadded; ++idx) {
      table[idx] = (float *)fftwf_malloc(sizeof(float) * paddedSize);
      table[idx][0] = 0;
      table[idx][paddedSize - 1] = 0;

      plan[idx]
        = fftwf_plan_dft_c2r_1d(tableSize, bandLimited, table[idx] + 1, FFTW_ESTIMATE);

      // TODO: Experiment with different frequency.
      frequency[idx] = 440.0f * powf(2.0f, (idx - 69.0f) / 12.0f);
    }

    // Last 3 tables are slince.
    for (size_t idx = nTablePadded - 3; idx < nTablePadded; ++idx) {
      for (size_t i = 0; i < paddedSize; ++i) table[idx][i] = 0;
    }
  }

  ~WaveTable()
  {
    for (auto &pln : plan) fftwf_destroy_plan(pln);
    for (auto &tbl : table) fftwf_free(tbl);
    fftwf_free(tmpSpec);
    fftwf_free(bandLimited);
    fftwf_free(spectrum);
  }

  inline float profile(float fi, float bwi, float shape)
  {
    if (bwi < 1e-5f) bwi = 1e-5f;
    auto x = fi / bwi;
    return powf(expf(-x * x) / bwi, shape);
  }

  void refreshTable()
  {
    isRefreshing = true;

    // table[0] and table[1] has full spectrum.
    bandLimited[0][0] = 0;
    bandLimited[0][1] = 0;
    std::memcpy(
      bandLimited + 1, spectrum + 1, sizeof(fftwf_complex) * (spectrumSize - 1));
    fftwf_execute(plan[0]);
    std::memcpy(table[1], table[0], sizeof(float) * paddedSize);

    for (size_t idx = 2; idx <= nTable; ++idx) {
      size_t bandIdx = size_t(spectrumSize * tableBaseFreq / frequency[idx]);
      bandIdx = std::clamp<size_t>(bandIdx, 1, spectrumSize);

      bandLimited[0][0] = 0;
      bandLimited[0][1] = 0;
      std::memcpy(bandLimited + 1, spectrum + 1, sizeof(fftwf_complex) * (bandIdx - 1));
      std::memset(
        bandLimited + bandIdx, 0, sizeof(fftwf_complex) * (spectrumSize - bandIdx));

      fftwf_execute(plan[idx]);
    }

    // Fill padded elements.
    for (size_t idx = 0; idx < nTablePadded - 1; ++idx) {
      table[idx][0] = table[idx][tableSize];
      table[idx][paddedSize - 2] = table[idx][1];
      table[idx][paddedSize - 1] = table[idx][2];
    }

    // Normalize.
    float max = 0.0f;
    for (size_t i = 0; i < tableSize; ++i) {
      auto value = fabsf(table[0][i]);
      if (max < value) max = value;
    }
    if (max != 0.0f) {
      for (size_t idx = 0; idx < nTablePadded - 1; ++idx) {
        for (size_t i = 0; i < paddedSize; ++i) table[idx][i] /= max;
      }
    }

    isRefreshing = false;
  }

  float sign(float x) { return float((0 < x) - (x < 0)); }

  void padsynth(
    float sampleRate,
    float tableBaseFreq,
    std::array<float, nPeak> &frequency,
    std::array<float, nPeak> &gain,
    std::array<float, nPeak> &phase,
    std::array<float, nPeak> &bandWidth,
    uint32_t seed,
    float expand,
    int32_t shift,
    uint32_t profileSkip,
    float profileShape,
    bool randomPitch,
    bool invertSpectrum,
    bool uniformPhaseProfile)
  {
    this->tableBaseFreq = tableBaseFreq;

    for (int32_t bin = 0; bin < spectrumSize; ++bin) {
      spectrum[bin][0] = 0;
      spectrum[bin][1] = 0;
    }

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> distFreq(100.0f, 8000.0f);
    for (int32_t peak = 0; peak < nPeak; ++peak) {
      float freq = randomPitch ? distFreq(rng) : frequency[peak];
      float bandHz = (powf(2.0f, bandWidth[peak] / 1200.0f) - 1.0f) * freq;
      float bandIdx = bandHz / (2.0f * sampleRate);

      float sigma = sqrtf(bandIdx * bandIdx / float(twopi));
      int32_t profileHalf = std::max<int32_t>(1, int32_t(spectrumSize * 5.0f * sigma));

      float freqIdx = freq * 2.0f / sampleRate;

      int32_t center = int32_t(freqIdx * spectrumSize);
      int32_t start = std::max<int32_t>(center - profileHalf, 0);
      int32_t end = std::min<int32_t>(center + profileHalf, spectrumSize);

      std::uniform_real_distribution<float> distPhase(0.0f, phase[peak]);
      float phi = distPhase(rng);
      for (int32_t bin = start; bin < end; bin += profileSkip) {
        float radius = gain[peak]
          * profile(bin / float(spectrumSize) - freqIdx, bandIdx,
                    std::floor(profileShape));
        if (!uniformPhaseProfile) phi = distPhase(rng);
        spectrum[bin][0] += radius * cosf(phi);
        spectrum[bin][1] += radius * sinf(phi);
      }
    }

    if (invertSpectrum) {
      float reMax = 0;
      float imMax = 0;
      for (int32_t bin = 1; bin < spectrumSize; ++bin) {
        float re = fabsf(spectrum[bin][0]);
        float im = fabsf(spectrum[bin][1]);
        if (reMax < re) reMax = re;
        if (imMax < im) imMax = im;
      }
      for (int32_t bin = 1; bin < spectrumSize; ++bin) {
        spectrum[bin][0] -= sign(spectrum[bin][0]) * reMax;
        spectrum[bin][1] -= sign(spectrum[bin][1]) * imMax;

        // TODO: test
        // spectrum[bin][0] -= std::copysign(reMax, spectrum[bin][0]);
        // spectrum[bin][1] -= std::copysign(imMax, spectrum[bin][1]);
      }
    }

    if (expand != 1.0f || shift != 0) {
      if (abs(shift) >= spectrumSize) {
        std::memcpy(tmpSpec, spectrum, sizeof(fftwf_complex) * spectrumSize);
      } else if (shift >= 0) {
        std::memset(tmpSpec, 0, sizeof(fftwf_complex) * shift);
        std::memcpy(
          tmpSpec + shift, spectrum, sizeof(fftwf_complex) * (spectrumSize - shift));
      } else { // shift < 0
        shift = abs(shift);
        std::memcpy(
          tmpSpec, spectrum + shift, sizeof(fftwf_complex) * (spectrumSize - shift));
        std::memset(tmpSpec + shift, 0, sizeof(fftwf_complex) * shift);
      }

      size_t bin = 1;
      for (; bin < spectrumSize; ++bin) {
        float tmpIdx = (bin - 1) / expand;
        int32_t low = int32_t(tmpIdx) + 1;
        if (low >= spectrumSize)
          break;
        else if (low < 0) {
        }
        size_t high = low + 1;
        float frac = tmpIdx - floorf(tmpIdx);
        spectrum[bin][0] = tmpSpec[low][0] + frac * (tmpSpec[high][0] - tmpSpec[low][0]);
        spectrum[bin][1] = tmpSpec[low][1] + frac * (tmpSpec[high][1] - tmpSpec[low][1]);
      }

      std::memset(spectrum + bin, 0, sizeof(fftwf_complex) * (spectrumSize - bin));
    }

    // Remove DC offset.
    spectrum[0][0] = 0.0f;
    spectrum[0][1] = 0.0f;

    refreshTable();
  }
};

template<size_t tableSize> struct TableOsc {
  static constexpr size_t paddedLast = tableSize + 1;
  float phase = 1; // table index starts from 1. 0 is padded index.
  float tick = 0;

  void setFrequency(float sampleRate, float frequency, float tableBaseFreq)
  {
    tick = frequency / tableBaseFreq;
    if (tick >= tableSize) tick = 0;
  }

  void reset()
  {
    phase = 1;
    tick = 0;
  }

  // notePitch is fractional note number. For example, notePitch = 60.12 means 60
  // semitones and 12 cents higher from midi note number 0.
  float process(float notePitch, std::array<float *, nTablePadded> &table)
  {
    phase += tick;
    if (phase > paddedLast) phase -= tableSize;

    if (notePitch <= 0) {
      float frac = phase - std::floor(phase);
      size_t x1 = size_t(phase);
      size_t x0 = x1 - 1;
      size_t x2 = x1 + 1;
      size_t x3 = x1 + 2;
      return cubicInterp(table[0][x0], table[0][x1], table[0][x2], table[0][x3], frac);
    } else if (notePitch >= notePitchUpperBound) {
      return 0;
    }
    notePitch += 1.0f;

    // // Bilinear interpolation.
    // auto yFrac = notePitch - floor(notePitch);
    // size_t south = size_t(notePitch);
    // size_t north = south + 1;
    // auto xFrac = phase - floor(phase);
    // size_t L = size_t(phase);
    // size_t R = L + 1;
    // auto x0 = table[south][L] + xFrac * (table[south][R] - table[south][L]);
    // auto x1 = table[north][L] + xFrac * (table[north][R] - table[north][L]);
    // return x0 + yFrac * (x1 - x0);

    // Bicubic interpolation.
    auto yFrac = notePitch - std::floor(notePitch);
    size_t iy1 = size_t(notePitch);
    size_t iy0 = iy1 - 1;
    size_t iy2 = iy1 + 1;
    size_t iy3 = iy1 + 2;

    auto xFrac = phase - std::floor(phase);
    size_t ix1 = size_t(phase);
    size_t ix0 = ix1 - 1;
    size_t ix2 = ix1 + 1;
    size_t ix3 = ix1 + 2;

    auto y0 = cubicInterp(
      table[iy0][ix0], table[iy0][ix1], table[iy0][ix2], table[iy0][ix3], xFrac);
    auto y1 = cubicInterp(
      table[iy1][ix0], table[iy1][ix1], table[iy1][ix2], table[iy1][ix3], xFrac);
    auto y2 = cubicInterp(
      table[iy2][ix0], table[iy2][ix1], table[iy2][ix2], table[iy2][ix3], xFrac);
    auto y3 = cubicInterp(
      table[iy3][ix0], table[iy3][ix1], table[iy3][ix2], table[iy3][ix3], xFrac);
    return cubicInterp(y0, y1, y2, y3, yFrac);
  }
};

template<size_t tableSize> struct alignas(64) TableOsc16 {
  static constexpr size_t paddedLast = tableSize + 1;
  Vec16f phase = 1; // table index starts from 1. 0 is padded index.
  Vec16f tick = 0;

  // Input phase is normalized in [0, 1], member phase is in [1, paddedLast].
  void setPhase(Vec16f phase) { this->phase = 1.0f + (phase - floor(phase)) * tableSize; }

  void setPhase(int index, float phase)
  {
    this->phase.insert(index, 1.0f + (phase - std::floor(phase)) * tableSize);
  }

  void setFrequency(Vec16f frequency, float tableBaseFreq)
  {
    tick = frequency / tableBaseFreq;
    tick = select(tick >= tableSize, 0, tick);
  }

  void setFrequency(int index, float frequency, float tableBaseFreq)
  {
    float tck = frequency / tableBaseFreq;
    tick.insert(index, tck >= tableSize ? 0 : tck);
  }

  void reset()
  {
    phase = 1;
    tick = 0;
  }

  inline Vec16f loadTable(Vec16i ix, Vec16i iy, std::array<float *, nTablePadded> &table)
  {
    return Vec16f(
      table[iy.extract(0)][ix.extract(0)], table[iy.extract(1)][ix.extract(1)],
      table[iy.extract(2)][ix.extract(2)], table[iy.extract(3)][ix.extract(3)],
      table[iy.extract(4)][ix.extract(4)], table[iy.extract(5)][ix.extract(5)],
      table[iy.extract(6)][ix.extract(6)], table[iy.extract(7)][ix.extract(7)],
      table[iy.extract(8)][ix.extract(8)], table[iy.extract(9)][ix.extract(9)],
      table[iy.extract(10)][ix.extract(10)], table[iy.extract(11)][ix.extract(11)],
      table[iy.extract(12)][ix.extract(12)], table[iy.extract(13)][ix.extract(13)],
      table[iy.extract(14)][ix.extract(14)], table[iy.extract(15)][ix.extract(15)]);
  }

  // notePitch is fractional note number. For example, notePitch = 60.12 means 60
  // semitones and 12 cents higher from midi note number 0.
  Vec16f process(Vec16f notePitch, std::array<float *, nTablePadded> &table)
  {
    phase += tick;
    phase = select(phase >= paddedLast, phase - tableSize, phase);

    notePitch = select(notePitch <= 0, 0, notePitch);
    notePitch += float(1);
    notePitch = select(notePitch >= notePitchUpperBound, notePitchUpperBound, notePitch);

    // Bilinear interpolation.
    Vec16f yFrac = notePitch - floor(notePitch);
    Vec16i iy0 = truncatei(notePitch);
    Vec16i iy1 = iy0 + 1;

    Vec16f xFrac = phase - floor(phase);
    Vec16i ix0 = truncatei(phase);
    Vec16i ix1 = ix0 + 1;

    Vec16f table00 = loadTable(ix0, iy0, table);
    Vec16f table01 = loadTable(ix1, iy0, table);
    Vec16f y0 = table00 + xFrac * (table01 - table00);

    Vec16f table10 = loadTable(ix0, iy1, table);
    Vec16f table11 = loadTable(ix1, iy1, table);
    Vec16f y1 = table10 + xFrac * (table11 - table10);

    return y0 + yFrac * (y1 - y0);
  }

  // Too slow.
  Vec16f processCubic(Vec16f notePitch, std::array<float *, nTablePadded> &table)
  {
    phase += tick;
    phase = select(phase >= paddedLast, phase - tableSize, phase);

    notePitch = select(notePitch <= 0, 0, notePitch);
    notePitch += float(1);
    notePitch = select(notePitch >= notePitchUpperBound, notePitchUpperBound, notePitch);

    // Bicubic interpolation.
    Vec16f yFrac = notePitch - floor(notePitch);
    Vec16i iy1 = truncatei(notePitch);
    Vec16i iy0 = iy1 - 1;
    Vec16i iy2 = iy1 + 1;
    Vec16i iy3 = iy1 + 2;

    Vec16f xFrac = phase - floor(phase);
    Vec16i ix1 = truncatei(phase);
    Vec16i ix0 = ix1 - 1;
    Vec16i ix2 = ix1 + 1;
    Vec16i ix3 = ix1 + 2;

    Vec16f table00 = loadTable(ix0, iy0, table);
    Vec16f table01 = loadTable(ix1, iy0, table);
    Vec16f table02 = loadTable(ix2, iy0, table);
    Vec16f table03 = loadTable(ix3, iy0, table);
    Vec16f y0 = cubicInterp(table00, table01, table02, table03, xFrac);

    Vec16f table10 = loadTable(ix0, iy1, table);
    Vec16f table11 = loadTable(ix1, iy1, table);
    Vec16f table12 = loadTable(ix2, iy1, table);
    Vec16f table13 = loadTable(ix3, iy1, table);
    Vec16f y1 = cubicInterp(table10, table11, table12, table13, xFrac);

    Vec16f table20 = loadTable(ix0, iy2, table);
    Vec16f table21 = loadTable(ix1, iy2, table);
    Vec16f table22 = loadTable(ix2, iy2, table);
    Vec16f table23 = loadTable(ix3, iy2, table);
    Vec16f y2 = cubicInterp(table20, table21, table22, table23, xFrac);

    Vec16f table30 = loadTable(ix0, iy3, table);
    Vec16f table31 = loadTable(ix1, iy3, table);
    Vec16f table32 = loadTable(ix2, iy3, table);
    Vec16f table33 = loadTable(ix3, iy3, table);
    Vec16f y3 = cubicInterp(table30, table31, table32, table33, xFrac);

    return cubicInterp(y0, y1, y2, y3, yFrac);
  }
};

template<size_t tableSize> struct alignas(64) LfoWaveTable {
  std::array<float, tableSize + 1> table;

  enum InterpType : int32_t { interpStep, interpLinear, interpCubic };

  void refreshTable(std::vector<float> &uiTable, int interpType)
  {
    const size_t last = table.size() - 1;
    switch (interpType) {
      case interpStep: {
        for (size_t idx = 0; idx < last; ++idx) {
          size_t uiIdx = size_t(uiTable.size() * idx / float(last));
          table[idx] = uiTable[uiIdx];
        }
      } break;

      case interpLinear: {
        uiTable.push_back(uiTable[0]);
        const size_t uiTableLast = uiTable.size() - 1;
        for (size_t idx = 0; idx < last; ++idx) {
          float targetIdx = uiTableLast * idx / float(last);
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
        const size_t uiTableLast = uiTable.size() - 3;
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

template<size_t tableSize> struct alignas(64) LfoTableOsc16 {
public:
  Vec16f phase = 0;
  Vec16f tick = 0;

  void setFrequency(float sampleRate, Vec16f frequency)
  {
    tick = frequency * tableSize / sampleRate;
    tick = select(tick >= tableSize, 0, tick);
  }

  void setFrequency(int index, float sampleRate, float frequency)
  {
    float tck = frequency * tableSize / sampleRate;
    tick.insert(index, tck >= tableSize ? 0 : tck);
  }

  void reset()
  {
    phase = 0;
    tick = 0;
  }

  void reset(int index) { phase.insert(index, 0); }

  inline Vec16f loadTable(Vec16i ix, std::array<float, tableSize + 1> &table)
  {
    return Vec16f(
      table[ix[0]], table[ix[1]], table[ix[2]], table[ix[3]], table[ix[4]], table[ix[5]],
      table[ix[6]], table[ix[7]], table[ix[8]], table[ix[9]], table[ix[10]],
      table[ix[11]], table[ix[12]], table[ix[13]], table[ix[14]], table[ix[15]]);
  }

  Vec16f process(std::array<float, tableSize + 1> &table)
  {
    phase += tick;
    phase = select(phase >= tableSize, phase - tableSize, phase);

    Vec16f xFrac = phase - floor(phase);
    Vec16i ix0 = truncatei(phase);
    Vec16i ix1 = ix0 + 1;

    Vec16f x0 = loadTable(ix0, table);
    Vec16f x1 = loadTable(ix1, table);
    return x0 + xFrac * (x1 - x0);
  }
};

} // namespace SomeDSP
