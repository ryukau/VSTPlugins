#pragma once

#include "constants.hpp"
#include "smoother.hpp"
#include "somemath.hpp"

#include <algorithm>

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return 0.5 * (1.0 - somecos<Sample>(pi * t));
}

enum class EnvelopeCurveType { attack, decay };

template<typename Sample, EnvelopeCurveType type> class PolyCurve {
public:
  PolyCurve(Sample sampleRate, Sample seconds, Sample power)
  {
    reset(sampleRate, seconds, power);
  }

  void reset(Sample sampleRate, Sample seconds, Sample power)
  {
    const auto sampleLength = 1.0 / sampleRate;
    if (seconds < sampleLength) seconds = sampleLength;
    length = seconds * sampleRate;

    this->power = power;

    if constexpr (type == EnvelopeCurveType::attack)
      counter = 0;
    else
      counter = length;
  }

  bool isTerminated()
  {
    if constexpr (type == EnvelopeCurveType::attack)
      return counter >= length;
    else
      return counter <= 0;
  }

  Sample process()
  {
    if constexpr (type == EnvelopeCurveType::attack) {
      if (counter >= length) return 1.0;
      return somepow<Sample>((Sample)(++counter) / length, power);
    } else {
      if (counter <= 0) return 0.0;
      return somepow<Sample>((Sample)(--counter) / length, power);
    }
  }

protected:
  Sample power;
  int32_t counter = 0;
  int32_t length;
};

template<typename Sample, EnvelopeCurveType type, size_t tableSize> class TableCurve {
public:
  // curve in [0.0, 1.0].
  TableCurve(Sample sampleRate, Sample seconds, Sample curve)
  {
    for (size_t i = 0; i < tableSize; ++i)
      tableF[i] = somepow<Sample>(i / Sample(tableSize - 1), Sample(4.5));

    for (size_t i = 0; i < tableSize; ++i) tableR[i] = 1 - tableF[tableSize - 1 - i];

    reset(sampleRate, seconds, curve);
  }

  void reset(Sample sampleRate, Sample seconds, Sample curve)
  {
    set(sampleRate, seconds, curve);

    if constexpr (type == EnvelopeCurveType::attack)
      phase = 0;
    else
      phase = tableSize - 1;
  }

  void set(Sample sampleRate, Sample seconds, Sample curve)
  {
    const auto nSample = seconds * sampleRate;
    tick = nSample < 1 ? 1 : Sample(1) / nSample;
    tick *= tableSize - 1;

    this->curve = curve;
  }

  bool isTerminated()
  {
    if constexpr (type == EnvelopeCurveType::attack)
      return phase >= tableSize - 1;
    else
      return phase <= 0;
  }

  Sample at(Sample pos)
  {
    size_t low = phase;
    size_t high = someceil<Sample>(phase);
    auto outF = tableF[low] + (pos - low) * (tableF[high] - tableF[low]);
    auto outR = tableR[low] + (pos - low) * (tableR[high] - tableR[low]);
    return outF + curve * (outR - outF);
  }

  Sample process()
  {
    if constexpr (type == EnvelopeCurveType::attack) {
      if (phase >= tableSize - 1) return 1;
      auto output = at(phase);
      phase += tick;
      return output;
    } else {
      if (phase <= 0) return 0;
      auto output = at(phase);
      phase -= tick;
      return output;
    }
  }

protected:
  Sample curve;
  Sample tick;
  Sample phase; // [0, tableSize].
  std::array<Sample, tableSize> tableF{};
  std::array<Sample, tableSize> tableR{};
};

template<typename Sample, typename Attack, typename Decay, typename Release>
class ADSREnvelope {
public:
  ADSREnvelope() {}

  void setup(Sample sampleRate)
  {
    this->sampleRate = sampleRate;
    smoother.sampleRate = sampleRate;
    declickLength = int32_t(1e-3 * sampleRate);
  }

  // curve in [0.0, 1.0].
  void reset(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample curve)
  {
    if (state == State::terminated) declickCounter = 0;
    state = State::attack;

    sustain = std::max<Sample>(Sample(0.0), std::min<Sample>(sustainLevel, Sample(1.0)));

    offset = value;
    range = Sample(1.0) - value;

    atk.reset(sampleRate, attackTime, curve);
    dec.reset(sampleRate, decayTime, curve);
    rel.reset(sampleRate, releaseTime, curve);
  }

  void set(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample curve)
  {
    switch (state) {
      case State::attack:
        atk.set(sampleRate, attackTime, curve);
        // Fall through.

      case State::decay:
        dec.set(sampleRate, decayTime, curve);
        // Fall through.

      case State::sustain:
        sustain
          = std::max<Sample>(Sample(0.0), std::min<Sample>(sustainLevel, Sample(1.0)));
        // Fall through.

      case State::release:
        rel.set(sampleRate, releaseTime, curve);
        // Fall through.

      default:
        break;
    }
  }

  void release()
  {
    range = value;
    state = State::release;
  }

  void terminate() { state = State::terminated; }
  bool isReleasing() { return state == State::release; }
  bool isTerminated() { return state == State::terminated; }
  Sample getValue() { return output; }

  Sample process()
  {
    switch (state) {
      case State::attack:
        value = range * atk.process() + offset;
        if (value >= Sample(1)) {
          state = State::decay;
          range = Sample(1.0) - sustain;
        }
        break;

      case State::decay:
        value = dec.process() * range + sustain;
        if (value <= sustain) state = State::sustain;
        break;

      case State::sustain:
        value = sustain;
        break;

      case State::release:
        value = rel.process() * range;
        if (rel.isTerminated()) state = State::terminated;
        break;

      default:
        return 0;
    }

    if (declickCounter < declickLength) {
      declickCounter += 1;
      output = smoother.process(
        value * cosinterp<Sample>(declickCounter / (Sample)declickLength));
    } else {
      output = smoother.process(value);
    }
    return output;
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, terminated };

  int32_t declickLength = 0;
  int32_t declickCounter = 0;

  Attack atk{44100, 1, 1};
  Decay dec{44100, 1, 1};
  Release rel{44100, 1, 1};

  PController<Sample> smoother{2048};

  State state = State::terminated;
  Sample value = 0;
  Sample output = 0;
  Sample sampleRate = 44100;
  Sample offset = 0;
  Sample range = 1;
  Sample sustain = 1;
};

// Almost always use double. float may be used if attack and curve is small.
// env(t) := t^alpha * exp(-beta * t)
// TODO: Find optimal range of alpha and beta.
template<typename Sample> class PolyExpEnvelope {
public:
  // attack is in seconds. curve is arbitrary value.
  void setup(Sample sampleRate) { this->sampleRate = sampleRate; }

  bool isReleasing() { return time >= attack; }

  void reset(Sample attack, Sample curve)
  {
    alpha = attack * curve;
    peak = somepow<Sample>(alpha / curve, alpha) * someexp<Sample>(-alpha);
    gamma = someexp<Sample>(-curve / sampleRate);
    tick = 1.0 / sampleRate;

    time = 0.0;
    value = 1.0;
  }

  Sample process()
  {
    auto output = somepow<Sample>(time, alpha) * value / peak;
    if (!std::isfinite(output)) return 0.0; // TODO
    time += tick;
    value *= gamma;
    return output;
  }

protected:
  Sample sampleRate = 44100;
  Sample value = 0;
  Sample peak = 1;
  Sample gamma = 0;
  Sample attack = 0;
  Sample tick = 0;
  Sample alpha = 0;
  Sample time = 0;
};

} // namespace SomeDSP
