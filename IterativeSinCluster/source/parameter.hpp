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

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
#include "../../test/value.hpp"
#else
#include "../../common/value.hpp"
#endif

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
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LinearScale<double> masterOctave;
  static SomeDSP::LinearScale<double> equalTemperament;
  static SomeDSP::LinearScale<double> pitchMultiply;
  static SomeDSP::LinearScale<double> pitchModulo;

  static SomeDSP::UIntScale<double> seed;
  static SomeDSP::LogScale<double> randomAmount;

  static SomeDSP::SPolyScale<double> chorusFrequency;
  static SomeDSP::LogScale<double> chorusDelayTimeRange;
  static SomeDSP::LogScale<double> chorusMinDelayTime;
  static SomeDSP::LinearScale<double> chorusPhase;
  static SomeDSP::LinearScale<double> chorusOffset;

  static SomeDSP::UIntScale<double> oscSemi;
  static SomeDSP::UIntScale<double> oscMilli;
  static SomeDSP::UIntScale<double> oscOctave;

  static SomeDSP::LogScale<double> gain;
  static SomeDSP::LinearScale<double> gainBoost;
  static SomeDSP::DecibelScale<double> gainDecibel;

  static SomeDSP::LinearScale<double> shelvingPitch;
  static SomeDSP::DecibelScale<double> shelvingGain;

  static SomeDSP::LogScale<double> envelopeA;
  static SomeDSP::LogScale<double> envelopeD;
  static SomeDSP::LogScale<double> envelopeS;
  static SomeDSP::LogScale<double> envelopeR;

  static SomeDSP::UIntScale<double> nVoice;
  static SomeDSP::LogScale<double> smoothness;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using LogValue = DoubleValue<SomeDSP::LogScale<double>>;
    using SPolyValue = DoubleValue<SomeDSP::SPolyScale<double>>;
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::aliasing]
      = std::make_unique<UIntValue>(0, Scales::boolScale, "aliasing", Info::kCanAutomate);
    value[ID::negativeSemi] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "negativeSemi", Info::kCanAutomate);
    value[ID::masterOctave] = std::make_unique<LinearValue>(
      0.5, Scales::masterOctave, "masterOctave", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<LinearValue>(
      Scales::equalTemperament.invmap(12), Scales::equalTemperament, "equalTemperament",
      Info::kCanAutomate);
    value[ID::pitchMultiply] = std::make_unique<LinearValue>(
      0.25, Scales::pitchMultiply, "pitchMultiply", Info::kCanAutomate);
    value[ID::pitchModulo] = std::make_unique<LinearValue>(
      0.0, Scales::pitchModulo, "pitchModulo", Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::randomRetrigger] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "randomRetrigger", Info::kCanAutomate);
    value[ID::randomGainAmount] = std::make_unique<LogValue>(
      0.0, Scales::randomAmount, "randomGainAmount", Info::kCanAutomate);
    value[ID::randomFrequencyAmount] = std::make_unique<LogValue>(
      0.0, Scales::randomAmount, "randomFrequencyAmount", Info::kCanAutomate);

    value[ID::chorusMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "chorusMix", Info::kCanAutomate);
    value[ID::chorusFrequency] = std::make_unique<SPolyValue>(
      0.5, Scales::chorusFrequency, "chorusFrequency", Info::kCanAutomate);
    value[ID::chorusDepth] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "chorusDepth", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange0] = std::make_unique<LogValue>(
      0.0, Scales::chorusDelayTimeRange, "chorusDelayTimeRange0", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange1] = std::make_unique<LogValue>(
      0.0, Scales::chorusDelayTimeRange, "chorusDelayTimeRange1", Info::kCanAutomate);
    value[ID::chorusDelayTimeRange2] = std::make_unique<LogValue>(
      0.0, Scales::chorusDelayTimeRange, "chorusDelayTimeRange2", Info::kCanAutomate);
    value[ID::chorusMinDelayTime0] = std::make_unique<LogValue>(
      0.5, Scales::chorusMinDelayTime, "chorusMinDelayTime0", Info::kCanAutomate);
    value[ID::chorusMinDelayTime1] = std::make_unique<LogValue>(
      0.5, Scales::chorusMinDelayTime, "chorusMinDelayTime1", Info::kCanAutomate);
    value[ID::chorusMinDelayTime2] = std::make_unique<LogValue>(
      0.5, Scales::chorusMinDelayTime, "chorusMinDelayTime2", Info::kCanAutomate);
    value[ID::chorusPhase] = std::make_unique<LinearValue>(
      0.0, Scales::chorusPhase, "chorusPhase", Info::kCanAutomate);
    value[ID::chorusOffset] = std::make_unique<LinearValue>(
      0.0, Scales::chorusOffset, "chorusOffset", Info::kCanAutomate);
    value[ID::chorusFeedback] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "chorusFeedback", Info::kCanAutomate);
    value[ID::chorusKeyFollow] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "chorusKeyFollow", Info::kCanAutomate);

    value[ID::gain]
      = std::make_unique<LogValue>(0.5, Scales::gain, "gain", Info::kCanAutomate);
    value[ID::gainBoost] = std::make_unique<LinearValue>(
      0.0, Scales::gainBoost, "gainBoost", Info::kCanAutomate);
    value[ID::gainA]
      = std::make_unique<LogValue>(0.0, Scales::envelopeA, "gainA", Info::kCanAutomate);
    value[ID::gainD]
      = std::make_unique<LogValue>(0.5, Scales::envelopeD, "gainD", Info::kCanAutomate);
    value[ID::gainS]
      = std::make_unique<LogValue>(0.5, Scales::envelopeS, "gainS", Info::kCanAutomate);
    value[ID::gainR]
      = std::make_unique<LogValue>(0.0, Scales::envelopeR, "gainR", Info::kCanAutomate);
    value[ID::gainEnvelopeCurve] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "gainEnvelopeCurve", Info::kCanAutomate);

    value[ID::lowShelfPitch] = std::make_unique<LinearValue>(
      0.0, Scales::shelvingPitch, "lowShelfPitch", Info::kCanAutomate);
    value[ID::lowShelfGain] = std::make_unique<DecibelValue>(
      0.5, Scales::shelvingGain, "lowShelfGain", Info::kCanAutomate);
    value[ID::highShelfPitch] = std::make_unique<LinearValue>(
      0.0, Scales::shelvingPitch, "highShelfPitch", Info::kCanAutomate);
    value[ID::highShelfGain] = std::make_unique<DecibelValue>(
      0.5, Scales::shelvingGain, "highShelfGain", Info::kCanAutomate);

    value[ID::gain0] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain0", Info::kCanAutomate);
    value[ID::gain1] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain1", Info::kCanAutomate);
    value[ID::gain2] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain2", Info::kCanAutomate);
    value[ID::gain3] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain3", Info::kCanAutomate);
    value[ID::gain4] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain4", Info::kCanAutomate);
    value[ID::gain5] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain5", Info::kCanAutomate);
    value[ID::gain6] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain6", Info::kCanAutomate);
    value[ID::gain7] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "gain7", Info::kCanAutomate);

    value[ID::semi0]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi0", Info::kCanAutomate);
    value[ID::semi1]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi1", Info::kCanAutomate);
    value[ID::semi2]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi2", Info::kCanAutomate);
    value[ID::semi3]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi3", Info::kCanAutomate);
    value[ID::semi4]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi4", Info::kCanAutomate);
    value[ID::semi5]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi5", Info::kCanAutomate);
    value[ID::semi6]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi6", Info::kCanAutomate);
    value[ID::semi7]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "semi7", Info::kCanAutomate);

    value[ID::milli0]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli0", Info::kCanAutomate);
    value[ID::milli1]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli1", Info::kCanAutomate);
    value[ID::milli2]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli2", Info::kCanAutomate);
    value[ID::milli3]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli3", Info::kCanAutomate);
    value[ID::milli4]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli4", Info::kCanAutomate);
    value[ID::milli5]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli5", Info::kCanAutomate);
    value[ID::milli6]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli6", Info::kCanAutomate);
    value[ID::milli7]
      = std::make_unique<UIntValue>(0, Scales::oscMilli, "milli7", Info::kCanAutomate);

    value[ID::overtone1] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0), Scales::gainDecibel, "overtone1",
      Info::kCanAutomate);
    value[ID::overtone2] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 2), Scales::gainDecibel, "overtone2",
      Info::kCanAutomate);
    value[ID::overtone3] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 3), Scales::gainDecibel, "overtone3",
      Info::kCanAutomate);
    value[ID::overtone4] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 4), Scales::gainDecibel, "overtone4",
      Info::kCanAutomate);
    value[ID::overtone5] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 5), Scales::gainDecibel, "overtone5",
      Info::kCanAutomate);
    value[ID::overtone6] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 6), Scales::gainDecibel, "overtone6",
      Info::kCanAutomate);
    value[ID::overtone7] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 7), Scales::gainDecibel, "overtone7",
      Info::kCanAutomate);
    value[ID::overtone8] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 8), Scales::gainDecibel, "overtone8",
      Info::kCanAutomate);
    value[ID::overtone9] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 9), Scales::gainDecibel, "overtone9",
      Info::kCanAutomate);
    value[ID::overtone10] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 10), Scales::gainDecibel, "overtone10",
      Info::kCanAutomate);
    value[ID::overtone11] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 11), Scales::gainDecibel, "overtone11",
      Info::kCanAutomate);
    value[ID::overtone12] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 12), Scales::gainDecibel, "overtone12",
      Info::kCanAutomate);
    value[ID::overtone13] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 13), Scales::gainDecibel, "overtone13",
      Info::kCanAutomate);
    value[ID::overtone14] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 14), Scales::gainDecibel, "overtone14",
      Info::kCanAutomate);
    value[ID::overtone15] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 15), Scales::gainDecibel, "overtone15",
      Info::kCanAutomate);
    value[ID::overtone16] = std::make_unique<DecibelValue>(
      Scales::gainDecibel.invmap(1.0 / 16), Scales::gainDecibel, "overtone16",
      Info::kCanAutomate);

    value[ID::chordGain0] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "chordGain0", Info::kCanAutomate);
    value[ID::chordGain1] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "chordGain1", Info::kCanAutomate);
    value[ID::chordGain2] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "chordGain2", Info::kCanAutomate);
    value[ID::chordGain3] = std::make_unique<DecibelValue>(
      1.0, Scales::gainDecibel, "chordGain3", Info::kCanAutomate);

    value[ID::chordSemi0]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "chordSemi0", Info::kCanAutomate);
    value[ID::chordSemi1]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "chordSemi1", Info::kCanAutomate);
    value[ID::chordSemi2]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "chordSemi2", Info::kCanAutomate);
    value[ID::chordSemi3]
      = std::make_unique<UIntValue>(0, Scales::oscSemi, "chordSemi3", Info::kCanAutomate);

    value[ID::chordMilli0] = std::make_unique<UIntValue>(
      0, Scales::oscMilli, "chordMilli0", Info::kCanAutomate);
    value[ID::chordMilli1] = std::make_unique<UIntValue>(
      0, Scales::oscMilli, "chordMilli1", Info::kCanAutomate);
    value[ID::chordMilli2] = std::make_unique<UIntValue>(
      0, Scales::oscMilli, "chordMilli2", Info::kCanAutomate);
    value[ID::chordMilli3] = std::make_unique<UIntValue>(
      0, Scales::oscMilli, "chordMilli3", Info::kCanAutomate);

    value[ID::chordPan0] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "chordPan0", Info::kCanAutomate);
    value[ID::chordPan1] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "chordPan1", Info::kCanAutomate);
    value[ID::chordPan2] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "chordPan2", Info::kCanAutomate);
    value[ID::chordPan3] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "chordPan3", Info::kCanAutomate);

    value[ID::nVoice]
      = std::make_unique<UIntValue>(5, Scales::nVoice, "nVoice", Info::kCanAutomate);
    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::defaultScale, "pitchBend", Info::kCanAutomate);

    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
  }

#ifdef TEST_DSP
  // Not used in DSP test.
  double getDefaultNormalized(int32_t) { return 0.0; }

#else
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

  double getDefaultNormalized(int32_t tag) override
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
#endif
};

} // namespace Synth
} // namespace Steinberg
