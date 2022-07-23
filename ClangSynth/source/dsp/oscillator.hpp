// (c) 2022 Takamitsu Endo
//
// This file is part of ClangSynth.
//
// ClangSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#define POCKETFFT_NO_MULTITHREADING
#include "../../../lib/pocketfft/pocketfft_hdronly.h"

#include "../../../common/dsp/constants.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

namespace SomeDSP {

template<typename T> class PocketFFT {
public:
  pocketfft::shape_t shape;
  pocketfft::stride_t strideR;
  pocketfft::stride_t strideC;
  pocketfft::shape_t axes;
  size_t ndata = 1;

  void setShape(pocketfft::shape_t _shape)
  {
    shape = _shape;

    strideR.resize(shape.size());
    strideC.resize(shape.size());

    size_t tmpR = sizeof(T);
    size_t tmpC = sizeof(std::complex<T>);
    for (int i = int(shape.size()) - 1; i >= 0; --i) {
      strideR[i] = tmpR;
      tmpR *= shape[i];
      strideC[i] = tmpC;
      tmpC *= shape[i];
    }

    ndata = 1;
    for (const auto &shp : shape) ndata *= shp;

    axes.resize(shape.size());
    for (size_t i = 0; i < axes.size(); ++i) axes[i] = i;
  }

  void setShape1D(size_t size)
  {
    pocketfft::shape_t _shape{size};
    setShape(_shape);
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

template<typename Sample, size_t nOvertone> struct WavetableParameter {
  std::array<std::complex<Sample>, nOvertone> overtoneAmp{};
  std::vector<std::complex<Sample>> source;
  std::vector<Sample> power;

  Sample denominatorSlope = Sample(1);
  Sample rotationSlope = 0;
  Sample rotationOffset = 0;
  Sample blur = Sample(1);
  size_t interval = 1;
  size_t highpassIndex = 0;

  WavetableParameter()
  {
    overtoneAmp[0].real(Sample(1));
    source.reserve(65536);
  }
};

template<typename Sample, size_t nOvertone> class Wavetable {
private:
  static constexpr Sample bendRange = Sample(1.7320508075688772935); // sqrt(3).
  static constexpr Sample baseFreq
    = Sample(0.5); // Lowest frequency without loss of harmonics.

  Sample basenote = Sample(0);
  Sample interval = Sample(12);
  Sample maxIdx = Sample(0);
  size_t bufSize = 2;

  std::vector<std::vector<Sample>> table{
    {Sample(0), Sample(0), Sample(0)},
    {Sample(0), Sample(0), Sample(0)},
  };

  std::vector<std::complex<Sample>> fullSpectrum;
  PocketFFT<Sample> fft;

  void generateSpectrum(size_t spectrumSize)
  {
    constexpr double halfpi = 1.57079632679489661923;
    constexpr double pi = 3.14159265358979323846;
    constexpr double twopi = 6.28318530717958647692;
    constexpr auto eps = std::numeric_limits<Sample>::epsilon();

    // Generate source spectrum amplitude.
    param.power.resize(spectrumSize);
    std::fill(param.power.begin(), param.power.end(), Sample(0));
    for (size_t k = 1; k < param.power.size(); ++k) {
      param.power[k] = Sample(1) / (Sample(1) + Sample(k - 1) * param.denominatorSlope);
    }

    // Blur with bidirectional filtering. `param.blur` is EMA filter coefficient.
    if (param.blur < Sample(1) && param.power.size() >= 2) {
      auto value = param.power[1];
      for (size_t k = 1; k < param.power.size(); ++k) {
        value += param.blur * (param.power[k] - value);
        param.power[k] = value;
      }
      for (size_t k = param.power.size() - 1; k > 0; --k) {
        value += param.blur * (param.power[k] - value);
        param.power[k] = value;
      }
    }

    // Highpass.
    for (size_t k = 1; k <= param.highpassIndex; ++k) {
      if (k >= param.power.size()) break;
      param.power[k] *= Sample(k) / Sample(param.highpassIndex);
    }

    // Normalize.
    auto sumAmp = std::transform_reduce(
      param.overtoneAmp.begin(), param.overtoneAmp.end(), Sample(0), std::plus{},
      [](auto v) { return std::abs(v); });
    if (sumAmp < eps) sumAmp = eps;
    auto normalizeAmp = Sample(bufSize) / sumAmp;

    // Fill complex spectrum.
    param.source.resize(param.power.size());
    std::fill(param.source.begin(), param.source.end(), std::complex<Sample>(0, 0));
    for (size_t k = 1; k < param.source.size(); k += param.interval) {
      param.source[k] = std::polar(
        param.power[k] * normalizeAmp,
        Sample(twopi) * (param.rotationOffset + Sample(k - 1) * param.rotationSlope));
    }

    // Overtones.
    fullSpectrum.resize(param.source.size());
    std::fill(fullSpectrum.begin(), fullSpectrum.end(), std::complex<Sample>(0, 0));
    for (size_t i = 0; i < nOvertone; ++i) {
      for (size_t k = 1; k < param.source.size(); ++k) {
        auto index = (i + 1) * k;
        if (index >= fullSpectrum.size()) break;

        fullSpectrum[index] += param.overtoneAmp[i] * param.source[k];
      }
    }
  }

public:
  WavetableParameter<Sample, nOvertone> param;

  void fillTable(Sample upRate)
  {
    size_t exponent = std::clamp<size_t>(
      size_t(std::log2(upRate / baseFreq)), 1, std::numeric_limits<size_t>::digits - 1);
    bufSize = size_t(1) << exponent;
    size_t nFreq = bufSize / 2; // Without DC.

    generateSpectrum(nFreq + 1);

    fft.setShape1D(bufSize);

    size_t nTable = size_t(std::log(Sample(nFreq)) / std::log(bendRange));
    maxIdx = Sample(nTable - 1);

    basenote = freqToNote(upRate / Sample(bufSize));
    interval = Sample(12) * std::log2(bendRange);

    table.resize(nTable + 1); // Last table is filled by 0.
    std::vector<std::complex<Sample>> destSpc(fullSpectrum.size());
    for (size_t idx = table.size() - 2; idx < table.size(); --idx) {
      size_t cutoff = size_t(nFreq * std::pow(bendRange, -Sample(idx))) + 1;
      std::fill(destSpc.begin(), destSpc.end(), std::complex<Sample>(0, 0));
      std::copy(fullSpectrum.begin(), fullSpectrum.begin() + cutoff, destSpc.begin());

      table[idx].resize(bufSize + 1);
      fft.c2r(destSpc.data(), table[idx].data());
      table[idx].back() = table[idx][0];
    }
    table.back().resize(bufSize + 1);
    std::fill(table.back().begin(), table.back().end(), Sample(0));
  }

  Sample process(Sample note, Sample phase) const
  {
    auto octFloat = std::clamp((note - basenote) / interval, Sample(0), maxIdx);
    auto iTbl = size_t(octFloat);
    auto yFrac = octFloat - Sample(iTbl);

    auto pos = Sample(bufSize) * phase;
    auto idx = size_t(pos);
    auto xFrac = pos - Sample(idx);

    auto a0 = table[iTbl][idx];
    auto a1 = table[iTbl][idx + 1];
    auto s0 = a0 + xFrac * (a1 - a0);

    auto b0 = table[iTbl + 1][idx];
    auto b1 = table[iTbl + 1][idx + 1];
    auto s1 = b0 + xFrac * (b1 - b0);

    return s0 + yFrac * (s1 - s0);
  }
};

template<typename Sample, size_t nOvertone> struct TableOsc {
  Sample phase = Sample(0);

  void reset() { phase = Sample(0); }

  Sample process(Sample upRate, Sample note, const Wavetable<Sample, nOvertone> &wt)
  {
    phase += std::clamp(noteToFreq(note) / upRate, Sample(0), Sample(0.5));
    phase -= std::floor(phase);
    return wt.process(note, phase);
  }
};

} // namespace SomeDSP
