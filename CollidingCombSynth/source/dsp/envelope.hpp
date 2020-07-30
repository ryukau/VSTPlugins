// (c) 2020 Takamitsu Endo
//
// This file is part of CollidingCombSynth.
//
// CollidingCombSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CollidingCombSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CollidingCombSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

namespace SomeDSP {

template<typename Sample> class ExpADSREnvelopeP {
public:
  void setup(Sample sampleRate) { tailLength = uint32_t(0.01 * sampleRate); }

  void reset(
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
  const Sample threshold = 1e-5;

  uint32_t tailLength = 32;
  uint32_t tailCounter = tailLength;

  PController<Sample> pController;
  State state = State::terminated;
  uint32_t atk = 0;
  Sample decTime = 0;
  Sample relTime = 0;
  Sample sustain = 1;
  Sample value = 0;
};

} // namespace SomeDSP
