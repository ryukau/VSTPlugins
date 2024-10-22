// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/multirate.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> class SoftClipper {
public:
  Sample order = 2; // Greater than 0.
  Sample clipY = 1; // Greater than 0.
  Sample ratio = 1; // In [0, 1].
  Sample slope = 0; // In [0, 1].

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
  void set(Sample clip, Sample order, Sample ratio, Sample slope)
  {
    this->clipY = std::max(Sample(1e-15), clip);
    this->order = std::max(Sample(1), order);
    this->ratio = std::min(ratio, Sample(0.999));
    this->slope = slope;
  }

  Sample process(Sample x0)
  {
    Sample absed = std::fabs(x0);

    Sample rc = clipY * ratio;
    if (absed <= rc) return x0;
    if (ratio >= Sample(1)) return std::copysign(clipY, x0);

    Sample xc = rc + order * (clipY - rc);
    Sample scale = (rc - clipY) / std::pow(xc - rc, order);
    Sample xs = xc - std::pow(-slope / (scale * order), Sample(1) / (order - Sample(1)));
    return absed < xs
      ? std::copysign(clipY + scale * std::pow(xc - absed, order), x0)
      : std::copysign(
          slope * (absed - xs) + clipY + scale * std::pow(xc - xs, order), x0);
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
