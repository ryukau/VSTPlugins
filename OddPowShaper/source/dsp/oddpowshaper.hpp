// (c) 2020 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../common/dsp/decimationLowpass.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> inline Sample safeClip(Sample input)
{
  return std::isfinite(input) ? std::clamp<Sample>(input, Sample(-128), Sample(128)) : 0;
}

template<typename Sample> class OddPowShaper {
public:
  Sample drive = 1;  // Must be greater than 0.
  uint8_t order = 0; // exponential = 2 * (1 + order).
  bool flip = false;
  bool inverse = false;

  Sample x1 = 0;
  DecimationLowpass16<Sample> lowpass;

  void reset()
  {
    x1 = 0;
    lowpass.reset();
  }

  Sample process(Sample x0)
  {
    Sample absed = std::fabs(x0 * drive);

    Sample y2 = std::fmod(absed, Sample(2)) - Sample(1);
    y2 *= y2;

    Sample expo = y2;
    for (uint8_t i = 0; i < order; ++i) expo *= y2;
    if (inverse) expo = Sample(1) / (Sample(1) + expo);
    if (flip) expo = Sample(1) - expo;

    Sample output = std::copysign(std::pow(absed, expo), x0);
    if (!inverse) output /= drive;

    return safeClip(output);
  }

  Sample process16(Sample x0)
  {
    Sample diff = x0 - x1;
    for (int i = 0; i < 16; ++i) lowpass.push(process(x1 + i / Sample(16) * diff));
    x1 = x0;
    if (std::isfinite(lowpass.output())) return lowpass.output();

    reset();
    return 0;
  }
};

} // namespace SomeDSP
