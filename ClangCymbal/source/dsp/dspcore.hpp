// (c) 2022 Takamitsu Endo
//
// This file is part of ClangCymbal.
//
// ClangCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/pcg-cpp/pcg_random.hpp"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "fdn.hpp"
#include "lfo.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr float minOscNoteOffsetRate = float(9.5); // ~= 12 * log2(sqrt(3)).

inline float calcNotePitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2((note - 69.0f) / equalTemperament);
}

inline float noteToPitch(float note, float equalTemperament = 12.0f)
{
  return std::exp2(note / equalTemperament);
}

enum class NoteState { active, release, rest };

#define NOTE_PROCESS_INFO_SMOOTHER(METHOD)                                               \
  envelope.interpType = pv[ID::modEnvelopeInterpolation]->getInt();                      \
  for (size_t idx = 0; idx < nModEnvelopeWavetable; ++idx) {                             \
    envelope.source[idx + 1] = pv[ID::modEnvelopeWavetable0 + idx]->getFloat();          \
  }                                                                                      \
  envelope.source[nModEnvelopeWavetable] = 0;                                            \
                                                                                         \
  fdnEnable = pv[ID::fdnEnable]->getInt();                                               \
                                                                                         \
  eqTemp = pv[ID::equalTemperament]->getFloat() + float(1);                              \
  auto semitone = int_fast32_t(pv[ID::semitone]->getInt()) - 120;                        \
  auto octave = int_fast32_t(pv[ID::octave]->getInt()) - 12;                             \
  auto milli = float(0.001) * (int_fast32_t(pv[ID::milli]->getInt()) - 1000);            \
  auto a4Hz = pv[ID::pitchA4Hz]->getFloat() + float(100);                                \
  auto pitchBend = pv[ID::pitchBendRange]->getFloat() * pv[ID::pitchBend]->getFloat();   \
  fdnFreqOffset.METHOD(a4Hz *calcNotePitch(                                              \
    eqTemp *octave + semitone + milli + pitchBend + float(69), eqTemp));                 \
                                                                                         \
  oscBounce.METHOD(pv[ID::oscBounce]->getFloat());                                       \
  oscBounceCurve.METHOD(pv[ID::oscBounceCurve]->getFloat());                             \
  oscJitter.METHOD(float(1) - pv[ID::oscJitter]->getFloat());                            \
  oscPulseGainRandomness.METHOD(pv[ID::oscPulseGainRandomness]->getFloat());             \
  oscNoisePulseRatio.METHOD(pv[ID::oscNoisePulseRatio]->getFloat());                     \
                                                                                         \
  fdnOvertoneOffset.METHOD(pv[ID::fdnOvertoneOffset]->getFloat());                       \
  fdnOvertoneMul.METHOD(pv[ID::fdnOvertoneMul]->getFloat());                             \
  fdnOvertoneAdd.METHOD(pv[ID::fdnOvertoneAdd]->getFloat());                             \
  fdnOvertoneModulo.METHOD(pv[ID::fdnOvertoneModulo]->getFloat());                       \
  fdnFeedback.METHOD(pv[ID::fdnFeedback]->getFloat());                                   \
                                                                                         \
  modEnvelopeToFdnPitch.METHOD(pv[ID::modEnvelopeToFdnPitch]->getFloat());               \
  modEnvelopeToFdnOvertoneAdd.METHOD(pv[ID::modEnvelopeToFdnOvertoneAdd]->getFloat());

struct NoteProcessInfo {
  pcg64 fdnRng;
  uint32_t previousSeed = 0;
  std::vector<std::vector<float>> fdnMatrixRandomBase;

  TableLFO<float, nModEnvelopeWavetable + 1, 1024, TableLFOType::envelope> envelope;
  float lfoPhase = 0;

  bool fdnEnable = true;
  float smootherKp = float(1);
  float eqTemp = float(12);

  ExpSmoother<float> oscBounce;
  ExpSmoother<float> oscBounceCurve;
  ExpSmoother<float> oscJitter;
  ExpSmoother<float> oscPulseGainRandomness;
  ExpSmoother<float> oscNoisePulseRatio;
  ExpSmoother<float> fdnFreqOffset;
  ExpSmoother<float> fdnOvertoneOffset;
  ExpSmoother<float> fdnOvertoneMul;
  ExpSmoother<float> fdnOvertoneAdd;
  ExpSmoother<float> fdnOvertoneModulo;
  ExpSmoother<float> fdnFeedback;
  ExpSmoother<float> modEnvelopeToFdnPitch;
  ExpSmoother<float> modEnvelopeToFdnOvertoneAdd;

  NoteProcessInfo()
  {
    fdnMatrixRandomBase.resize(fdnMatrixSize);
    for (size_t i = 0; i < fdnMatrixRandomBase.size(); ++i) {
      fdnMatrixRandomBase[i].resize(fdnMatrixSize - i);
    }
  }

