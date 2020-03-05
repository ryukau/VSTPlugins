// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/somemath.hpp"
#include "../../../lib/juce_FastMathApproximations.h"

#include <array>
#include <memory>

namespace SomeDSP {

enum class BiquadType {
  lowpass,
  highpass,
  bandpass,
  notch,
};

enum class ShaperType { hardclip, tanh, sinRunge, cubicExpDecayAbs };

template<typename Sample> class SerialFilter4 {
public:
  BiquadType type = BiquadType::lowpass;
  Sample fs;
  Sample f0;
  Sample q;

  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample a0 = 0.0;
  Sample a1 = 0.0;
  Sample a2 = 0.0;

  Sample w0 = 0.0;
  Sample cos_w0 = 0.0;
  Sample sin_w0 = 0.0;
  Sample alpha = 0.0;

  std::array<Sample, 4> x0{};
  std::array<Sample, 4> x1{};
  std::array<Sample, 4> x2{};
  std::array<Sample, 4> y0{};
  std::array<Sample, 4> y1{};
  std::array<Sample, 4> y2{};

  Sample feedback = 0.0;
  Sample saturation = 1.0;
  ShaperType shaper = ShaperType::sinRunge;

  SerialFilter4(Sample sampleRate, Sample cutoff, Sample resonance)
    : fs(sampleRate), f0(cutoff), q(resonance)
  {
  }

  void reset()
  {
    b0 = b1 = b2 = 0.0f;
    a0 = a1 = a2 = 0.0f;
    clear();
  }

  void clear()
  {
    feedback = 0.0;
    x1.fill(0);
    x2.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  Sample clamp(Sample value, Sample min, Sample max)
  {
    return value < min ? min : value > max ? max : value;
  }

  void setLowpass()
  {
    alpha = sin_w0 / (Sample(2.0) * q);
    b0 = (Sample(1.0) - cos_w0) / Sample(2.0);
    b1 = Sample(1.0) - cos_w0;
    b2 = (Sample(1.0) - cos_w0) / Sample(2.0);
    a0 = Sample(1.0) + alpha;
    a1 = -Sample(2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;
  }

  void setHighpass()
  {
    alpha = sin_w0 / (Sample(2.0) * q);
    b0 = (Sample(1.0) + cos_w0) / Sample(2.0);
    b1 = -(Sample(1.0) + cos_w0);
    b2 = (Sample(1.0) + cos_w0) / Sample(2.0);
    a0 = Sample(1.0) + alpha;
    a1 = -Sample(2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;
  }

  void setBandpass()
  {
    // 0.34657359027997264 = log(2) / 2.
    alpha = sin_w0 * somesinh<Sample>(Sample(0.34657359027997264) * q * w0 / sin_w0);
    b0 = alpha;
    b1 = 0.0;
    b2 = -alpha;
    a0 = Sample(1.0) + alpha;
    a1 = -Sample(2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;
  }

  void setNotch()
  {
    alpha = sin_w0 * somesinh<Sample>(Sample(0.34657359027997264) * q * w0 / sin_w0);
    b0 = Sample(1.0);
    b1 = -Sample(2.0) * cos_w0;
    b2 = Sample(1.0);
    a0 = Sample(1.0) + alpha;
    a1 = -Sample(2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;
  }

  void setCutoffQ(Sample hz, Sample q)
  {
    f0 = clamp(hz, Sample(20.0), Sample(20000.0));
    this->q = clamp(q, Sample(1e-5), Sample(1.0));

    w0 = twopi * f0 / fs;
    cos_w0 = juce::dsp::FastMathApproximations::cos<Sample>(w0);
    sin_w0 = juce::dsp::FastMathApproximations::sin<Sample>(w0);
    switch (type) {
      default:
      case BiquadType::lowpass:
        setLowpass();
        break;

      case BiquadType::highpass:
        setHighpass();
        break;

      case BiquadType::bandpass:
        setBandpass();
        break;

      case BiquadType::notch:
        setNotch();
        break;
    }
  }

  Sample shaperSinRunge(Sample x)
  {
    return somesin<Sample>(Sample(2.0 * pi) * x) / (Sample(1.0) + Sample(10.0) * x * x);
  }

  Sample shaperCubicExpDecayAbs(Sample x)
  {
    // Solve x for: diff(x^3*exp(-x), x) = 0,
    // then we get: x = 0, 27 * math.exp(-3).
    // 0.7439087749328765 = 1 / (27 * math.exp(-3))
    return Sample(0.7439087749328765) * x * x * x * someexp<Sample>(-somefabs<Sample>(x));
  }

  Sample shaperHardclip(Sample x)
  {
    return x < Sample(-1.0) ? Sample(-1.0) : x > Sample(1.0) ? Sample(1.0) : x;
  }

  Sample process(Sample input)
  {
    input = saturation * (input - feedback * y0[3]);
    switch (shaper) {
      default:
      case ShaperType::hardclip:
        input = shaperHardclip(input);
        break;

      case ShaperType::tanh:
        // input = sometanh<Sample>(input);
        input = juce::dsp::FastMathApproximations::tanh<Sample>(input);
        break;

      case ShaperType::sinRunge:
        input = shaperSinRunge(input);
        break;

      case ShaperType::cubicExpDecayAbs:
        input = shaperCubicExpDecayAbs(input);
        break;
    }

    x0[0] = input;
    x0[1] = y0[0];
    x0[2] = y0[1];
    x0[3] = y0[2];

    y0[0] = (b0 * x0[0] + b1 * x1[0] + b2 * x2[0] - a1 * y1[0] - a2 * y2[0]) / a0;
    y0[1] = (b0 * x0[1] + b1 * x1[1] + b2 * x2[1] - a1 * y1[1] - a2 * y2[1]) / a0;
    y0[2] = (b0 * x0[2] + b1 * x1[2] + b2 * x2[2] - a1 * y1[2] - a2 * y2[2]) / a0;
    y0[3] = (b0 * x0[3] + b1 * x1[3] + b2 * x2[3] - a1 * y1[3] - a2 * y2[3]) / a0;

    x2[0] = x1[0];
    x2[1] = x1[1];
    x2[2] = x1[2];
    x2[3] = x1[3];

    x1[0] = x0[0];
    x1[1] = x0[1];
    x1[2] = x0[2];
    x1[3] = x0[3];

    y2[0] = y1[0];
    y2[1] = y1[1];
    y2[2] = y1[2];
    y2[3] = y1[3];

    y1[0] = y0[0];
    y1[1] = y0[1];
    y1[2] = y0[2];
    y1[3] = y0[3];

    if (std::isfinite(y0[3])) return y0[3];
    clear();
    return 0.0;
  }
};

} // namespace SomeDSP
