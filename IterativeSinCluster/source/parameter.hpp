// (c) 2019-2020 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <vector>

#include "../../common/value.hpp"

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  aliasing,
  negativeSemi,
  masterOctave,
  equalTemperament,
  pitchMultiply,
  pitchModulo,

  seed,
  randomRetrigger,
  randomGainAmount,
  randomFrequencyAmount,

  chorusMix,
  chorusFrequency,
  chorusDepth,
  chorusDelayTimeRange0,
  chorusDelayTimeRange1,
  chorusDelayTimeRange2,
  chorusMinDelayTime0,
  chorusMinDelayTime1,
  chorusMinDelayTime2,
  chorusPhase,
  chorusOffset,
  chorusFeedback,
  chorusKeyFollow,

  gain,
  gainBoost,
  gainA,
  gainD,
  gainS,
  gainR,
  gainEnvelopeCurve,

  lowShelfPitch,
  lowShelfGain,
  highShelfPitch,
  highShelfGain,

  gain0,
  gain1,
  gain2,
  gain3,
  gain4,
  gain5,
  gain6,
  gain7,

  semi0,
  semi1,
  semi2,
  semi3,
  semi4,
  semi5,
  semi6,
  semi7,

  milli0,
  milli1,
  milli2,
  milli3,
  milli4,
  milli5,
  milli6,
  milli7,

  overtone1,
  overtone2,
  overtone3,
  overtone4,
  overtone5,
  overtone6,
  overtone7,
  overtone8,
  overtone9,
  overtone10,
  overtone11,
  overtone12,
  overtone13,
  overtone14,
  overtone15,
  overtone16,

  chordGain0,
  chordGain1,
  chordGain2,
  chordGain3,

  chordSemi0,
  chordSemi1,
  chordSemi2,
  chordSemi3,

  chordMilli0,
  chordMilli1,
  chordMilli2,
  chordMilli3,

  chordPan0,
  chordPan1,
  chordPan2,
  chordPan3,

  nVoice,
  smoothness,

  pitchBend,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  Scales()
    : boolScale(1)
    , defaultScale(0.0, 1.0)
    , masterOctave(-4.0, 4.0)
    , equalTemperament(1.0, 60.0)
    , pitchMultiply(0.0, 4.0)
    , pitchModulo(0.0, 60.0)
    , seed(16777215)
    , randomAmount(0.0, 1.0, 0.5, 0.1)
    , chorusFrequency(-40.0, 40.0, 0.6)
    , chorusDelayTimeRange(0.00003, 0.001, 0.5, 0.0001)
    , chorusMinDelayTime(0, 0.01, 0.5, 0.001)
    , chorusPhase(0, SomeDSP::twopi)
    , chorusOffset(0, SomeDSP::twopi / 3.0)
    , oscSemi(120)
    , oscMilli(1000)
    , oscOctave(8)
    , gain(0.0, 4.0, 0.5, 1.0)
    , gainBoost(1.0, 16.0)
    , gainDecibel(-40.0, 0.0, true)
    , shelvingPitch(0.0, 60.0)
    , shelvingGain(-30.0, 30.0, true)
    , envelopeA(0.0001, 16.0, 0.5, 2.0)
    , envelopeD(0.0001, 16.0, 0.5, 4.0)
    , envelopeS(0.0, 0.9995, 0.5, 0.3)
    , envelopeR(0.001, 16.0, 0.5, 2.0)
    , nVoice(5)
    , smoothness(0.0, 0.5, 0.1, 0.04)
  {
  }

  SomeDSP::UIntScale<double> boolScale;
  SomeDSP::LinearScale<double> defaultScale;

  SomeDSP::LinearScale<double> masterOctave;
  SomeDSP::LinearScale<double> equalTemperament;
  SomeDSP::LinearScale<double> pitchMultiply;
  SomeDSP::LinearScale<double> pitchModulo;

  SomeDSP::UIntScale<double> seed;
  SomeDSP::LogScale<double> randomAmount;

  SomeDSP::SPolyScale<double> chorusFrequency;
  SomeDSP::LogScale<double> chorusDelayTimeRange;
  SomeDSP::LogScale<double> chorusMinDelayTime;
  SomeDSP::LinearScale<double> chorusPhase;
  SomeDSP::LinearScale<double> chorusOffset;

  SomeDSP::UIntScale<double> oscSemi;
  SomeDSP::UIntScale<double> oscMilli;
  SomeDSP::UIntScale<double> oscOctave;

  SomeDSP::LogScale<double> gain;
  SomeDSP::LinearScale<double> gainBoost;
  SomeDSP::DecibelScale<double> gainDecibel;

  SomeDSP::LinearScale<double> shelvingPitch;
  SomeDSP::DecibelScale<double> shelvingGain;

  SomeDSP::LogScale<double> envelopeA;
  SomeDSP::LogScale<double> envelopeD;
  SomeDSP::LogScale<double> envelopeS;
  SomeDSP::LogScale<double> envelopeR;

  SomeDSP::UIntScale<double> nVoice;
  SomeDSP::LogScale<double> smoothness;
};

