// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "delay.hpp"
#include "envelope.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

inline float calcMasterPitch(int32_t octave, int32_t semi, int32_t milli, float bend)
{
  return 12 * octave + semi + milli / 1000.0f + (bend - 0.5f) * 4.0f;
}

inline float calcDelayPitch(int32_t semi, int32_t milli, float equalTemperament)
{
  return powf(2.0f, -(semi + 0.001f * milli) / equalTemperament);
}

inline float
notePitchToFrequency(float notePitch, float equalTemperament, float a4Hz = 440.0f)
{
  return a4Hz * powf(2.0f, (notePitch - 69.0f) / equalTemperament);
}

enum class NoteState { active, release, rest };

struct NoteProcessInfo {
  std::minstd_rand rng{0};

  LinearSmoother<float> masterPitch;
  LinearSmoother<float> equalTemperament;
  LinearSmoother<float> pitchA4Hz;
  LinearSmoother<float> filterCutoff;
  LinearSmoother<float> filterResonance;
  LinearSmoother<float> filterAmount;
  LinearSmoother<float> filterKeyFollow;
  LinearSmoother<float> delayMix;
  LinearSmoother<float> delayDetune;
  LinearSmoother<float> delayFeedback;
  LinearSmoother<float> lfoFrequency;
  LinearSmoother<float> lfoAmount;
  LinearSmoother<float> lfoLowpass;

  LfoTableOsc<lfoTableSize> lfo;
  EMAFilter<float> lowpass;
  float lfoOut = 0;

  void process(float sampleRate, LfoWavetable<lfoTableSize> &lfoWavetable)
  {
    masterPitch.process();
    equalTemperament.process();
    pitchA4Hz.process();
    filterCutoff.process();
    filterResonance.process();
    filterAmount.process();
    filterKeyFollow.process();
    delayMix.process();
    delayDetune.process();
    delayFeedback.process();
    lfoFrequency.process();
    lfoAmount.process();
    lfoLowpass.process();

    lowpass.setP(lfoLowpass.getValue());
    lfoOut = 1.0f
      + lfoAmount.getValue()
        * lowpass.process(
          lfo.process(lfoWavetable.table, sampleRate, lfoFrequency.getValue()));
    if (lfoOut < 0.0f) lfoOut = 0.0f;
  }

  void reset(GlobalParameter &param, float sampleRate)
  {
    using ID = ParameterID::ID;

    masterPitch.reset(calcMasterPitch(
      int32_t(param.value[ID::oscOctave]->getInt()) - 12,
      param.value[ID::oscSemi]->getInt() - 120,
      param.value[ID::oscMilli]->getInt() - 1000,
      param.value[ID::pitchBend]->getFloat()));

    auto et = param.value[ID::equalTemperament]->getFloat() + 1;
    equalTemperament.reset(et);
    pitchA4Hz.reset(param.value[ID::pitchA4Hz]->getFloat() + 100);

    filterCutoff.reset(param.value[ID::filterCutoff]->getFloat());
    filterResonance.reset(param.value[ID::filterResonance]->getFloat());
    filterAmount.reset(param.value[ID::filterAmount]->getFloat());
    filterKeyFollow.reset(param.value[ID::filterKeyFollow]->getFloat());

    delayMix.reset(param.value[ID::delayMix]->getFloat());
    delayDetune.reset(calcDelayPitch(
      param.value[ID::delayDetuneSemi]->getInt() - 120,
      param.value[ID::delayDetuneMilli]->getInt() - 1000, et));
    delayFeedback.reset(param.value[ID::delayFeedback]->getFloat());

    const float beat = float(param.value[ID::lfoTempoNumerator]->getInt() + 1)
      / float(param.value[ID::lfoTempoDenominator]->getInt() + 1);
    lfoFrequency.reset(1.0f);
    lfoAmount.reset(param.value[ID::lfoDelayAmount]->getFloat());
    lfoLowpass.reset(
      EMAFilter<float>::cutoffToP(sampleRate, param.value[ID::lfoLowpass]->getFloat()));

    lfo.reset();
    lowpass.reset();
    lfoOut = 0;
  }
};

class Note {
public:
  NoteState state = NoteState::rest;

  int32_t id = -1;
  float velocity = 0;
  float noteFreq = 1;
  float pan = 0.5f;
  float gain = 0;

  ExpADSREnvelope<float> gainEnvelope;
  LinearADSREnvelope<float> filterEnvelope;
  AttackGate<float> delayGate;
  TableOsc osc;
  LP3<float> filter;
  Delay<float> delay;
  float delaySeconds = 0;

  void setup(float sampleRate);
  void noteOn(
    int32_t noteId,
    float notePitch,
    float velocity,
    float pan,
    float phase,
    float sampleRate,
    Wavetable &wavetable,
    NoteProcessInfo &info,
    GlobalParameter &param);
  void release();
  void release(float seconds);
  void rest();
  void reset();
  bool isAttacking();
  float getGain();
  std::array<float, 2>
  process(float sampleRate, Wavetable &wavetable, NoteProcessInfo &info);
};

class DSPCore {
public:
  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  static constexpr size_t maxVoice = 128;
  GlobalParameter param;

  std::vector<MidiNote> midiNotes;

  DSPCore();

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters(float tempo);
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity);
  void fillTransitionBuffer(size_t noteIndex);
  void noteOff(int32_t noteId);
  void refreshTable();
  void refreshLfo();

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
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

  void processMidiNote(uint32_t frame)
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
  void setUnisonPan(size_t nUnison);

  float sampleRate = 44100.0f;

  std::vector<PeakInfo<float>> peakInfos;

  bool prepareRefresh = true;
  bool isTableRefeshed = false;
  bool isLFORefreshed = false;
  Wavetable wavetable;
  LfoWavetable<lfoTableSize> lfoWavetable;

  size_t nVoice = 32;
  int32_t panCounter = 0;
  std::vector<size_t> noteIndices;
  std::vector<size_t> voiceIndices;
  std::vector<float> unisonPan;
  std::array<Note, maxVoice> notes;

  NoteProcessInfo info;
  LinearSmoother<float> interpMasterGain;

  std::vector<std::array<float, 2>> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};
