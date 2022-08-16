// (c) 2022 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <limits>

namespace SomeDSP {

template<typename Sample> class LinearTempoSynchronizer {
private:
  enum class State { free, steady, catchingUp };

  State state = State::free;
  Sample v1 = 0;
  Sample p1 = 0;
  Sample lastTempo = 0;
  Sample lastSync = 0;
  double lastElapsedBeats = 0;

  static constexpr Sample transitionTime = Sample(1024); // In samples.
  Sample v2 = 0;
  Sample counter = 0;

  inline void setLastValues(Sample tempo, Sample sync, double elapsedBeats)
  {
    lastTempo = tempo;
    lastSync = sync;
    lastElapsedBeats = elapsedBeats;
  }

  template<typename T> inline T wrap(T value) { return value - std::floor(value); }

public:
  Sample getPhase() { return p1; }

  // sync is duration in beat.
  void reset(Sample sampleRate, Sample tempo, Sample sync)
  {
    v1 = std::fabs(sync) <= std::numeric_limits<Sample>::epsilon()
      ? 0
      : tempo / (Sample(60) * sampleRate * sync);
    p1 = 0;
    lastTempo = tempo;
    lastSync = sync;
    counter = 0;
  }

  // Must call this method at the start of each DSP processing cycle.
  void
  prepare(Sample sampleRate, Sample tempo, Sample sync, double elapsedBeats, bool isFree)
  {
    if (std::fabs(sync) <= std::numeric_limits<Sample>::epsilon()) {
      v1 = 0;
      setLastValues(tempo, sync, elapsedBeats);
      return;
    }

    v1 = tempo / (Sample(60) * sampleRate * sync);

    if (isFree) state = State::free;
    if (state == State::free) {
      if (!isFree) state = State::steady;
      setLastValues(tempo, sync, elapsedBeats);
      return;
    }

    auto p0 = wrap(Sample(elapsedBeats / sync));
    if (lastTempo != tempo || lastSync != sync || lastElapsedBeats > elapsedBeats) {
      state = State::catchingUp;
      v2 = wrap(p0 + v1 * transitionTime - p1) / transitionTime;
      counter = 0;
    } else if (state != State::catchingUp) {
      p1 = p0;
    }

    setLastValues(tempo, sync, elapsedBeats);
  }

  Sample process()
  {
    auto outPhase = p1;
    if (state == State::catchingUp) {
      if (++counter >= transitionTime) {
        state = State::steady;
        counter = 0;
      }
      p1 = wrap(p1 + v2);
      return outPhase;
    }

    p1 = wrap(p1 + v1);
    return outPhase;
  }
};

template<typename Sample> struct LFOPhase {
  Sample offset = 0;

  Sample process(Sample phase)
  {
    auto q = phase + offset;
    return q - std::floor(q);
  }
};

} // namespace SomeDSP
