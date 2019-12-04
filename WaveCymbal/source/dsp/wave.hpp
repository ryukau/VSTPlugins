#pragma once

#include <array>

#include "../../../lib/juce_FastMathApproximations.h"
#include "constants.hpp"

namespace SomeDSP {

template<typename Sample, size_t maxLength> class Wave1D {
public:
  size_t length = 1;

  // damping ~= [0, 1). Blow up at large number.
  void setup(
    Sample sampleRate,
    size_t length,
    Sample damping,
    Sample pulsePosition,
    Sample pulseWidth)
  {
    this->sampleRate = sampleRate;
    set(length, damping, pulsePosition, pulseWidth);
    reset();
  }

  void set(size_t length, Sample damping, Sample pulsePosition, Sample pulseWidth)
  {
    this->length = length > maxLength ? maxLength : length;

    this->damping = somepow<Sample>(damping, 44100.0f / sampleRate);

    this->pulsePosition = size_t(pulsePosition * this->length);

    size_t pw = size_t(pulseWidth * this->length);
    this->pulseWidth = pw < 4 ? 4 : pw;

    // alpha = 0.5 when sampleRate = 44100.0
    // 0.5 = A^2 * dt^2
    // sqrt(0.5) / dt = A
    // A = 31183.409050326747
    // A = 972405000

    alpha = 972405000 / (sampleRate * sampleRate);
    beta = 2 - 2 * alpha;
  }

  void step()
  {
    auto tmp = wave2;
    wave2 = wave1;
    wave1 = wave0;
    wave0 = tmp;

    size_t end = length - 1;

    wave0[0] = damping * (alpha * (wave1[end] + wave1[1]) + beta * wave1[0] - wave2[0]);

    for (size_t i = 1; i < end; ++i) {
      wave0[i]
        = damping * (alpha * (wave1[i - 1] + wave1[i + 1]) + beta * wave1[i] - wave2[i]);
    }

    wave0[end]
      = damping * (alpha * (wave1[end - 1] + wave1[0]) + beta * wave1[end] - wave2[end]);
  }

  void reset()
  {
    wave0.fill(0);
    wave1.fill(0);
    wave2.fill(0);
  }

  Sample at(size_t index)
  {
    if (index < 0) return 0;
    if (index >= length) return 0;
    return wave0[index];
  }

  // Unsafe fast lookup.
  Sample &operator[](const size_t index) { return wave0[index]; }

  void pulse(Sample height)
  {
    // Hanning window.
    size_t index = size_t(pulsePosition - pulseWidth / Sample(2.0));
    while (index >= length) index += length;

    auto twoPi_N1 = Sample(twopi) / (pulseWidth - 1);
    height /= pulseWidth * Sample(0.5);
    for (size_t i = 0; i < pulseWidth; ++i) {
      wave0[index] += height
        * (Sample(1.0)
           - juce::dsp::FastMathApproximations::cos<Sample>(twoPi_N1 * i - Sample(pi)));
      index += 1;
      if (index >= length) index -= length;
    }
  }

  void process(Sample input)
  {
    if (input != 0) pulse(input);
    step();
  }

protected:
  Sample sampleRate = 44100;
  Sample damping = 0.9;

  Sample alpha = 0;
  Sample beta = 0;

  size_t pulseWidth = 0;
  size_t pulsePosition = 0;

  std::array<Sample, maxLength> wave0{};
  std::array<Sample, maxLength> wave1{};
  std::array<Sample, maxLength> wave2{};
};

} // namespace SomeDSP