  void reset(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    previousSeed = pv[ID::fdnSeed]->getInt();
    fdnRng.seed(previousSeed);

    std::normal_distribution<float> dist{}; // mean 0, stddev 1.
    for (auto &row : fdnMatrixRandomBase) {
      for (auto &value : row) value = dist(fdnRng);
    }

    envelope.reset();
    lfoPhase = 0;

    NOTE_PROCESS_INFO_SMOOTHER(reset);
  }

  void setParameters(GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    auto seed = pv[ID::fdnSeed]->getInt();
    if (previousSeed != seed) {
      previousSeed = seed;

      std::normal_distribution<float> dist{};
      for (auto &row : fdnMatrixRandomBase) {
        for (auto &value : row) value = dist(fdnRng);
      }
    }

    NOTE_PROCESS_INFO_SMOOTHER(push);
  }

  void process()
  {
    envelope.processRefresh();

    oscBounce.process();
    oscBounceCurve.process();
    oscJitter.process();
    oscPulseGainRandomness.process();
    oscNoisePulseRatio.process();
    fdnFreqOffset.process();
    fdnOvertoneOffset.process();
    fdnOvertoneMul.process();
    fdnOvertoneAdd.process();
    fdnOvertoneModulo.process();
    fdnFeedback.process();
    modEnvelopeToFdnPitch.process();
    modEnvelopeToFdnOvertoneAdd.process();
  }
};

class Note {
public:
  NoteState state = NoteState::rest;

  int_fast32_t id = -1;
  float velocity = 0;
  float fdnPitch = 0;
  float oscNote = 0;
  float gain = 0;
  float releaseSwitch = float(1);

  ExpSmoother<float> oscDensity;
  ExpSmoother<float> oscLowpassCutoff;
  ExpSmoother<float> oscLowpassQ;
  ExpSmoother<float> tremoloMix;
  ExpSmoother<float> tremoloDepth;
  ExpSmoother<float> tremoloDelayTime;
  ExpSmoother<float> tremoloModToDelayTimeOffset;
  ExpSmoother<float> tremoloModSmoothingKp;

  float impulse = 0;
  SampleAndHoldNoise<float> pulsar;
  GenericSVF<float, 0> oscLowpass;
  DoubleEmaADEnvelope<float> envelope;
  DoubleEMAFilter<float> oscEnvelopeSmoother;
  EnvelopePhase<float> modEnvelopePhase;
  std::array<float, fdnMatrixSize> overtoneRandomness{};
  FeedbackDelayNetwork<float, fdnMatrixSize> fdn;
  Tremolo<float> tremolo;
  DoubleEMAFilter<float> tremoloSmoother;
  NoteGate<float> gate;
  DoubleEMAFilter<float> gateSmoother;

  void setup(float sampleRate);
  void reset(float sampleRate, NoteProcessInfo &info, GlobalParameter &param);
  void setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param);
  void noteOn(
    int_fast32_t noteId,
    float notePitch,
    float velocity,
    float pan,
    float sampleRate,
    NoteProcessInfo &info,
    GlobalParameter &param);
  void release(float sampleRate);
  void rest();
  bool isAttacking();
  float getGain();
  std::array<float, 2> process(float sampleRate, NoteProcessInfo &info);
};

class DSPCore final {
public:
  GlobalParameter param;
  bool isInitialized = false;
  bool isPlaying = false;

  DSPCore();

  void setup(double sampleRate); // Initialize when sampling rate is changed.
  void reset();                  // Stop sounds.
  void startup();                // Reset phase, random seed etc.
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity);
  void noteOff(int_fast32_t noteId);
  void fillTransitionBuffer(size_t noteIndex);

  struct MidiNote {
    bool isNoteOn;
    size_t frame;
    int_fast32_t id;
    int_fast16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    size_t frame,
    int_fast32_t noteId,
    int_fast16_t pitch,
    float tuning,
    float velocity)
  {
    MidiNote note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch;
    note.tuning = tuning;
    note.velocity = velocity;
    midiNotes.push_back(note);
  }

  void processMidiNote(size_t frame)
  {
    while (true) {
      auto it = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {
        return nt.frame == frame;
      });
      if (it == std::end(midiNotes)) return;
      if (it->isNoteOn)
        noteOn(it->id, it->pitch, it->tuning, it->velocity);
      else
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }

private:
  static constexpr size_t upFold = 2;
  bool prepareRefresh = true;
  bool isWavetableRefeshed = false;
  float sampleRate = 44100.0f;
  float upRate = 88200.0f;
  DecibelScale<float> velocityMap{-60, 0, true};

  Note note;
  NoteProcessInfo info;
  ExpSmoother<float> interpMasterGain;

  std::array<std::array<float, 2>, 2> halfIn{{}};
  std::array<HalfBandIIR<float, HalfBandCoefficient<float>>, 2> halfbandIir;

  std::vector<std::array<float, 2>> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};
