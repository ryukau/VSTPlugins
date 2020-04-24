// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../../../common/dsp/somemath.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> inline void trimNoteFreq(Sample &noteFreq)
{
  if (somefabs(noteFreq) < Sample(0.001)) noteFreq = Sample(0.001);
}

template<typename Sample> inline Sample adaptTime(Sample seconds, Sample noteFreq)
{
  const Sample cycle = Sample(1) / noteFreq;
  return seconds >= cycle ? seconds : cycle > Sample(0.1) ? Sample(0.1) : cycle;
}

template<typename Sample> class ExpDecayCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = Sample(1);
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    alpha = somepow(threshold, Sample(1) / (seconds * sampleRate));
  }

  bool isTerminated() { return value <= threshold; }

  Sample process()
  {
    if (value <= threshold) return Sample(0);
    value *= alpha;
    return value - threshold;
  }

protected:
  constexpr static Sample threshold = 1e-5;
  Sample value = 0;
  Sample alpha = 0;
};

template<typename Sample> class ExpAttackCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = threshold;
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    alpha = somepow(Sample(1) / threshold, Sample(1) / (seconds * sampleRate));
  }

  bool isTerminated() { return value >= Sample(1); }

  Sample process()
  {
    value *= alpha;
    if (value >= Sample(1)) return Sample(1 - threshold);
    return value - threshold;
  }

protected:
  constexpr static Sample threshold = 1e-5;
  Sample value = 0;
  Sample alpha = 0;
};

template<typename Sample> class LinAttackCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = 0;
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    ramp = (Sample(1) - threshold) / (sampleRate * seconds);
  }

  bool isTerminated() { return value >= Sample(1); }

  Sample process()
  {
    value += ramp;
    if (value >= Sample(1 - threshold)) return Sample(1 - threshold);
    return value;
  }

protected:
  constexpr static Sample threshold = 1e-5;
  Sample value = 0;
  Sample ramp = 0;
};

template<typename Sample> class AttackGate {
public:
  void reset(Sample sampleRate, Sample attackTime, Sample noteFreq)
  {
    trimNoteFreq(noteFreq);
    atk.reset(sampleRate, adaptTime(attackTime, noteFreq));
  }

  Sample process() { return atk.process(); }

  LinAttackCurve<Sample> atk;
};

template<typename Sample> class ExpADSREnvelope {
public:
  void reset(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample curve,
    Sample noteFreq)
  {
    trimNoteFreq(noteFreq);

    state = State::attack;
    sus.reset(sustainLevel);

    this->curve = std::clamp<Sample>(curve, Sample(0), Sample(1));
    attackTime = adaptTime(attackTime, noteFreq);
    atk.reset(sampleRate, attackTime);
    atkLin.reset(sampleRate, attackTime);

    dec.reset(sampleRate, decayTime);

    sus.push(std::clamp<Sample>(sustainLevel, Sample(0), Sample(1)));

    rel.reset(sampleRate, adaptTime(releaseTime, noteFreq));
  }

  void set(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample curve,
    Sample noteFreq)
  {
    trimNoteFreq(noteFreq);

    switch (state) {
      default:
      case State::attack:
        this->curve = std::clamp<Sample>(curve, Sample(0), Sample(1));
        attackTime = adaptTime(attackTime, noteFreq);
        atk.set(sampleRate, attackTime);
        atkLin.set(sampleRate, attackTime);
        // Fall through.

      case State::decay:
        dec.set(sampleRate, decayTime);
        // Fall through.

      case State::sustain:
        sus.push(std::clamp<Sample>(sustainLevel, Sample(0), Sample(1)));
        // Fall through.

      case State::release:
        rel.set(sampleRate, adaptTime(releaseTime, noteFreq));
        break;
    }
  }

  void release()
  {
    range = value;
    state = State::release;
  }

  void terminate()
  {
    value = 0;
    state = State::terminated;
  }

  bool isAttacking() { return state == State::attack; }
  bool isReleasing() { return state == State::release; }
  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    const auto susV = sus.process();

    switch (state) {
      case State::attack: {
        auto atkPos = atk.process();
        value = atkPos + curve * (atkLin.process() - atkPos);
        if (atk.isTerminated()) state = State::decay;
      } break;

      case State::decay:
        value = (Sample(1) - susV) * dec.process() + susV;
        if (value <= susV) state = State::sustain;
        break;

      case State::sustain:
        value = susV;
        break;

      case State::release:
        value = range * rel.process();
        if (rel.isTerminated()) state = State::terminated;
        break;

      default:
        return 0;
    }
    return value;
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, terminated };

  ExpAttackCurve<Sample> atk{};
  LinAttackCurve<Sample> atkLin{};
  ExpDecayCurve<Sample> dec{};
  ExpDecayCurve<Sample> rel{};

  LinearSmoother<Sample> sus;

  State state = State::terminated;
  Sample value = 0;
  Sample curve = 0;
  Sample range = 1;
};

template<typename Sample> class LinearADSREnvelope {
public:
  Sample secondToDelta(Sample sampleRate, Sample seconds)
  {
    return Sample(1) / (sampleRate * seconds);
  }

  void reset(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample noteFreq)
  {
    state = State::attack;
    value = Sample(1);
    sus.reset(sustainLevel);
    set(sampleRate, attackTime, decayTime, sustainLevel, releaseTime, noteFreq);
  }

  void set(
    Sample sampleRate,
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample noteFreq)
  {
    sus.push(std::clamp<Sample>(sustainLevel, Sample(0), Sample(1)));
    trimNoteFreq(noteFreq);
    atk = secondToDelta(sampleRate, adaptTime(attackTime, noteFreq));
    dec = secondToDelta(sampleRate, adaptTime(decayTime, noteFreq));
    rel = secondToDelta(sampleRate, adaptTime(releaseTime, noteFreq));
  }

  void release()
  {
    state = State::release;
    value = Sample(1);
    relRange = out;
  }

  bool isAttacking() { return state == State::attack; }
  bool isReleasing() { return state == State::release; }
  bool isTerminated() { return state == State::terminated; }

  Sample process()
  {
    if (value <= Sample(0)) {
      state = static_cast<State>(static_cast<int>(state) + 1);
      value = Sample(1);
    }

    const auto susV = sus.process();

    switch (state) {
      case State::attack:
        value -= atk;
        out = Sample(1) - value;
        break;

      case State::decay:
        value -= dec;
        out = (Sample(1) - susV) * value + susV;
        break;

      case State::sustain:
        out = susV;
        break;

      case State::release:
        value -= rel;
        out = relRange * value;
        break;

      default:
        return Sample(0);
    }
    return std::clamp<Sample>(out, Sample(0), Sample(1));
  }

protected:
  enum class State : int { attack, decay, sustain, release, terminated };

  State state = State::terminated;
  LinearSmoother<Sample> sus;

  Sample atk = 0.01;
  Sample dec = 0.01;
  Sample rel = 0.01;
  Sample relRange = 0.5;
  Sample value = 0;
  Sample out = 0;
};

} // namespace SomeDSP
