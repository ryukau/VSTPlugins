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

#include "constants.hpp"
#include "smoother.hpp"
#include "somemath.hpp"

#include <algorithm>

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return 0.5 * (1.0 - somecos<Sample>(pi * t));
}

template<typename Sample> class ExpDecay {
public:
  bool isTerminated = true;

  void setup(
    Sample sampleRate,
    Sample decayTime = 1.0,
    Sample declickTime = 0.001,
    Sample threshold = 1e-5)
  {
    declickLength = declickTime * sampleRate;
    this->sampleRate = sampleRate;
    this->threshold = threshold;

    setDecayTime(decayTime);
  }

  void reset(Sample decayTime)
  {
    setDecayTime(decayTime);
    if (!isTerminated) declickOffset = output;
    isTerminated = false;
    declickCounter = 0;
    value = Sample(1.0);
  }

  Sample process()
  {
    if (isTerminated) return Sample(0);

    value *= alpha;
    output = value - threshold;

    if (declickCounter < declickLength) {
      output *= declickOffset
        + (Sample(1) - declickOffset)
          * cosinterp<Sample>(Sample(declickCounter) / declickLength);
      ++declickCounter;
    } else if (output <= Sample(0)) {
      isTerminated = true;
    }

    return output;
  }

protected:
  void setDecayTime(Sample decayTime)
  {
    if (decayTime * sampleRate <= declickLength)
      alpha = threshold;
    else
      alpha = somepow<Sample>(
        threshold, Sample(1.0) / (decayTime * sampleRate - declickLength));
  }

  int32_t declickLength = 0;
  int32_t declickCounter = 0;
  Sample declickOffset = 0;

  Sample sampleRate = 44100;
  Sample alpha = Sample(0);
  Sample threshold = 1e-5;
  Sample value = Sample(1);
  Sample output = Sample(0);
};

} // namespace SomeDSP
