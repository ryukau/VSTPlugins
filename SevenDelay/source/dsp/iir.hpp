// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include <cmath>

namespace SomeDSP {

constexpr double pi = 3.14159265358979323846;
constexpr double twopi = 6.28318530717958647692;

enum class ZDF1PoleType {
  lowpass,
  highpass,
  allpass,
};

template <typename Sample>
class ZDF1Pole {
public:
  ZDF1Pole(Sample sampleRate, ZDF1PoleType type, Sample cutoff)
    : sampleRate(sampleRate), type(type)
  {
    setCutoff(cutoff);
    reset();
  }

  void setCutoff(Sample hz)
  {
    Sample omega_c = tan(pi * hz / sampleRate);
    g = omega_c / (1.0 + omega_c);

    switch (type) {
      default:
      case ZDF1PoleType::lowpass:
        break;

      case ZDF1PoleType::highpass:
        two_g = 2.0 * g;
        g_hp = 1.0 / (1.0 + g);
        break;

      case ZDF1PoleType::allpass:
        g_ap = 2.0 * g / (1.0 + g);
        break;
    }
  }

  void reset()
  {
    s = 0.0;
    y = 0.0;
  }

  Sample process(Sample input)
  {
    switch (type) {
      default:
      case ZDF1PoleType::lowpass:
        Sample v = (input - s) * g;
        y = s + v;
        s = y + v;
        return y;

      case ZDF1PoleType::highpass:
        y = (input - s) * g_hp;
        s += y * two_g;
        return y;

      case ZDF1PoleType::allpass:
        Sample xs = input - s;
        s += xs * g_ap;
        return s - xs;
    }
  }

protected:
  Sample sampleRate;
  ZDF1PoleType type;
  Sample s = 0.0;
  Sample y = 0.0;
  Sample g = 0.0;
  Sample g_hp = 0.0;
  Sample two_g = 0.0;
  Sample g_ap = 0.0;
};

enum class SVFType {
  lowpass,
  highpass,
  bandpass,
  notch,
  peaking,
  allpass,
  lowshelf,
  highshelf,
  bandshelf,
};

template <typename Sample>
class SVF {
public:
  SVF(Sample sampleRate, SVFType type, Sample cutoff, Sample resonance, Sample gain)
    : sampleRate(sampleRate), type(type), cutoff(cutoff), resonance(resonance)
  {
    setGain(gain);
  }

  void setCoefficient()
  {
    Sample omega_c = tan(pi * cutoff / sampleRate);
    g = omega_c / (1 + omega_c);
    twoR = 2 * resonance;
    g1 = twoR + g;
    d = 1 / (1 + 2 * resonance + g * g);
  }

  void setType(SVFType value)
  {
    type = value;
    setCoefficient();
  }

  void setCutoff(Sample hz)
  {
    cutoff = hz > 0.0 ? hz : 0.0;
    setCoefficient();
  }

  void setGain(Sample dB)
  {
    k = pow(10.0, dB / 20.0) - 1;
    setCoefficient();
  }

  // value is (0, 1].
  void setQ(Sample value)
  {
    q = value < 1e-5 ? 1e-5 : value;
    setCoefficient();
  }

  void reset()
  {
    yLP = 0.0;
    yBP = 0.0;
    yHP = 0.0;

    s1 = 0.0;
    s2 = 0.0;
  }

  Sample process(Sample input)
  {
    yHP = (input - g1 * s1 - s2) * d;

    Sample v1 = g * yHP;
    yBP = v1 + s1;
    s1 = yBP + v1;

    Sample v2 = g * yBP;
    yLP = v2 + s2;
    s2 = yLP + v2;

    switch (type) {
      default:
      case SVFType::lowpass:
        return yLP;

      case SVFType::highpass:
        return yHP;

      case SVFType::bandpass:
        return twoR * yBP;

      case SVFType::notch:
        return input - twoR * yBP;

      case SVFType::peaking:
        return yLP - yHP;

      case SVFType::allpass:
        return input - 2.0 * twoR * yBP;

      case SVFType::lowshelf:
        return input + k * yLP;

      case SVFType::highshelf:
        return input + k * yHP;

      case SVFType::bandshelf:
        return input + k * twoR * yBP;
    }
  }

protected:
  Sample sampleRate;
  SVFType type;
  Sample cutoff;
  Sample resonance;

  Sample yLP = 0.0;
  Sample yBP = 0.0;
  Sample yHP = 0.0;

  Sample s1 = 0.0;
  Sample s2 = 0.0;

  Sample g = 0.0;
  Sample g1 = 0.0;
  Sample d = 0.0;
  Sample twoR = 0.0;
  Sample k = 0.0;
};

enum class BiquadType {
  lowpass,
  highpass,
  bandpass,
  notch,
  peaking,
  allpass,
  lowshelf,
  highshelf,
};

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
template <typename Sample>
class Biquad {
public:
  Biquad(Sample sampleRate, BiquadType type, Sample cutoff, Sample q, Sample gain)
    : fs(sampleRate), type(type), f0(cutoff), q(q)
  {
    setGain(gain);
  }

