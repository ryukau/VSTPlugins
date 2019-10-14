// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../parameter.hpp"
#include "constants.hpp"
#include "envelope.hpp"
#include "iir.hpp"
#include "noise.hpp"
#include "oscillator.hpp"
#include "smoother.hpp"

#include <array>
#include <cmath>
#include <memory>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

template<typename Sample> struct NoteProcessInfo {
  Sample osc1Gain;
  Sample osc1Pitch;
  Sample osc1Sync;
  int32_t osc1SyncType;
  uint32_t osc1PTROrder;

  Sample osc2Gain;
  Sample osc2Pitch;
  Sample osc2Sync;
  int32_t osc2SyncType;
  uint32_t osc2PTROrder;

  Sample fmOsc1ToSync1;
  Sample fmOsc1ToFreq2;
  Sample fmOsc2ToSync1;

  Sample gainEnvelopeCurve;

  Sample filterCutoff;
  Sample filterResonance;
  Sample filterFeedback;
  Sample filterSaturation;
  Sample filterCutoffAmount;
  Sample filterResonanceAmount;
  Sample filterKeyToCutoff;
  Sample filterKeyToFeedback;

  Sample modEnvelopeToFreq1;
  Sample modEnvelopeToSync1;
  Sample modEnvelopeToFreq2;
  Sample modEnvelopeToSync2;
  Sample modLFO;
  Sample modLFOToFreq1;
  Sample modLFOToSync1;
  Sample modLFOToFreq2;
  Sample modLFOToSync2;
};

enum class NoteState { active, release, rest };

template<typename Sample> class Note {
public:
  NoteState state = NoteState::rest;

  int32_t id = -1;
  Sample normalizedKey = 0.0;
  Sample velocity = 0.0;
  Sample gain = 0.0;
  Sample frequency = 0.0;
  bool bypassFilter = false;

  PTRSyncSaw<Sample> saw1;
  PTRSyncSaw<Sample> saw2;
  std::array<float, 2> oscBuffer = {0.0, 0.0};

  SerialFilter<Sample, Biquad<Sample>, 4> filter;

  ExpADSREnvelope<float> gainEnvelope;
  LinearEnvelope<float> filterEnvelope;
  PolyExpEnvelope<double> modEnvelope;

  Note(Sample sampleRate)
    : saw1(sampleRate, 0.0, 0.0)
    , saw2(sampleRate, 0.0, 0.0)
    , filter(sampleRate, 20000.0, 0.5)
    , gainEnvelope(sampleRate, 0.2, 0.5, 0.2, 1.0)
    , filterEnvelope(sampleRate, 0.2, 0.5, 0.2, 1.0)
    , modEnvelope(sampleRate, 0.0, 1.0)
  {
  }

  void setup(int32_t noteId,
    Sample normalizedKey,
    Sample frequency,
    Sample velocity,
    GlobalParameter &param);
  void release();
  void rest();
  Sample process(NoteProcessInfo<Sample> &info);
};

class DSPCore {
public:
  GlobalParameter param;

  void setup(double sampleRate);
  void free();                      // Release memory.
  void reset();                     // Stop sounds.
  void startup();                   // Reset phase, random seed etc.
  void setParameters(double tempo); // tempo is beat per minutes.
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int32_t noteId, int16 pitch, float tuning, float velocity);
  void noteOff(int32_t noteId, int16 pitch);

protected:
  float sampleRate = 44100.0f;
  float lfoPhase = 0.0f;
  float lfoValue = 0.0f;

  Pink<float> noise{0};

  NoteProcessInfo<float> noteInfo;

  LinearSmoother<float> interpMasterGain;
  LinearSmoother<float> interpOsc1Gain;
  LinearSmoother<float> interpOsc1Pitch;
  LinearSmoother<float> interpOsc1Sync;
  LinearSmoother<float> interpOsc2Gain;
  LinearSmoother<float> interpOsc2Pitch;
  LinearSmoother<float> interpOsc2Sync;
  LinearSmoother<float> interpFMOsc1ToSync1;
  LinearSmoother<float> interpFMOsc1ToFreq2;
  LinearSmoother<float> interpFMOsc2ToSync1;
  LinearSmoother<float> interpModEnvelopeToFreq1;
  LinearSmoother<float> interpModEnvelopeToSync1;
  LinearSmoother<float> interpModEnvelopeToFreq2;
  LinearSmoother<float> interpModEnvelopeToSync2;
  LinearSmoother<float> interpModLFOFrequency;
  LinearSmoother<float> interpModLFONoiseMix;
  LinearSmoother<float> interpModLFOToFreq1;
  LinearSmoother<float> interpModLFOToSync1;
  LinearSmoother<float> interpModLFOToFreq2;
  LinearSmoother<float> interpModLFOToSync2;
  LinearSmoother<float> interpGainEnvelopeCurve;
  LinearSmoother<float> interpFilterCutoff;
  LinearSmoother<float> interpFilterResonance;
  LinearSmoother<float> interpFilterFeedback;
  LinearSmoother<float> interpFilterSaturation;
  LinearSmoother<float> interpFilterCutoffAmount;
  LinearSmoother<float> interpFilterResonanceAmount;
  LinearSmoother<float> interpFilterKeyToCutoff;
  LinearSmoother<float> interpFilterKeyToFeedback;

  std::array<std::array<std::unique_ptr<Note<float>>, 2>, 32> notes;

  // Transition happens when synth is playing all notes and user send a new note on.
  // transitionBuffer is used to store a release of a note to reduce pop noise.
  std::vector<float> transitionBuffer;
  bool isTransitioning = false;
  size_t mptIndex = 0; // mpt for Max Poly Transition.
  size_t mptStop = 0;
};

} // namespace Synth
} // namespace Steinberg
