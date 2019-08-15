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

} // namespace SomeDSP