  void clearBuffer()
  {
    b0 = 0;
    b1 = 0;
    b2 = 0;

    a0 = 0;
    a1 = 0;
    a2 = 0;

    x1 = 0;
    x2 = 0;

    y1 = 0;
    y2 = 0;
  }

  void setType(BiquadType value)
  {
    type = value;
    setCoefficient();
  }

  void setCutoff(Sample hz)
  {
    f0 = hz > 0.0 ? hz : 0.0;
    setCoefficient();
  }

  void setGain(Sample dB)
  {
    A = pow(10.0, gain / 40.0);
    setCoefficient();
  }

  // value is (0, 1].
  void setQ(Sample value)
  {
    q = value < 1e-5 ? 1e-5 : value;
    setCoefficient();
  }

  void setCoefficient()
  {
    Sample w0 = twopi * f0 / fs;
    Sample cos_w0 = cos(w0);
    Sample sin_w0 = sin(w0);

    switch (type) {
      default:
      case BiquadType::lowpass:
        Sample alpha = sin_w0 / (2.0 * q);
        b0 = (1.0 - cos_w0) / 2.0;
        b1 = 1.0 - cos_w0;
        b2 = (1.0 - cos_w0) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha;
        break;

      case BiquadType::highpass:
        Sample alpha = sin_w0 / (2.0 * q);
        b0 = (1.0 + cos_w0) / 2.0;
        b1 = -(1.0 + cos_w0);
        b2 = (1.0 + cos_w0) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha;
        break;

      case BiquadType::bandpass:
        Sample alpha = sin_w0 * sinh(log(2.0) / 2.0 * q * w0 / sin_w0);
        b0 = alpha;
        b1 = 0.0;
        b2 = -alpha;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha;
        break;

      case BiquadType::notch:
        Sample alpha = sin_w0 * sinh(log(2.0) / 2.0 * q * w0 / sin_w0);
        b0 = 1.0;
        b1 = -2.0 * cos_w0;
        b2 = 1.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha;
        break;

      case BiquadType::peaking:
        Sample alpha = sin_w0 * sinh(log(2.0) / 2.0 * q * w0 / sin_w0);
        b0 = 1.0 + alpha * A;
        b1 = -2.0 * cos_w0;
        b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha / A;
        break;

      case BiquadType::allpass:
        Sample alpha = sin_w0 * sinh(log(2.0) / 2.0 * q * w0 / sin_w0);
        b0 = 1.0 - alpha;
        b1 = -2.0 * cos_w0;
        b2 = 1.0 + alpha;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cos_w0;
        a2 = 1.0 - alpha;
        break;

      case BiquadType::lowshelf:
        Sample alpha = sin_w0 / 2.0 * sqrt((A + 1.0 / A) * (1.0 / q - 1.0) + 2.0);
        Sample two_sqrt_A_alpha
          = sin_w0 * sqrt((A * A + 1.0) * (1.0 / q - 1.0) + 2.0 * A);
        b0 = A * ((A + 1.0) - (A - 1.0) * cos_w0 + two_sqrt_A_alpha);
        b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos_w0);
        b2 = A * ((A + 1.0) - (A - 1.0) * cos_w0 - two_sqrt_A_alpha);
        a0 = (A + 1.0) + (A - 1.0) * cos_w0 + two_sqrt_A_alpha;
        a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos_w0);
        a2 = (A + 1.0) + (A - 1.0) * cos_w0 - two_sqrt_A_alpha;
        break;

      case BiquadType::highshelf:
        Sample alpha = sin_w0 / 2.0 * sqrt((A + 1.0 / A) * (1.0 / q - 1.0) + 2.0);
        Sample two_sqrt_A_alpha
          = sin_w0 * sqrt((A * A + 1.0) * (1.0 / q - 1.0) + 2.0 * A);
        b0 = A * ((A + 1.0) + (A - 1.0) * cos_w0 + two_sqrt_A_alpha);
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cos_w0);
        b2 = A * ((A + 1.0) + (A - 1.0) * cos_w0 - two_sqrt_A_alpha);
        a0 = (A + 1.0) - (A - 1.0) * cos_w0 + two_sqrt_A_alpha;
        a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cos_w0);
        a2 = (A + 1.0) - (A - 1.0) * cos_w0 - two_sqrt_A_alpha;
        break;
    }

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
  }

  Sample process(Sample input)
  {
    Sample output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    x2 = x1;
    x1 = input;

    y2 = y1;
    y1 = output;

    return output;
  }

protected:
  Sample fs;
  BiquadType type;
  Sample f0;
  Sample A;
  Sample q;

  Sample b0 = 0;
  Sample b1 = 0;
  Sample b2 = 0;

  Sample a0 = 0;
  Sample a1 = 0;
  Sample a2 = 0;

  Sample x1 = 0;
  Sample x2 = 0;

  Sample y1 = 0;
  Sample y2 = 0;
}; // namespace SomeDSP

} // namespace SomeDSP
