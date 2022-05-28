// (c) 2020-2022 Takamitsu Endo
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

#include "multiratecoefficient.hpp"

#include <algorithm>

namespace SomeDSP {

template<typename Sample, typename Sos> class DecimationLowpass {
  std::array<Sample, Sos::co.size()> x0{};
  std::array<Sample, Sos::co.size()> x1{};
  std::array<Sample, Sos::co.size()> x2{};
  std::array<Sample, Sos::co.size()> y0{};
  std::array<Sample, Sos::co.size()> y1{};
  std::array<Sample, Sos::co.size()> y2{};

public:
  void reset()
  {
    x0.fill(0);
    x1.fill(0);
    x2.fill(0);
    y0.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  void push(Sample input) noexcept
  {
    x0[0] = input;
    std::copy(y0.begin(), y0.end() - 1, x0.begin() + 1);

    for (size_t i = 0; i < Sos::co.size(); ++i) {
      y0[i]                      //
        = Sos::co[i][0] * x0[i]  //
        + Sos::co[i][1] * x1[i]  //
        + Sos::co[i][2] * x2[i]  //
        - Sos::co[i][3] * y1[i]  //
        - Sos::co[i][4] * y2[i]; //
    }

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;
  }

  inline Sample output() { return y0[Sos::co.size() - 1]; }
};

template<typename Sample> class FirstOrderAllpass {
private:
  Sample x1 = 0;
  Sample y1 = 0;

public:
  void reset()
  {
    x1 = 0;
    y1 = 0;
  }

  Sample process(Sample x0, Sample a)
  {
    y1 = a * (x0 - y1) + x1;
    x1 = x0;
    return y1;
  }
};

template<typename Sample, typename Coefficient> class HalfBandIIR {
private:
  std::array<FirstOrderAllpass<Sample>, Coefficient::h0_a.size()> ap0;
  std::array<FirstOrderAllpass<Sample>, Coefficient::h1_a.size()> ap1;

public:
  void reset()
  {
    for (auto &ap : ap0) ap.reset();
    for (auto &ap : ap1) ap.reset();
  }

  // input[0] must be earlier sample.
  Sample process(const std::array<Sample, 2> &input)
  {
    auto s0 = input[0];
    for (size_t i = 0; i < ap0.size(); ++i) s0 = ap0[i].process(s0, Coefficient::h0_a[i]);
    auto s1 = input[1];
    for (size_t i = 0; i < ap1.size(); ++i) s1 = ap1[i].process(s1, Coefficient::h1_a[i]);
    return Sample(0.5) * (s0 + s1);
  }
};

template<typename Sample, typename FractionalDelayFIR> class FirUpSampler {
  std::array<Sample, FractionalDelayFIR::bufferSize> buf{};

public:
  std::array<Sample, FractionalDelayFIR::upfold> output;

  void reset() { buf.fill(Sample(0)); }

  void process(Sample input)
  {
    std::rotate(buf.rbegin(), buf.rbegin() + 1, buf.rend());
    buf[0] = input;

    std::fill(output.begin(), output.end(), Sample(0));
    for (size_t i = 0; i < FractionalDelayFIR::coefficient.size(); ++i) {
      auto &&phase = FractionalDelayFIR::coefficient[i];
      for (size_t n = 0; n < phase.size(); ++n) output[i] += buf[n] * phase[n];
    }
  }
};

template<typename Sample, typename FractionalDelayFIR> class TruePeakMeterConvolver {
  std::array<Sample, FractionalDelayFIR::bufferSize> buf{};

public:
  std::array<Sample, FractionalDelayFIR::upfold> output;

  void reset() { buf.fill(Sample(0)); }

  void process(Sample input)
  {
    std::rotate(buf.rbegin(), buf.rbegin() + 1, buf.rend());
    buf[0] = input;

    std::fill(output.begin(), output.end() - 1, Sample(0));
    output.back() = buf[FractionalDelayFIR::intDelay];
    for (size_t i = 0; i < FractionalDelayFIR::coefficient.size(); ++i) {
      auto &&phase = FractionalDelayFIR::coefficient[i];
      for (size_t n = 0; n < phase.size(); ++n) output[i] += buf[n] * phase[n];
    }
  }
};

template<typename Sample, size_t upSample> class CubicUpSampler {
  std::array<Sample, 4> buf{};

public:
  std::array<Sample, upSample> output;

  void reset() { buf.fill(Sample(0)); }

  // Range of t is in [0, 1]. Interpolates between y1 and y2.
  inline Sample cubicInterp(const std::array<Sample, 4> &y, Sample t)
  {
    auto t2 = t * t;
    auto c0 = y[1] - y[2];
    auto c1 = (y[2] - y[0]) * 0.5f;
    auto c2 = c0 + c1;
    auto c3 = c0 + c2 + (y[3] - y[1]) * 0.5f;
    return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y[1];
  }

  void process(Sample input)
  {
    std::rotate(buf.begin(), buf.begin() + 1, buf.end());
    buf.back() = input;

    std::fill(output.begin() + 1, output.end(), Sample(0));
    output[0] = buf[1];
    for (size_t i = 1; i < output.size(); ++i) {
      output[i] = cubicInterp(buf, Sample(i) / Sample(upSample));
    }
  }
};

template<typename Sample, size_t upSample> class LinearUpSampler {
  Sample buf = 0;

public:
  std::array<Sample, upSample> output;

  void reset() { buf = 0; }

  void process(Sample input)
  {
    Sample diff = input - buf;
    for (size_t i = 0; i < output.size(); ++i) {
      output[i] = buf + diff * Sample(i) / Sample(upSample);
    }
    buf = input;
  }
};

template<typename Sample> struct OverSampler16 {
  static constexpr size_t fold = 16;

  FirUpSampler<Sample, Fir16FoldUpSample<Sample>> upSampler;
  std::array<Sample, 16> inputBuffer{};
  DecimationLowpass<Sample, Sos16FoldFirstStage<Sample>> lowpass;
  HalfBandIIR<Sample, HalfBandCoefficient<Sample>> halfbandIir;

  void reset()
  {
    upSampler.reset();
    inputBuffer.fill({});
    lowpass.reset();
    halfbandIir.reset();
  }

  void push(Sample x0) { upSampler.process(x0); }
  Sample at(size_t index) { return upSampler.output[index]; }

  Sample process()
  {
    std::array<Sample, 2> halfBandInput;
    for (size_t i = 0; i < 8; ++i) lowpass.push(inputBuffer[i]);
    halfBandInput[0] = lowpass.output();
    for (size_t i = 8; i < 16; ++i) lowpass.push(inputBuffer[i]);
    halfBandInput[1] = lowpass.output();
    return halfbandIir.process(halfBandInput);
  }
};

} // namespace SomeDSP
