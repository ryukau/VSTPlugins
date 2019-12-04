// (c) 2019 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

#include "constants.hpp"
#include "somemath.hpp"

namespace SomeDSP {

template<typename Sample> class SVFAllpass {
public:
  SVFAllpass(Sample sampleRate, Sample cutoff, Sample resonance)
    : sampleRate(sampleRate), cutoff(cutoff), resonance(resonance)
  {
  }

  // q in (0, 1].
  void setCutoffQ(Sample cutoffHz, Sample q)
  {
    cutoff = cutoffHz > 0.0 ? cutoffHz : 0.0;
    resonance = q < Sample(1e-5) ? Sample(1e-5) : q;

    Sample omega_c = sometan<Sample>(Sample(pi) * cutoff / sampleRate);
    g = omega_c / (Sample(1.0) + omega_c);
    twoR = 2 * resonance;
    g1 = twoR + g;
    d = Sample(1.0) / (Sample(1.0) + Sample(2.0) * resonance + g * g);
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
  Sample sampleRate = 44100;
  Sample cutoff = 100;
  Sample resonance = 0.5;

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
  void setup(Sample sampleRate) { fs = sampleRate; }

  void reset()
  {
    b0 = b1 = b2 = 0.0;
    a0 = a1 = a2 = 0.0;
    x1 = x2 = 0.0;
    y1 = y2 = 0.0;
  }

  // q in (0, 1].
  void setCutoffQ(Sample cutoffHz, Sample q)
  {
    f0 = cutoffHz >= Sample(1.0) ? cutoffHz : Sample(1.0);
    this->q = q < Sample(1e-5) ? Sample(1e-5) : q;

    Sample w0 = Sample(twopi) * f0 / fs;
    Sample cos_w0 = somecos<Sample>(w0);
    Sample sin_w0 = somesin<Sample>(w0);

    Sample alpha = sin_w0 / (Sample(2.0) * this->q);
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

    if (std::isfinite(output)) return output;
    reset();
    return output;
  }

protected:
  Sample fs = 44100;
  Sample f0 = 100;
  Sample q = 0.1;

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
