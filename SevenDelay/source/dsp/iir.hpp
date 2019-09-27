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

inline double somesin(double x) { return ::sin(x); }

inline float somesin(float x) { return ::sinf(x); }

inline double somecos(double x) { return ::cos(x); }

inline float somecos(float x) { return ::cosf(x); }

inline double sometan(double x) { return ::tan(x); }

inline float sometan(float x) { return ::tanf(x); }

template<typename Sample> class SVF {
public:
  SVF(Sample sampleRate, Sample cutoff, Sample resonance)
    : sampleRate(sampleRate), cutoff(cutoff), resonance(resonance)
  {
  }

  void setCoefficient()
  {
    Sample omega_c = sometan(Sample(pi) * cutoff / sampleRate);
    g = omega_c / (Sample(1.0) + omega_c);
    twoR = 2 * resonance;
    g1 = twoR + g;
    d = Sample(1.0) / (Sample(1.0) + Sample(2.0) * resonance + g * g);
  }

  void setCutoff(Sample hz)
  {
    cutoff = hz > 0.0 ? hz : 0.0;
    setCoefficient();
  }

  // value is (0, 1].
  void setQ(Sample value)
  {
    resonance = value < Sample(1e-5) ? Sample(1e-5) : value;
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

    Sample output = input - Sample(2.0) * twoR * yBP;
    if (std::isfinite(output)) return output;

    reset();
    return 0.0;
  }

protected:
  Sample sampleRate;
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
};

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
template<typename Sample> class BiquadHighPass {
public:
  BiquadHighPass(Sample sampleRate, Sample cutoff, Sample q)
    : fs(sampleRate), f0(cutoff), q(q)
  {
  }

  void reset()
  {
    b0 = b1 = b2 = 0.0;
    a0 = a1 = a2 = 0.0;
    x1 = x2 = 0.0;
    y1 = y2 = 0.0;
  }

  void setCutoff(Sample hz)
  {
    f0 = hz > 0.0 ? hz : 0.0;
    setCoefficient();
  }

  // value is (0, 1].
  void setQ(Sample value)
  {
    q = value < Sample(1e-5) ? Sample(1e-5) : value;
    setCoefficient();
  }

  void setCoefficient()
  {
    Sample w0 = Sample(twopi) * f0 / fs;
    Sample cos_w0 = somecos(w0);
    Sample sin_w0 = somesin(w0);

    Sample alpha = sin_w0 / (Sample(2.0) * q);
    b0 = (Sample(1.0) + cos_w0) / Sample(2.0);
    b1 = -(Sample(1.0) + cos_w0);
    b2 = (Sample(1.0) + cos_w0) / Sample(2.0);
    a0 = Sample(1.0) + alpha;
    a1 = -Sample(2.0) * cos_w0;
    a2 = Sample(1.0) - alpha;

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
  Sample f0;
  Sample q;

  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample a0 = 0.0;
  Sample a1 = 0.0;
  Sample a2 = 0.0;
  Sample x1 = 0.0;
  Sample x2 = 0.0;
  Sample y1 = 0.0;
  Sample y2 = 0.0;
};

} // namespace SomeDSP
