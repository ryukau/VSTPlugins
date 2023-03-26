// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

constexpr size_t channel = 2;

template<typename T> inline std::array<T, 2> calcPan(T inL, T inR, T pan, T spread)
{
  T balanceL = std::clamp<T>(spread, T(0), T(1));
  T balanceR = std::clamp<T>(T(1) - spread, T(0), T(1));
  T sigL = inL + balanceL * (inR - inL);
  T sigR = inL + balanceR * (inR - inL);

  pan = std::clamp<T>(pan, T(0), T(1));
  if (pan < T(0.5)) {
    return {
      (T(0.5) + pan) * sigL + (T(0.5) - pan) * sigR,
      sigR * T(2) * pan,
    };
  }
  return {
    sigL * (T(2) - T(2) * pan),
    (pan - T(0.5)) * sigL + (T(1.5) - pan) * sigR,
  };
}

template<typename T> inline T calcNotePitch(T note)
{
  auto pitch = std::exp2((note - T(69)) / T(12));
  return T(1) / std::max(pitch, std::numeric_limits<float>::epsilon());
}

void DSPCore::setup(double sampleRate)
{
  SmootherCommon<double>::setSampleRate(double(sampleRate));

  for (size_t i = 0; i < delay.size(); ++i)
    delay[i].setup(double(sampleRate), double(1), maxDelayTime);

  for (size_t i = 0; i < filter.size(); ++i) filter[i].setup(double(sampleRate));

  for (size_t i = 0; i < dcKiller.size(); ++i) {
    dcKiller[i].setup(double(sampleRate), double(0.1));
  }

  lfoPhaseTick = double(twopi) / sampleRate;

  startup();
}

void DSPCore::reset()
{
  midiNotes.clear();
  noteStack.clear();
  notePitchMultiplier = double(1);

  for (size_t i = 0; i < channel; ++i) {
    delay[i].reset();
    filter[i].reset();
    dcKiller[i].reset();
  }
  startup();
}

void DSPCore::startup()
{
  delayOut.fill({});
  lfoPhase = param.value[ParameterID::lfoInitialPhase]->getDouble();
}