struct PlugParameter {
  Scales scale;
  std::vector<std::unique_ptr<ValueInterface>> value;

  PlugParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = FloatValue<SomeDSP::LinearScale<double>>;
    using LogValue = FloatValue<SomeDSP::LogScale<double>>;
    using SPolyValue = FloatValue<SomeDSP::SPolyScale<double>>;
    using DecibelValue = FloatValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, scale.boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::aliasing]
      = std::make_unique<UIntValue>(0, scale.boolScale, "aliasing", Info::kCanAutomate);
    value[ID::negativeSemi] = std::make_unique<UIntValue>(
      0, scale.boolScale, "negativeSemi", Info::kCanAutomate);
    value[ID::masterOctave] = std::make_unique<LinearValue>(
      0.5, scale.masterOctave, "masterOctave", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<LinearValue>(
      scale.equalTemperament.invmap(12), scale.equalTemperament, "equalTemperament",
      Info::kCanAutomate);
    value[ID::pitchMultiply] = std::make_unique<LinearValue>(
      0.25, scale.pitchMultiply, "pitchMultiply", Info::kCanAutomate);
    value[ID::pitchModulo] = std::make_unique<LinearValue>(
      0.0, scale.pitchModulo, "pitchModulo", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, scale.seed, "seed", Info::kCanAutomate);
    value[ID::randomRetrigger] = std::make_unique<UIntValue>(
      0, scale.boolScale, "randomRetrigger", Info::kCanAutomate);
    value[ID::randomGainAmount] = std::make_unique<LogValue>(
      0.0, scale.randomAmount, "randomGainAmount", Info::kCanAutomate);
    value[ID::randomFrequencyAmount] = std::make_unique<LogValue>(
      0.0, scale.randomAmount, "randomFrequencyAmount", Info::kCanAutomate);

    value[ID::chorusMix] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "chorusMix", Info::kCanAutomate);
    value[ID::chorusFrequency] = std::make_unique<SPolyValue>(
      0.5, scale.chorusFrequency, "chorusFrequency", Info::kCanAutomate);
    value[ID::chorusDepth] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "chorusDepth", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange0] = std::make_unique<LogValue>(
      0.0, scale.chorusDelayTimeRange, "chorusDelayTimeRange0", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange1] = std::make_unique<LogValue>(
      0.0, scale.chorusDelayTimeRange, "chorusDelayTimeRange1", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange2] = std::make_unique<LogValue>(
      0.0, scale.chorusDelayTimeRange, "chorusDelayTimeRange2", Info::kCanAutomate);
    value[ID::chorusMinDelayTime0] = std::make_unique<LogValue>(
      0.5, scale.chorusMinDelayTime, "chorusMinDelayTime0", Info::kCanAutomate);
    value[ID::chorusMinDelayTime1] = std::make_unique<LogValue>(
      0.5, scale.chorusMinDelayTime, "chorusMinDelayTime1", Info::kCanAutomate);
    value[ID::chorusMinDelayTime2] = std::make_unique<LogValue>(
      0.5, scale.chorusMinDelayTime, "chorusMinDelayTime2", Info::kCanAutomate);
    value[ID::chorusPhase] = std::make_unique<LinearValue>(
      0.0, scale.chorusPhase, "chorusPhase", Info::kCanAutomate);
    value[ID::chorusOffset] = std::make_unique<LinearValue>(
      0.0, scale.chorusOffset, "chorusOffset", Info::kCanAutomate);
    value[ID::chorusFeedback] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "chorusFeedback", Info::kCanAutomate);
    value[ID::chorusKeyFollow] = std::make_unique<UIntValue>(
      true, scale.boolScale, "chorusKeyFollow", Info::kCanAutomate);

    value[ID::gain0] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain0", Info::kCanAutomate);
    value[ID::gain1] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain1", Info::kCanAutomate);
    value[ID::gain2] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain2", Info::kCanAutomate);
    value[ID::gain3] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain3", Info::kCanAutomate);
    value[ID::gain4] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain4", Info::kCanAutomate);
    value[ID::gain5] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain5", Info::kCanAutomate);
    value[ID::gain6] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain6", Info::kCanAutomate);
    value[ID::gain7] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "gain7", Info::kCanAutomate);

    value[ID::semi0]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi0", Info::kCanAutomate);
    value[ID::semi1]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi1", Info::kCanAutomate);
    value[ID::semi2]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi2", Info::kCanAutomate);
    value[ID::semi3]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi3", Info::kCanAutomate);
    value[ID::semi4]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi4", Info::kCanAutomate);
    value[ID::semi5]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi5", Info::kCanAutomate);
    value[ID::semi6]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi6", Info::kCanAutomate);
    value[ID::semi7]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "semi7", Info::kCanAutomate);

    value[ID::milli0]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli0", Info::kCanAutomate);
    value[ID::milli1]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli1", Info::kCanAutomate);
    value[ID::milli2]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli2", Info::kCanAutomate);
    value[ID::milli3]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli3", Info::kCanAutomate);
    value[ID::milli4]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli4", Info::kCanAutomate);
    value[ID::milli5]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli5", Info::kCanAutomate);
    value[ID::milli6]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli6", Info::kCanAutomate);
    value[ID::milli7]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "milli7", Info::kCanAutomate);

    value[ID::overtone1] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0), scale.gainDecibel, "overtone1", Info::kCanAutomate);
    value[ID::overtone2] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 2), scale.gainDecibel, "overtone2",
      Info::kCanAutomate);
    value[ID::overtone3] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 3), scale.gainDecibel, "overtone3",
      Info::kCanAutomate);
    value[ID::overtone4] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 4), scale.gainDecibel, "overtone4",
      Info::kCanAutomate);
    value[ID::overtone5] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 5), scale.gainDecibel, "overtone5",
      Info::kCanAutomate);
    value[ID::overtone6] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 6), scale.gainDecibel, "overtone6",
      Info::kCanAutomate);
    value[ID::overtone7] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 7), scale.gainDecibel, "overtone7",
      Info::kCanAutomate);
    value[ID::overtone8] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 8), scale.gainDecibel, "overtone8",
      Info::kCanAutomate);
    value[ID::overtone9] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 9), scale.gainDecibel, "overtone9",
      Info::kCanAutomate);
    value[ID::overtone10] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 10), scale.gainDecibel, "overtone10",
      Info::kCanAutomate);
    value[ID::overtone11] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 11), scale.gainDecibel, "overtone11",
      Info::kCanAutomate);
    value[ID::overtone12] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 12), scale.gainDecibel, "overtone12",
      Info::kCanAutomate);
    value[ID::overtone13] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 13), scale.gainDecibel, "overtone13",
      Info::kCanAutomate);
    value[ID::overtone14] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 14), scale.gainDecibel, "overtone14",
      Info::kCanAutomate);
    value[ID::overtone15] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 15), scale.gainDecibel, "overtone15",
      Info::kCanAutomate);
    value[ID::overtone16] = std::make_unique<DecibelValue>(
      scale.gainDecibel.invmap(1.0 / 16), scale.gainDecibel, "overtone16",
      Info::kCanAutomate);

    value[ID::chordGain0] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "chordGain0", Info::kCanAutomate);
    value[ID::chordGain1] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "chordGain1", Info::kCanAutomate);
    value[ID::chordGain2] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "chordGain2", Info::kCanAutomate);
    value[ID::chordGain3] = std::make_unique<DecibelValue>(
      1.0, scale.gainDecibel, "chordGain3", Info::kCanAutomate);

    value[ID::chordSemi0]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "chordSemi0", Info::kCanAutomate);
    value[ID::chordSemi1]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "chordSemi1", Info::kCanAutomate);
    value[ID::chordSemi2]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "chordSemi2", Info::kCanAutomate);
    value[ID::chordSemi3]
      = std::make_unique<UIntValue>(0, scale.oscSemi, "chordSemi3", Info::kCanAutomate);

    value[ID::chordMilli0]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "chordMilli0", Info::kCanAutomate);
    value[ID::chordMilli1]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "chordMilli1", Info::kCanAutomate);
    value[ID::chordMilli2]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "chordMilli2", Info::kCanAutomate);
    value[ID::chordMilli3]
      = std::make_unique<UIntValue>(0, scale.oscMilli, "chordMilli3", Info::kCanAutomate);

    value[ID::chordPan0] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "chordPan0", Info::kCanAutomate);
    value[ID::chordPan1] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "chordPan1", Info::kCanAutomate);
    value[ID::chordPan2] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "chordPan2", Info::kCanAutomate);
    value[ID::chordPan3] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "chordPan3", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, scale.gain, "gain", Info::kCanAutomate);
    value[ID::gainBoost] = std::make_unique<LinearValue>(
      0.0, scale.gainBoost, "gainBoost", Info::kCanAutomate);
    value[ID::gainA]
      = std::make_unique<LogValue>(0.0, scale.envelopeA, "gainA", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<LogValue>(0.5, scale.envelopeD, "gainD", Info::kCanAutomate);
    value[ID::gainS]
      = std::make_unique<LogValue>(0.5, scale.envelopeS, "gainS", Info::kCanAutomate);
    value[ID::gainR]
      = std::make_unique<LogValue>(0.0, scale.envelopeR, "gainR", Info::kCanAutomate);
    value[ID::gainEnvelopeCurve] = std::make_unique<LinearValue>(
      0.0, scale.defaultScale, "gainEnvelopeCurve", Info::kCanAutomate);

    value[ID::lowShelfPitch] = std::make_unique<LinearValue>(
      0.0, scale.shelvingPitch, "lowShelfPitch", Info::kCanAutomate);
    value[ID::lowShelfGain] = std::make_unique<DecibelValue>(
      0.5, scale.shelvingGain, "lowShelfGain", Info::kCanAutomate);
    value[ID::highShelfPitch] = std::make_unique<LinearValue>(
      0.0, scale.shelvingPitch, "highShelfPitch", Info::kCanAutomate);
    value[ID::highShelfGain] = std::make_unique<DecibelValue>(
      0.5, scale.shelvingGain, "highShelfGain", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<UIntValue>(5, scale.nVoice, "nVoice", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, scale.smoothness, "smoothness", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, scale.defaultScale, "pitchBend", Info::kCanAutomate);

    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
  }

  tresult setState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->setState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult getState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->getState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters)
  {
    for (auto &val : value)
      if (val->addParameter(parameters)) return kResultFalse;
    return kResultOk;
  }

  double getDefaultNormalized(int32_t tag)
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
};

} // namespace Synth
} // namespace Steinberg
