// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return Sample(0.5) * (Sample(1) - std::cos(Sample(pi) * t));
}

template<typename Sample> class ExpDecay {
public:
  bool isTerminated = true;

  void setup(
    Sample sampleRate,
    Sample decayTime = Sample(1),
    Sample declickTime = Sample(0.001),
    Sample threshold = Sample(1e-5))
  {
    declickLength = int(declickTime * sampleRate);
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

  void terminate(Sample decayTime)
  {
    isTerminated = true;
    setDecayTime(decayTime);
    declickCounter = 0;
    declickOffset = 0;
    value = Sample(1);
    output = 0;
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
      alpha = std::pow(threshold, Sample(1.0) / (decayTime * sampleRate - declickLength));
  }

  int declickLength = 0;
  int declickCounter = 0;
  Sample declickOffset = 0;

  Sample sampleRate = 44100;
  Sample alpha = Sample(0);
  Sample threshold = Sample(1e-5);
  Sample value = Sample(1);
  Sample output = Sample(0);
};

} // namespace SomeDSP
