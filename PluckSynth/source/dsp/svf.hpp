// (c) 2022 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

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
template<typename Sample, size_t type> class SVF {
private:
  Sample ic1eq = Sample(0);
  Sample ic2eq = Sample(0);

  Sample g = 0;
  Sample k = 0;

public:
  SVF() { static_assert(type <= 5, "SVF type must be less than or equal to 5."); }

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

template<typename Sample, size_t type, size_t nCascade> class CascadeSVF {
public:
  std::array<SVF<Sample, type>, nCascade> svf;

  void setup(Sample normalizedFreq, Sample Q)
  {
    for (auto &filt : svf) filt.setup(normalizedFreq, Q);
  }

  void reset()
  {
    for (auto &filt : svf) filt.reset();
  }

  Sample process(Sample v0)
  {
    for (auto &filt : svf) v0 = filt.process(v0);
    return v0;
  }
};

} // namespace SomeDSP