void DSPCore::setParameters()
{
  SmootherCommon<double>::setTime(param.value[ParameterID::smoothness]->getDouble());

  // This won't work if sync is on and tempo < 15. Up to 8 sec or 8/16 beat.
  // 15.0 comes from (60 sec per minute) * (4 beat) / (16 beat).
  auto time = param.value[ParameterID::time]->getDouble() * notePitchMultiplier;
  if (param.value[ParameterID::tempoSync]->getInt()) {
    if (time < double(1))
      time *= double(15) / double(tempo);
    else
      time = std::floor(double(2) * time) * double(7.5) / double(tempo);
  }

  auto offset = param.value[ParameterID::offset]->getDouble();
  interpTime[0].push(offset < double(0) ? time * (double(1) + offset) : time);
  interpTime[1].push(offset > double(0) ? time * (double(1) - offset) : time);

  interpWetMix.push(param.value[ParameterID::wetMix]->getDouble());
  interpDryMix.push(param.value[ParameterID::dryMix]->getDouble());
  interpFeedback.push(
    param.value[ParameterID::negativeFeedback]->getInt()
      ? -param.value[ParameterID::feedback]->getDouble()
      : param.value[ParameterID::feedback]->getDouble());
  interpLfoTimeAmount.push(param.value[ParameterID::lfoTimeAmount]->getDouble());
  interpLfoToneAmount.push(param.value[ParameterID::lfoToneAmount]->getDouble());
  interpLfoFrequency.push(param.value[ParameterID::lfoFrequency]->getDouble());
  interpLfoShape.push(param.value[ParameterID::lfoShape]->getDouble());

  interpPanIn.push(param.value[ParameterID::inPan]->getDouble());
  interpSpreadIn.push(param.value[ParameterID::inSpread]->getDouble());
  interpPanOut.push(param.value[ParameterID::outPan]->getDouble());
  interpSpreadOut.push(param.value[ParameterID::outSpread]->getDouble());

  interpToneCutoff.push(param.value[ParameterID::toneCutoff]->getDouble());
  interpToneQ.push(param.value[ParameterID::toneQ]->getDouble());
  interpToneMix.push(
    double(Scales::toneMix.map(param.value[ParameterID::toneCutoff]->getNormalized())));

  interpDCKill.push(param.value[ParameterID::dckill]->getDouble());
  interpDCKillMix.push(double(
    Scales::dckillMix.reverseMap(param.value[ParameterID::dckill]->getNormalized())));
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  SmootherCommon<double>::setBufferSize(double(length));

  const bool lfoHold = !param.value[ParameterID::lfoHold]->getInt();
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    auto sign = (pi < lfoPhase) - (lfoPhase < pi);
    const auto lfo
      = sign * std::pow(std::abs(std::sin(lfoPhase)), interpLfoShape.process());
    const auto lfoTime = interpLfoTimeAmount.process() * (double(1) + lfo);

    delay[0].setTime(interpTime[0].process() + lfoTime);
    delay[1].setTime(interpTime[1].process() + lfoTime);

    const auto feedback = interpFeedback.process();
    const auto inDelay = calcPan(
      double(in0[i]), double(in1[i]), interpPanIn.process(), interpSpreadIn.process());
    delayOut[0] = delay[0].process(inDelay[0] + feedback * delayOut[0]);
    delayOut[1] = delay[1].process(inDelay[1] + feedback * delayOut[1]);

    const auto lfoTone
      = interpLfoToneAmount.process() * (double(0.5) * lfo + double(0.5));
    auto toneCutoff = interpToneCutoff.process() * lfoTone * lfoTone;
    if (toneCutoff < double(20)) toneCutoff = double(20);
    const auto toneQ = interpToneQ.process();
    filter[0].setCutoffQ(toneCutoff, toneQ);
    filter[1].setCutoffQ(toneCutoff, toneQ);
    auto filterOutL = filter[0].process(delayOut[0]);
    auto filterOutR = filter[1].process(delayOut[1]);
    const auto toneMix = interpToneMix.process();
    delayOut[0] = filterOutL + toneMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + toneMix * (delayOut[1] - filterOutR);

    const auto dckill = interpDCKill.process();
    dcKiller[0].setCutoff(dckill);
    dcKiller[1].setCutoff(dckill);
    filterOutL = dcKiller[0].process(delayOut[0]);
    filterOutR = dcKiller[1].process(delayOut[1]);
    const auto dckillMix = interpDCKillMix.process();
    // dckillmix == 1 -> delayout
    delayOut[0] = filterOutL + dckillMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + dckillMix * (delayOut[1] - filterOutR);
    delayOut = calcPan(
      delayOut[0], delayOut[1], interpPanOut.process(), interpSpreadOut.process());

    const auto wet = interpWetMix.process();
    const auto dry = interpDryMix.process();
    out0[i] = float(dry * in0[i] + wet * delayOut[0]);
    out1[i] = float(dry * in1[i] + wet * delayOut[1]);

    if (lfoHold) {
      lfoPhase += interpLfoFrequency.process() * lfoPhaseTick;
      if (lfoPhase > double(2) * pi) lfoPhase -= double(2) * pi;
    }
  }
}

void DSPCore::noteOn(NoteInfo &info)
{
  notePitchMultiplier = calcNotePitch(info.pitch);
  updateDelayTime();

  noteStack.push_back(info);
}

void DSPCore::noteOff(int_fast32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.empty()) {
    notePitchMultiplier = double(1);
  } else {
    notePitchMultiplier = calcNotePitch(noteStack.back().pitch);
  }
  updateDelayTime();
}

void DSPCore::updateDelayTime()
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  auto time = param.value[ParameterID::time]->getFloat() * notePitchMultiplier;
  if (param.value[ParameterID::tempoSync]->getInt()) {
    if (time < double(1))
      time *= double(15) / double(tempo);
    else
      time = std::floor(double(2) * time) * double(7.5) / double(tempo);
  }

  auto offset = param.value[ParameterID::offset]->getFloat();
  interpTime[0].push(offset < double(0) ? time * (double(1) + offset) : time);
  interpTime[1].push(offset > double(0) ? time * (double(1) - offset) : time);
}
