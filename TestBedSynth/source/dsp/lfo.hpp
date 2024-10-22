// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace SomeDSP {

// Integer sample delay.
template<typename Sample, size_t length> class FixedIntDelay {
private:
  std::array<Sample, length> buf{};
  size_t wptr = 0;
  size_t rptr = 0;

public:
  void reset()
  {
    buf.fill({});
    wptr = 0;
    rptr = 0;
  }

  void setFrames(size_t delayFrames)
  {
    if (delayFrames >= length) delayFrames = length;
    rptr = wptr - delayFrames;
    if (rptr >= length) rptr += length; // Unsigned negative overflow case.
  }

  Sample process(Sample input)
  {
    if (++wptr >= buf.size()) wptr = 0;
    buf[wptr] = input;

    if (++rptr >= buf.size()) rptr = 0;
    return buf[rptr];
  }
};

template<typename Sample> struct EMAHighpass {
  Sample v1 = 0;

  void reset(Sample value = 0) { v1 = value; }

  Sample process(Sample input, Sample kp)
  {
    v1 += kp * (input - v1);
    return input - v1;
  }
};

template<typename Sample, size_t tableSize> class EasyLfo {
private:
  Sample phase = 0;

public:
  DoubleEMAFilter<Sample> smoother;

  void reset()
  {
    phase = 0;
    smoother.reset();
  }

  Sample process(
    Sample phaseDelta, Sample smootherKp, const std::array<Sample, tableSize> &table)
  {
    phase += phaseDelta;
    phase -= std::floor(phase);
    return smoother.processKp(table[size_t(Sample(tableSize) * phase)], smootherKp);
  }
};

template<typename Sample> class ADSREnvelope {
public:
  enum class State { attack, decay, release, terminated };

  State state = State::terminated;
  size_t attackLength = 0;
  size_t counter = 0;
  Sample atkKp = 1;
  Sample decKp = 1;
  Sample relKp = 1;
  DoubleEMAFilter<Sample> smoother;

  Sample value() { return smoother.v2; }

  bool isAttacking() { return state == State::attack; }
  bool isTerminated() { return state == State::terminated; }

  void reset()
  {
    state = State::terminated;
    smoother.reset();
  }

  void noteOn(size_t attackSamples)
  {
    state = State::attack;
    attackLength = attackSamples;
    counter = 0;
    smoother.reset();
  }

  void noteOff() { state = State::release; }

  void prepare(Sample attackKp, Sample decayKp, Sample releaseKp)
  {
    atkKp = attackKp;
    decKp = decayKp;
    relKp = releaseKp;
  }

  Sample process(Sample sustainAmplitude)
  {
    if (state == State::attack) {
      if (++counter >= attackLength) state = State::decay;
      const auto ramp = Sample(counter) / Sample(attackLength / 2);
      return smoother.processKp(std::min(ramp, Sample(1)), atkKp);
    } else if (state == State::decay) {
      return smoother.processKp(sustainAmplitude, decKp);
    } else if (state == State::release) {
      constexpr Sample eps = Sample(std::numeric_limits<float>::epsilon());
      if (smoother.v2 <= eps) {
        state = State::terminated;
        smoother.reset();
      }
      return smoother.processKp(Sample(0), relKp);
    }
    return 0;
  }
};

} // namespace SomeDSP
