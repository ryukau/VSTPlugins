// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <memory>

#include "constants.hpp"
#include "somemath.hpp"

namespace SomeDSP {

// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html
template<typename Sample> struct ResonantFilter {
  Sample buf0 = 0;
  Sample buf1 = 0;
  Sample feedback = 0;
  Sample freq = 0;

  void reset()
  {
    buf0 = 0;
    buf1 = 0;
  }

  // cutoff and q are in [0, 1].
  void setCutoffQ(Sample cutoff, Sample q)
  {
    freq = cutoff;
    feedback = q + q / (Sample(1) - freq);
  }

  Sample process(Sample input)
  {
    buf0 += freq * (input - buf0 + feedback * (buf0 - buf1));
    buf1 += freq * (buf0 - buf1);
    return buf1;
  }
};

} // namespace SomeDSP
