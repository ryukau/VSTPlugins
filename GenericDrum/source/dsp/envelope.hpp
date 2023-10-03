// (c) 2023 Takamitsu Endo
//
// This file is part of GenericDrum.
//
// GenericDrum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenericDrum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenericDrum.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../../../common/dsp/solver.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace SomeDSP {

// Non-recursive form of DoubleEMAEnvelope output. Negated because `minimizeScalarBrent`
// finds minimum.
template<typename T> T doubleEmaEnvelopeD0Negative(T n, T k_A, T k_D)
{
  auto A = std::pow(T(1) - k_A, n + T(1)) * (k_A * n + k_A + T(1));
  auto D = std::pow(T(1) - k_D, n + T(1)) * (k_D * n + k_D + T(1));
  return (A - T(1)) * D;
}

template<typename T> T samplesToKp(T timeInSamples)
{
  if (timeInSamples < std::numeric_limits<T>::epsilon()) return T(1);
  auto y = T(1) - std::cos(T(twopi) / timeInSamples);
  return -y + std::sqrt(y * (y + T(2)));
}

template<typename Sample> class DoubleEmaADEnvelope {
private:
  Sample v1_A = 0;
  Sample v2_A = 0;

  Sample v1_D = 0;
  Sample v2_D = 0;

  Sample k_A = Sample(1);
  Sample k_D = Sample(1);

  Sample gain = 0; // Gain to normalize peak to 1.

  inline void initState()
  {
    v1_A = 0;
    v2_A = 0;
    v1_D = Sample(1);
    v2_D = Sample(1);
  }

public:
  void reset()
  {
    initState();
    gain = 0;
  }

  void noteOn(Sample targetAmplitude, Sample attackTimeSamples, Sample decayTimeSamples)
  {
    // Using `double` for minimization. `float` is inaccurate over 10^4 samples.
    auto kA = samplesToKp<double>(attackTimeSamples);
    auto kD = samplesToKp<double>(decayTimeSamples);

    if (kA == 1.0 || kD == 1.0) {
      gain = Sample(1);
      k_A = Sample(kA);
      k_D = Sample(kD);
    } else {
      auto result = minimizeScalarBrent<double>(
        [&](double n) { return doubleEmaEnvelopeD0Negative<double>(n, kA, kD); });

      auto peak = -result.second;
      gain
        = peak < std::numeric_limits<double>::epsilon() ? Sample(1) : Sample(1.0 / peak);
      k_A = Sample(kA);
      k_D = Sample(kD);
    }
    gain *= targetAmplitude;

    initState();
  }

  Sample process()
  {
    v1_A += k_A * (Sample(1) - v1_A);
    v2_A += k_A * (v1_A - v2_A);

    v1_D += k_D * (Sample(0) - v1_D);
    v2_D += k_D * (v1_D - v2_D);

    return gain * v2_A * v2_D;
  }
};

/**
Use with DoubleEmaADEnvelope.

When note-on comes in, and DoubleEmaADEnvelope has to reset, move the current value of
DoubleEmaADEnvelope to TransitionReleaseSmoother using `prepare()`. `process()` output of
DoubleEmaADEnvelope and TransitionReleaseSmoother are summed together.
*/
template<typename Sample> class TransitionReleaseSmoother {
private:
  Sample v0 = 0;
  Sample decay = 0;

public:
  // decaySamples = sampleRate * seconds.
  void setup(Sample decaySamples)
  {
    decay = std::pow(std::numeric_limits<Sample>::epsilon(), Sample(1) / decaySamples);
  }

  void reset() { v0 = 0; }
  void prepare(Sample value) { v0 += value; }
  Sample process() { return v0 *= decay; }
};

} // namespace SomeDSP
