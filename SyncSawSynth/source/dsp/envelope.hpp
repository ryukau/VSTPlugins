// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <cmath>

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return Sample(0.5) * (Sample(1) - std::cos(Sample(pi) * t));
}

// When using float, time will be shorten.
// env(t) := exp(-beta * t)
template<typename Sample> class ExpADSREnvelope {
public:
  // attackTime, decayTime, releaseTime and declickTime are in seconds.
  // sustainLevel in [0, 1]. 0.0 < threshold < 1.0.
  ExpADSREnvelope(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = Sample(0.001),
    Sample threshold = Sample(1e-5))
    : sampleRate(sampleRate)
  {
    reset(attackTime, decayTime, sustainLevel, releaseTime, declickTime, threshold);
  }

  void reset(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = Sample(0.001),
    Sample threshold = Sample(1e-5))
  {
    state = State::attack;
    value = threshold;
    sustain.reset(sustainLevel);
    set(attackTime, decayTime, sustainLevel, releaseTime, declickTime, threshold);
  }

  // This method is slow.
  void set(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample declickTime = Sample(0.001),
    Sample threshold = Sample(1e-5))
  {
    this->threshold = threshold;

    const auto sampleLength = Sample(4.0) / sampleRate;

    if (attackTime < sampleLength) attackTime = sampleLength;

    this->decayTime = (decayTime < sampleLength) ? sampleLength : decayTime;

    sustainLevel = std::max<Sample>(0.0, std::min<Sample>(sustainLevel, Sample(1.0)));
    sustain.push(sustainLevel);

    declickLength = int32_t(declickTime * sampleRate);

    if (releaseTime * sampleRate <= declickLength)
      releaseAlpha = threshold;
    else
      releaseAlpha
        = std::pow(threshold, Sample(1.0) / (releaseTime * sampleRate - declickLength));

    switch (state) {
      case State::attack:
        alpha
          = std::pow(Sample(1.0) / threshold, Sample(1.0) / (attackTime * sampleRate));
        break;

      case State::decay:
        alpha = std::pow(threshold, Sample(1.0) / (decayTime * sampleRate));
        break;

      case State::release:
        alpha = releaseAlpha;
        break;

      default:
        break;
    }
  }

  void release()
  {
    switch (state) {
      case State::attack:
        releaseRange = value;
        break;

      case State::decay:
        releaseRange = value - value * sustain.getValue() + sustain.getValue();
        break;

      case State::terminated:
        return;

      default:
        releaseRange = sustain.getValue();
        break;
    }

    value = Sample(1.0);
    alpha = releaseAlpha;
    state = State::release;
  }

  void terminate()
  {
    state = State::terminated;
    declickCounter = 0;
    releaseRange = 1;
    value = 0;
    sustain.reset(0);
  }

  bool isAttacking() { return state == State::attack; }
  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    sustain.process();

    Sample output;
    switch (state) {
      case State::attack:
        value *= alpha;
        if (value >= Sample(1.0)) {
          value = Sample(1.0);
          state = State::decay;
          alpha = std::pow(threshold, Sample(1.0) / (decayTime * sampleRate));
        }
        output = value;
        break;

      case State::decay:
        value *= alpha;
        output = value - value * sustain.getValue() + sustain.getValue();
        if (output > sustain.getValue() + threshold) break;
        state = State::sustain;
        break;

      case State::sustain:
        return sustain.getValue();

      case State::release:
        value *= alpha;
        if (value > threshold) {
          output = value * releaseRange;
          break;
        }
        value *= releaseRange;
        state = State::declickOut;
        output = value;
        break;

      case State::declickOut:
        value *= alpha;
        declickCounter -= 1;
        if (declickCounter > 0)
          return value * cosinterp<Sample>(declickCounter / (Sample)declickLength);
        value = 0.0;
        state = State::terminated;
        return value;

      default:
        output = 0;
        break;
    }

    if (state != State::declickOut && declickCounter < declickLength) {
      declickCounter += 1;
      output *= cosinterp<Sample>(declickCounter / (Sample)declickLength);
    }

    return output;
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, declickOut, terminated };

  int32_t declickLength;
  int32_t declickCounter = 0;

  State state = State::terminated;
  Sample sampleRate;
  Sample decayTime;
  Sample releaseAlpha;
  Sample releaseRange = 1;
  Sample alpha;
  Sample value;
  Sample threshold = Sample(1e-5);
  LinearSmoother<Sample> sustain;
};

template<typename Sample> class LinearEnvelope {
public:
  LinearEnvelope(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime)
    : sampleRate(sampleRate)
  {
    reset(attackTime, decayTime, sustainLevel, releaseTime);
  }

  void reset(Sample attackTime, Sample decayTime, Sample sustainLevel, Sample releaseTime)
  {
    set(attackTime, decayTime, sustainLevel, releaseTime);
    state = State::attack;
  }

  void set(Sample attackTime, Sample decayTime, Sample sustainLevel, Sample releaseTime)
  {
    sustain = std::clamp(sustainLevel, Sample(0), Sample(1));
    decayRange = Sample(1) - sustain;

    attackDelta = Sample(1) / attackTime / sampleRate;
    decayDelta = Sample(1) / decayTime / sampleRate;
    releaseDelta = Sample(1) / releaseTime / sampleRate;
  }

  void release()
  {
    if (state == State::terminated) return;
    state = State::release;
    releaseRange = value;
  }

  void terminate()
  {
    state = State::terminated;
    value = 0;
  }

  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    switch (state) {
      case State::attack:
        value += attackDelta;
        if (value >= 1.0) {
          state = State::decay;
          value = 1.0;
        }
        return value;

      case State::decay:
        value -= decayDelta * decayRange;
        if (value <= sustain) {
          state = State::sustain;
          value = sustain;
        }
        return value;

      case State::sustain:
        return sustain;

      case State::release:
        value -= releaseDelta * releaseRange;
        if (value < 0.0) {
          state = State::terminated;
          value = 0.0;
        }
        return value;

      default:
        break;
    }
    return 0.0;
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, terminated };
  State state = State::terminated;
  Sample value = 0;
  Sample sampleRate;
  Sample sustain;
  Sample attackDelta;
  Sample decayDelta;
  Sample decayRange;
  Sample releaseDelta;
  Sample releaseRange;
};

// Almost always use double. float may be used if attack and curve is small.
// env(t) := t^alpha * exp(-beta * t)
// TODO: Find optimal range of alpha and beta.
template<typename Sample> class PolyExpEnvelope {
public:
  // attack is in seconds. curve is arbitrary value.
  PolyExpEnvelope(Sample sampleRate, Sample attack, Sample curve) : sampleRate(sampleRate)
  {
    reset(attack, curve);
  }

  bool isReleasing() { return time >= attack; }

  void reset(Sample attack, Sample curve)
  {
    alpha = attack * curve;
    peak = std::pow(alpha / curve, alpha) * std::exp(-alpha);
    gamma = std::exp(-curve / sampleRate);
    tick = 1.0 / sampleRate;

    time = 0.0;
    value = 1.0;
  }

  void terminate()
  {
    value = 0;
    time = 0;
  }

  Sample process()
  {
    auto output = std::pow(time, alpha) * value / peak;
    if (!std::isfinite(output)) return 0.0; // TODO
    time += tick;
    value *= gamma;
    return output;
  }

protected:
  Sample sampleRate;
  Sample value;
  Sample peak;
  Sample gamma;
  Sample attack;
  Sample tick;
  Sample alpha;
  Sample time;
};

} // namespace SomeDSP
