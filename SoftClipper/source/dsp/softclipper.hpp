// (c) 2020 Takamitsu Endo
//
// This file is part of SoftClipper.
//
// SoftClipper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoftClipper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoftClipper.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../common/dsp/decimationLowpass.hpp"
#include "../../../common/dsp/somemath.hpp"

#include <algorithm>

namespace SomeDSP {

template<typename Sample> class SoftClipper {
public:
  Sample order = 2; // Greater than 0.
  Sample clipY = 1; // Greater than 0.
  Sample ratio = 1; // In [0, 1].
  Sample slope = 0; // In [0, 1].

  Sample x1 = 0;
  DecimationLowpass16<Sample> lowpass;

  void reset()
  {
    x1 = 0;
    lowpass.reset();
  }

  void set(Sample clip, Sample order, Sample ratio, Sample slope)
  {
    this->clipY = clip;
    this->order = order;
    this->ratio = ratio;
    this->slope = slope;
  }

  Sample process(Sample x0)
  {
    Sample absed = somefabs(x0);

    Sample rc = clipY * ratio;
    if (absed <= rc) return x0;

    Sample xc = rc + order * (clipY - rc);
    Sample scale = (rc - clipY) / somepow(xc - rc, order);
    Sample xs = xc - somepow(-slope / (scale * order), Sample(1) / (order - Sample(1)));
    if (absed < xs) return somecopysign(clipY + scale * somepow(xc - absed, order), x0);

    return somecopysign(
      slope * (absed - xs) + clipY + scale * somepow(xc - xs, order), x0);
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
