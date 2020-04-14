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
#include "../../../common/dsp/somemath.hpp"

#include <algorithm>

namespace SomeDSP {

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
    Sample absed = somefabs(x0 * drive);

    Sample y2 = somefmod(absed, Sample(2)) - Sample(1);
    y2 *= y2;

    Sample expo = y2;
    for (uint8_t i = 0; i < order; ++i) expo *= y2;
    if (inverse) expo = Sample(1) / (Sample(1) + expo);
    if (flip) expo = Sample(1) - expo;

    Sample output = somecopysign(somepow(absed, expo), x0);
    if (!inverse) output /= drive;

    return std::isfinite(output) ? output : 0;
  }

  Sample process16(Sample x0)
  {
    Sample diff = x0 - x1;
    lowpass.push(process(x1));
    lowpass.push(process(x1 + Sample(0.0625) * diff));
    lowpass.push(process(x1 + Sample(0.1250) * diff));
    lowpass.push(process(x1 + Sample(0.1875) * diff));
    lowpass.push(process(x1 + Sample(0.2500) * diff));
    lowpass.push(process(x1 + Sample(0.3125) * diff));
    lowpass.push(process(x1 + Sample(0.3750) * diff));
    lowpass.push(process(x1 + Sample(0.4375) * diff));
    lowpass.push(process(x1 + Sample(0.5000) * diff));
    lowpass.push(process(x1 + Sample(0.5625) * diff));
    lowpass.push(process(x1 + Sample(0.6250) * diff));
    lowpass.push(process(x1 + Sample(0.6875) * diff));
    lowpass.push(process(x1 + Sample(0.7500) * diff));
    lowpass.push(process(x1 + Sample(0.8125) * diff));
    lowpass.push(process(x1 + Sample(0.8750) * diff));
    lowpass.push(process(x1 + Sample(0.9375) * diff));
    x1 = x0;
    if (std::isfinite(lowpass.output())) return lowpass.output();

    reset();
    return 0;
  }
};

} // namespace SomeDSP
