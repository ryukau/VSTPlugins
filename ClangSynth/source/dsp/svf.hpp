// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include <cmath>

namespace SomeDSP {

/**
Translation of SVF in Faust filter.lib.
https://faustlibraries.grame.fr/libs/filters/#svf-filters

Arguments of `process`.
- fs             : Sampling rate.
- v0             : Input.
- normalizedFreq : Normalized frequency. In theory, range is in [0, 1).
- Q              : Quality factor, or resonance.
- shelvingGaindB : Gain for bell and shelving filters.

List of `type`.
- 0: LP
- 1: BP
- 2: HP
- 3: Notch
- 4: Peak
- 5: AP
*/
template<typename Sample, size_t type> class GenericSVF {
private:
  Sample ic1eq = Sample(0);
  Sample ic2eq = Sample(0);

  Sample g = 0;
  Sample k = 0;

public:
  GenericSVF() { static_assert(type <= 5, "SVF type must be less than or equal to 5."); }

  // normalizedFreq = cutoffHz / sampleRate.
  void setup(Sample normalizedFreq, Sample Q)
  {
    g = std::tan(normalizedFreq * Sample(pi));
    k = Sample(1) / Q;
  }

  void reset()
  {
    ic1eq = 0;
    ic2eq = 0;
  }

  Sample process(Sample v0)
  {
    // tick.
    auto v1 = (ic1eq + g * (v0 - ic2eq)) / (Sample(1) + g * (g + k));
    auto v2 = ic2eq + g * v1;

    ic1eq = Sample(2) * v1 - ic1eq;
    ic2eq = Sample(2) * v2 - ic2eq;

    // Mix.
    if constexpr (type == 0) {
      return v2;
    } else if (type == 1) {
      return v1;
    } else if (type == 2) {
      return v0 - k * v1 - v2;
    } else if (type == 3) {
      return v0 - k * v1;
    } else if (type == 4) {
      return v0 - k * v1 - Sample(2) * v2;
    } else if (type == 5) {
      return v0 - Sample(2) * k * v1;
    }
    return Sample(0); // Shouldn't reach here.
  }
};

template<typename Sample, size_t length> class ParallelSVF {
private:
  std::array<Sample, length> ic1eq{};
  std::array<Sample, length> ic2eq{};

  Sample g = 0;
  Sample k = 0;
  Sample denom = Sample(1);

public:
  void setCutoff(Sample normalizedFreq, Sample Q)
  {
    g = std::tan(normalizedFreq * Sample(pi));
    k = Sample(1) / Q;
    denom = Sample(1) / (Sample(1) + g * (g + k));
  }

  void reset()
  {
    ic1eq.fill(0);
    ic2eq.fill(0);
  }

  void lowpass(std::array<Sample, length> &v0)
  {
    for (size_t n = 0; n < length; ++n) {
      auto v1 = (ic1eq[n] + g * (v0[n] - ic2eq[n])) * denom;
      auto v2 = ic2eq[n] + g * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] = v2;
    }
  }

  void highpass(std::array<Sample, length> &v0)
  {
    for (size_t n = 0; n < length; ++n) {
      auto v1 = (ic1eq[n] + g * (v0[n] - ic2eq[n])) * denom;
      auto v2 = ic2eq[n] + g * v1;
      ic1eq[n] = Sample(2) * v1 - ic1eq[n];
      ic2eq[n] = Sample(2) * v2 - ic2eq[n];
      v0[n] -= k * v1 + v2;
    }
  }
};

} // namespace SomeDSP
