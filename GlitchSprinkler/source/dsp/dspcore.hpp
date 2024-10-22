// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "polynomial.hpp"

#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore;
using PolySolver = PolynomialCoefficientSolver<double, nPolyOscControl>;

class Voice {
private:
  DSPCore &core;

public:
  Voice(DSPCore &core) : core(core) {}

  enum class State { active, release, terminate, rest };
  State state = State::rest;
  double lastGain = 0; // Used for note stealing to choose the most quite voice.

  int32_t noteId = -1;
  int32_t noteNumber = 0;
  double noteCent = 0;
  double noteVelocity = 0;

  unsigned unisonIndex = 1;
  double unisonRatio = double(0);
  double unisonPan = double(0.5);
  double unisonGain = double(1);

  std::minstd_rand rngArpeggio{0};

  int_fast32_t arpeggioTie = 1;
  int_fast32_t arpeggioTimer = 0;
  int_fast32_t arpeggioLoopCounter = 0;
  int_fast32_t terminationCounter = 0;

  bool scheduleUpdateNote = false;
  int_fast32_t pwmLower = 0;
  int_fast32_t pwmPoint = 0;
  uint_fast32_t pwmBitMask = 0;
  int_fast32_t pwmDirection = 1;
  int_fast32_t pwmChangeCounter = 1;
  int_fast32_t phasePeriod = 1;
  int_fast32_t phaseCounter = 0;
  double oscSync = double(1);
  double fmIndex = double(0);
  double saturationGain = double(1);
  double decayRatio = double(1);
  double decayGain = double(0);
  double decayEmaRatio = double(1);
  double decayEmaValue = double(0);
  std::array<double, PolySolver::nPolynomialPoint> polynomialCoefficients{};

  double filterDecayRatio = double(1);
  double filterDecayGain = double(0);
  double cutoffBase = double(1);
  double cutoffMod = double(0);
  double resonanceBase = double(0);
  double resonanceMod = double(0);
  double notchBase = double(1);
  double notchMod = double(0);
  ResonantEmaLowpass1A1<double> lowpass;

  void reset();
  void setParameters();
  std::array<double, 2> processFrame();
  void updateNote();
  void resetArpeggio(unsigned seed);
};

class DSPCore {
public:
  struct NoteInfo {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int32_t channel;
    int32_t noteNumber;
    float cent;
    float velocity;
  };

  GlobalParameter param;

  double sampleRate = 48000.0;

  bool isPlaying = false;
  double tempo = 120.0;
  double beatsElapsed = 0.0;
  double previousBeatsElapsed = 0.0;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

  double currentBeat = 0;
  double pitchModifier = double(1);
  bool isPolyphonic = true;
  Voice::State noteOffState = Voice::State::terminate;

  int_fast32_t arpeggioNoteDuration = std::numeric_limits<int_fast32_t>::max();
  int_fast32_t arpeggioLoopLength = std::numeric_limits<int_fast32_t>::max();
  int_fast32_t terminationLength = 0;

  bool arpeggioSwitch = false;
  bool filterSwitch = false;
  bool softEnvelopeSwitch = false;
  bool pwmSwitch = false;
  bool pwmBidrection = false;
  bool bitmaskSwitch = false;
  double lowpassInterpRate = double(1) / double(64);
  double softAttackEmaRatio = double(1);
  double pwmRatio = double(1);
  int_fast32_t pwmChangeCycle = 1;
  int_fast32_t pwmAmount = 1;
  DecibelScale<double> velocityMap{-60, 0, true};
  ExpSmoother<double> safetyFilterMix;
  ExpSmoother<double> outputGain;

  bool isPolynomialUpdated = false;
  PolySolver polynomial;

  // Maybe make it possible to change the pitch modifier channel.
  static constexpr size_t pitchModifierChannel = 15;

  std::vector<NoteInfo> modifierNotes;
  std::vector<NoteInfo> midiNotes;
  std::vector<NoteInfo> activeNote;
  std::vector<NoteInfo> activeModifier;
  std::vector<size_t> noteIndices;

  unsigned nextSteal = 0;
  std::vector<Voice> voices;

  std::array<SafetyFilter<double>, 2> safetyFilter;

  DSPCore()
  {
    modifierNotes.reserve(2048);
    midiNotes.reserve(2048);
    activeNote.reserve(2048);
    activeModifier.reserve(2048);

    noteIndices.reserve(2048);
    std::fill(noteIndices.begin(), noteIndices.end(), size_t(0));

    voices.reserve(256);
    for (size_t i = 0; i < 256; ++i) voices.emplace_back(*this);
  }

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(NoteInfo &info);
  void noteOff(int_fast32_t noteId);
  void modNoteOn(NoteInfo &info);
  void modNoteOff(int_fast32_t noteId);

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t channel,
    int16_t noteNumber,
    float tuning,
    float velocity)
  {
    NoteInfo note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.channel = channel;
    note.noteNumber = noteNumber;
    note.cent = tuning;
    note.velocity = velocity;

    if (note.channel == pitchModifierChannel) {
      modifierNotes.push_back(note);
    } else {
      midiNotes.push_back(note);
    }
  }

#define DEFINE_NOTE_PROC_FUNC(FUNC_NAME, VECTOR, ON_FUNC, OFF_FUNC)                      \
  void FUNC_NAME(size_t frame)                                                           \
  {                                                                                      \
    while (true) {                                                                       \
      auto it = std::find_if(VECTOR.begin(), VECTOR.end(), [&](const NoteInfo &nt) {     \
        return nt.frame == frame;                                                        \
      });                                                                                \
      if (it == std::end(VECTOR)) return;                                                \
      if (it->isNoteOn) {                                                                \
        ON_FUNC(*it);                                                                    \
      } else {                                                                           \
        OFF_FUNC(it->id);                                                                \
      }                                                                                  \
      VECTOR.erase(it);                                                                  \
    }                                                                                    \
  }

  DEFINE_NOTE_PROC_FUNC(processMidiNote, midiNotes, noteOn, noteOff);
  DEFINE_NOTE_PROC_FUNC(processModifierNote, modifierNotes, modNoteOn, modNoteOff);
};
