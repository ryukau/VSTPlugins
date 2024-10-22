// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/multirate.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> inline Sample safeClip(Sample input)
{
  return std::isfinite(input) ? std::clamp<Sample>(input, Sample(-1024), Sample(1024))
                              : 0;
}

template<typename Sample> class OddPowShaper {
public:
  Sample drive = 1; // Must be greater than 0.
  size_t order = 0; // exponential = 2 * (1 + order).
  bool flip = false;
  bool inverse = false;

  FirUpSampler<Sample, Fir16FoldUpSample<Sample>> upSampler;
  DecimationLowpass<Sample, Sos16FoldFirstStage<Sample>> lowpass;
  HalfBandIIR<Sample, HalfBandCoefficient<Sample>> halfbandIir;

  size_t latency() { return Fir16FoldUpSample<Sample>::intDelay; }

  void reset()
  {
    upSampler.reset();
    lowpass.reset();
    halfbandIir.reset();
  }

  // Only used for GUI.
  void set(Sample drive, size_t order, bool flip, bool inverse)
  {
    this->drive = drive;
    this->order = order;
    this->flip = flip;
    this->inverse = inverse;
  }

  Sample process(Sample x0)
  {
    Sample absed = std::fabs(x0 * drive);

    Sample y2 = std::fmod(absed, Sample(2)) - Sample(1);
    y2 *= y2;

    Sample expo = y2;
    for (size_t i = 0; i < order; ++i) expo *= y2;
    if (inverse) expo = Sample(1) / (Sample(1) + expo);
    if (flip) expo = Sample(1) - expo;

    Sample output = std::copysign(std::pow(absed, expo), x0);
    if (!inverse) output /= drive;

    return safeClip(output);
  }

  Sample process16(Sample x0)
  {
    upSampler.process(x0);

    std::array<Sample, 2> halfBandInput;
    for (size_t i = 0; i < 8; ++i) lowpass.push(process(upSampler.output[i]));
    halfBandInput[0] = lowpass.output();
    for (size_t i = 8; i < 16; ++i) lowpass.push(process(upSampler.output[i]));
    halfBandInput[1] = lowpass.output();

    return halfbandIir.process(halfBandInput);
  }
};

} // namespace SomeDSP
