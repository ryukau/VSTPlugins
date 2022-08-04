// (c) 2022 Takamitsu Endo
//
// This file is part of ClangCymbal.
//
// ClangCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"

#include <cmath>
#include <numeric>
#include <random>

namespace SomeDSP {

// Basically `(S&H) * (noise)`.
template<typename Sample> class SampleAndHoldNoise {
private:
  Sample phase = 0;
  Sample gain = Sample(1);
  Sample decay = Sample(1);

public:
  void reset()
  {
    phase = 0;
    gain = Sample(1);
  }

  void setDecay(Sample timeInSample)
  {
    decay = timeInSample < Sample(1)
      ? 0
      : Sample(std::pow(std::numeric_limits<double>::epsilon(), 1.0 / timeInSample));
  }

  // `density` is inverse of average samples between impulses.
  Sample process(
    Sample jitter,
    Sample density,
    Sample envelope,
    Sample randomGain,
    Sample noisePulseRatio,
    pcg64 &rng)
  {
    jitter = std::clamp(jitter, float(0), float(1));
    noisePulseRatio = std::clamp(noisePulseRatio, float(0), float(1));

    auto noise = processNoise(jitter, density, envelope, randomGain, rng);
    auto pulse = processBlit(density);

    return noise + noisePulseRatio * (pulse - noise);
  }

  inline Sample processNoise(
    Sample jitter, Sample density, Sample envelope, Sample randomGain, pcg64 &rng)
  {
    // Standard deviation (sigma) is set to 1/3 to normalize amplitude to almost [-1, 1].
    // 99.7% of the value falls between -3 sigma and +3 sigma (68–95–99.7 rule).
    std::normal_distribution<Sample> normal(Sample(0), Sample(1) / Sample(3));
    std::uniform_real_distribution<Sample> uniform(Sample(1) - jitter, Sample(1));

    phase += uniform(rng) * density / envelope;
    if (phase >= Sample(1)) {
      phase -= std::floor(phase);
      gain = Sample(1) + randomGain * (normal(rng) - Sample(1));
    } else {
      gain *= decay;
    }
    return gain * normal(rng);
  }

  // Band limited impulse trains. Must be called after `processNoise()` to increment
  // phase.
  inline Sample processBlit(Sample density)
  {
    constexpr auto eps = std::numeric_limits<Sample>::epsilon();

    if (density < eps) return Sample(0);
    auto M = 2 * std::floor(1 / (2 * density)) + 1;
    auto x = M * phase;
    auto denom = std::sin(Sample(pi) * x / M);
    if (std::abs(denom) < eps) return Sample(1);
    return density * std::sin(Sample(pi) * x) / denom;
  }
};

} // namespace SomeDSP
