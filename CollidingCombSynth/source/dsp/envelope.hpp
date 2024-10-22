// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample> class ExpADSREnvelopeP {
public:
  void setup(Sample sampleRate) { tailLength = uint32_t(0.01 * sampleRate); }

  void prepare(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime)
  {
    state = State::attack;
    sustain = std::clamp<Sample>(sustainLevel, Sample(0), Sample(1));
    atk = int32_t(sampleRate * attackTime);
    decTime = decayTime;
    relTime = releaseTime;
    pController.setCutoff(sampleRate, Sample(1) / attackTime);
  }

  void reset()
  {
    state = State::terminated;
    pController.reset(0);
  }

  void set(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime)
  {
    switch (state) {
      case State::attack:
        atk = int32_t(sampleRate * attackTime);
        // Fall through.

      case State::decay:
        decTime = decayTime;
        sustain = std::clamp<Sample>(sustainLevel, Sample(0), Sample(1));
        // Fall through.

      case State::release:
        relTime = releaseTime;

      default:
        break;
    }

    if (state == State::attack)
      pController.setCutoff(sampleRate, Sample(1) / attackTime);
    else if (state == State::decay)
      pController.setCutoff(sampleRate, Sample(1) / decayTime);
    else if (state == State::release)
      pController.setCutoff(sampleRate, Sample(1) / releaseTime);
  }

  void release(Sample sampleRate)
  {
    state = State::release;
    pController.setCutoff(sampleRate, Sample(1) / relTime);
  }

  bool isAttacking() { return state == State::attack; }
  bool isReleasing() { return state == State::release; }
  bool isTerminated() { return state == State::terminated; }

  Sample process(Sample sampleRate)
  {
    switch (state) {
      case State::attack: {
        value = pController.process(Sample(1));
        --atk;
        if (atk == 0) {
          state = State::decay;
          pController.setCutoff(sampleRate, Sample(1) / decTime);
        }
      } break;

      case State::decay:
        value = pController.process(sustain);
        break;

      case State::release:
        value = pController.process(0);
        if (value < threshold) {
          value = threshold;
          state = State::tail;
          tailCounter = tailLength;
        }
        break;

      case State::tail:
        --tailCounter;
        value = threshold * tailCounter / float(tailLength);
        if (tailCounter == 0) {
          state = State::terminated;
          pController.reset(0);
        } else {
          pController.reset(value);
        }
        break;

      default:
        return 0;
    }
    return value;
  }

private:
  enum class State : int32_t { attack, decay, release, tail, terminated };
  const Sample threshold = Sample(1e-5);

  uint32_t tailLength = 32;
  uint32_t tailCounter = tailLength;

  EMAFilter<Sample> pController;
  State state = State::terminated;
  uint32_t atk = 0;
  Sample decTime = 0;
  Sample relTime = 0;
  Sample sustain = 1;
  Sample value = 0;
};

} // namespace SomeDSP
