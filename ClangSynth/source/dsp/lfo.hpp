// (c) 2022 Takamitsu Endo
//
// This file is part of ClangSynth.
//
// ClangSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <limits>

namespace SomeDSP {

// PCHIP interpolation. Monotonic between control points.
template<typename T> T pchipInterp(T y0, T y1, T y2, T y3, T t)
{
  T m0 = y1 - y0;
  T m1 = y2 - y1;
  T m2 = y3 - y2;

  T dk0 = m0 * m1 <= 0 ? 0 : T(2) * (m0 * m1) / (m0 + m1);
  T dk1 = m1 * m2 <= 0 ? 0 : T(2) * (m1 * m2) / (m1 + m2);

  T t2 = t * t;
  T c0 = y1 - y2;
  T c1 = dk0;
  T c2 = c0 + c1;
  T c3 = c0 + c2 + dk1;
  return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y1;
}

enum class TableLFOType { lfo, envelope };

template<typename Sample, size_t sourceSize, size_t tableSize, TableLFOType tableType>
class TableLFO {
private:
  enum Interpolation : unsigned { step, linear, pchip };

  size_t refreshCounter = 0;
  size_t refreshInterval = 512; // In samples.
  size_t tableIndex = 0;
  Sample fade = 0;

public:
  std::array<Sample, sourceSize + 3> source{};              // +3 size for cubic interp.
  std::array<std::array<Sample, tableSize + 1>, 2> table{}; // +1 size for lerp.
  unsigned interpType = Interpolation::step;

  void setup(Sample sampleRate, Sample refreshIntervalSeconds)
  {
    refreshInterval = size_t(sampleRate * refreshIntervalSeconds);
  }

  void reset()
  {
    refreshCounter = 0;
    tableIndex = 0;
    fade = 0;

    refreshTable(0);
    refreshTable(1);
  }

  void refreshTable(size_t foreIndex)
  {
    if constexpr (tableType == TableLFOType::envelope) {
      source[0] = source[1];
      source[sourceSize + 1] = 0;
      source[sourceSize + 2] = 0;
    } else { // tableType == TableLFOType::lfo
      source[0] = source[sourceSize];
      source[sourceSize + 1] = source[1];
      source[sourceSize + 2] = source[2];
    }

    auto &tbl = table[foreIndex];

    if (interpType == Interpolation::linear) {
      for (size_t idx = 0; idx < tableSize; ++idx) {
        Sample target = Sample(1) + Sample(idx * sourceSize) / Sample(tableSize);
        size_t i0 = size_t(target);
        Sample frac = target - Sample(i0);
        tbl[idx] = source[i0] + frac * (source[i0 + 1] - source[i0]);
      }
    } else if (interpType == Interpolation::pchip) {
      for (size_t idx = 0; idx < tableSize; ++idx) {
        Sample target = Sample(idx * sourceSize) / Sample(tableSize);
        size_t i0 = size_t(target);
        auto &&value = pchipInterp(
          source[i0], source[i0 + 1], source[i0 + 2], source[i0 + 3],
          target - Sample(i0));
        tbl[idx] = std::clamp(value, Sample(-1), Sample(1));
      }
    } else { // Interpolation::step or default.
      for (size_t idx = 0; idx < tableSize; ++idx) {
        Sample target = Sample(idx * sourceSize) / Sample(tableSize);
        tbl[idx] = source[1 + size_t(target)];
      }
    }

    if constexpr (tableType == TableLFOType::envelope) {
      tbl.back() = 0;
    } else { // tableType == TableLFOType::lfo
      tbl.back() = tbl[0];
    }
  }

  void processRefresh()
  {
    if (++refreshCounter > refreshInterval) {
      refreshCounter = 0;
      tableIndex ^= 1;
      refreshTable(tableIndex);
    }
    fade = Sample(refreshCounter) / Sample(refreshInterval);
  }

  Sample process(const Sample phase) const
  {
    if constexpr (tableType == TableLFOType::envelope) {
      if (phase >= Sample(1)) return 0;
    }

    const auto &fore = table[tableIndex ^ 1];
    const auto &back = table[tableIndex];
    auto target = Sample(tableSize) * phase;
    size_t i0 = size_t(target);
    Sample fraction = target - Sample(i0);

    auto foreOut = fore[i0] + fraction * (fore[i0 + 1] - fore[i0]);
    auto backOut = back[i0] + fraction * (back[i0 + 1] - back[i0]);
    return foreOut + fade * (backOut - foreOut);
  }
};

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
    v1 = std::fabs(sync) <= std::numeric_limits<Sample>::min()
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

template<typename Sample> struct EnvelopePhase {
  uint_fast32_t timeInSamples = 1;
  uint_fast32_t counter = 0;

  void noteOn(Sample sampleRate, Sample timeInSecond)
  {
    timeInSamples = std::max(uint_fast32_t(0), uint_fast32_t(sampleRate * timeInSecond));
    counter = 0;
  }

  Sample process()
  {
    if (counter >= timeInSamples) return Sample(1);
    ++counter;
    return Sample(counter) / Sample(timeInSamples);
  }
};

} // namespace SomeDSP
