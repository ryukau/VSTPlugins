// (c) 2019 Takamitsu Endo
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

#include <array>
#include <memory>

#include "constants.hpp"
#include "somemath.hpp"

namespace SomeDSP {

enum class BiquadType {
  lowpass,
  highpass,
  bandpass,
  notch,
};

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
template<typename Sample> class Biquad {
public:
  BiquadType type;
  Sample fs;
  Sample f0;
  Sample q;

  Sample x1 = 0.0;
  Sample x2 = 0.0;
  Sample y1 = 0.0;
  Sample y2 = 0.0;

  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample a0 = 0.0;
  Sample a1 = 0.0;
  Sample a2 = 0.0;

  Biquad(Sample sampleRate, Sample cutoff, Sample q) : fs(sampleRate), f0(cutoff), q(q) {}

  void reset()
  {
    b0 = b1 = b2 = 0.0f;
    a0 = a1 = a2 = 0.0f;
    x1 = x2 = 0.0f;
    y1 = y2 = 0.0f;
  }

  void clear()
  {
    x1 = x2 = 0.0f;
    y1 = y2 = 0.0f;
  }

  void setType(BiquadType value)
  {
    type = value;
    setCoefficient();
  }

  void setCutoffQ(Sample hz, Sample q)
  {
    f0 = hz > 0.0 ? hz : 0.0;
    this->q = q < Sample(1e-5) ? Sample(1e-5) : q;
    setCoefficient();
  }

  void setCoefficient()
  {
    Sample w0 = twopi * f0 / fs;

    // These cos and sin are performance bottle neck.
    Sample cos_w0 = somecos<Sample>(w0);
    Sample sin_w0 = somesin<Sample>(w0);

    switch (type) {
      default:
      case BiquadType::lowpass: {
        Sample alpha = sin_w0 / (Sample(2.0) * q);
        b0 = (Sample(1.0) - cos_w0) / Sample(2.0);
        b1 = Sample(1.0) - cos_w0;
        b2 = (Sample(1.0) - cos_w0) / Sample(2.0);
        a0 = Sample(1.0) + alpha;
        a1 = -Sample(2.0) * cos_w0;
        a2 = Sample(1.0) - alpha;
      } break;

      case BiquadType::highpass: {
        Sample alpha = sin_w0 / (Sample(2.0) * q);
        b0 = (Sample(1.0) + cos_w0) / Sample(2.0);
        b1 = -(Sample(1.0) + cos_w0);
        b2 = (Sample(1.0) + cos_w0) / Sample(2.0);
        a0 = Sample(1.0) + alpha;
        a1 = -Sample(2.0) * cos_w0;
        a2 = Sample(1.0) - alpha;
      } break;

      case BiquadType::bandpass: {
        // 0.34657359027997264 = log(2) / 2.
        Sample alpha = sin_w0 * somesinh<Sample>(0.34657359027997264 * q * w0 / sin_w0);
        b0 = alpha;
        b1 = 0.0;
        b2 = -alpha;
        a0 = Sample(1.0) + alpha;
        a1 = -Sample(2.0) * cos_w0;
        a2 = Sample(1.0) - alpha;
      } break;

      case BiquadType::notch: {
        Sample alpha = sin_w0 * somesinh<Sample>(0.34657359027997264 * q * w0 / sin_w0);
        b0 = Sample(1.0);
        b1 = -Sample(2.0) * cos_w0;
        b2 = Sample(1.0);
        a0 = Sample(1.0) + alpha;
        a1 = -Sample(2.0) * cos_w0;
        a2 = Sample(1.0) - alpha;
      } break;
    }
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
  }

  void copyCoefficient(std::unique_ptr<Biquad<Sample>> const &biquad)
  {
    a0 = biquad->a0;
    a1 = biquad->a1;
    a2 = biquad->a2;
    b0 = biquad->b0;
    b1 = biquad->b1;
    b2 = biquad->b2;
  }

  Sample process(Sample input)
  {
    Sample output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    x2 = x1;
    x1 = input;

    y2 = y1;
    y1 = output;

    if (std::isfinite(output)) return output;

    clear();
    return 0.0;
  }
};

template<typename Sample> inline Sample shaperSinRunge(Sample x)
{
  return somesin<Sample>(Sample(2.0 * pi) * x) / (Sample(1.0) + Sample(10.0) * x * x);
}

template<typename Sample> inline Sample shaperCubicExpDecayAbs(Sample x)
{
  // Solve x for: diff(x^3*exp(-x), x) = 0,
  // then we get: x = 0, 27 * math.exp(-3).
  // 0.7439087749328765 = 1 / (27 * math.exp(-3))
  return 0.7439087749328765 * x * x * x * someexp<Sample>(-somefabs<Sample>(x));
}

template<typename Sample> inline Sample shaperHardclip(Sample x)
{
  return x < Sample(-1.0) ? Sample(-1.0) : x > Sample(1.0) ? Sample(1.0) : x;
}

enum class ShaperType { hardclip, tanh, sinRunge, cubicExpDecayAbs };

template<typename Sample, typename Filter, size_t nFilter> class SerialFilter {
public:
  Sample feedback = 0.0;
  Sample saturation = 1.0;
  ShaperType shaper = ShaperType::sinRunge;

  SerialFilter(Sample sampleRate, Sample cutoff, Sample resonance)
  {
    for (size_t i = 0; i < filter.size(); ++i)
      filter[i] = std::make_unique<Filter>(sampleRate, cutoff, resonance);
  }

  void reset()
  {
    for (size_t i = 0; i < filter.size(); ++i) filter[i]->reset();
  }

  void clear()
  {
    output = 0.0;
    for (size_t i = 0; i < filter.size(); ++i) filter[i]->clear();
  }

  void setType(BiquadType type)
  {
    for (size_t i = 0; i < filter.size(); ++i) filter[i]->type = type;
  }

  void setCutoffQ(Sample hz, Sample q)
  {
    filter[0]->setCutoffQ(hz, q);
    for (size_t i = 1; i < filter.size(); ++i) filter[i]->copyCoefficient(filter[0]);
  }

  Sample process(Sample input)
  {
    switch (shaper) {
      default:
      case ShaperType::hardclip:
        output = shaperHardclip<Sample>(saturation * (input - feedback * output));
        break;

      case ShaperType::tanh:
        output = sometanh<Sample>(saturation * (input - feedback * output));
        break;

      case ShaperType::sinRunge:
        output = shaperSinRunge<Sample>(saturation * (input - feedback * output));
        break;

      case ShaperType::cubicExpDecayAbs:
        output = shaperCubicExpDecayAbs<Sample>(saturation * (input - feedback * output));
        break;
    }
    for (size_t i = 0; i < filter.size(); ++i) output = filter[i]->process(output);
    return output;
  }

protected:
  Sample output = 0.0;
  std::array<std::unique_ptr<Filter>, nFilter> filter;
};

} // namespace SomeDSP
