// (c) 2023 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

static constexpr size_t nPolyOscControl = 13;

constexpr size_t nReservedParameter = 256;
constexpr size_t nReservedGuiParameter = 64;

constexpr int transposeOctaveOffset = 10;
constexpr int transposeSemitoneOffset = 36;

namespace Steinberg {
namespace Synth {

enum Tuning {
  equalTemperament12,
  equalTemperament5,
  equalTemperament10Major, // TODO
  equalTemperament10Minor, // TODO
  justIntonation5LimitMajor,
  justIntonation5LimitMinor, // TODO

  Tuning_ENUM_LENGTH,
};

enum PitchScale {
  octave,
  et5Chromatic,
  et12Major,
  et12Minor,
  overtone32,

  PitchScale_ENUM_LENGTH,
};

namespace ParameterID {
enum ID {
  bypass,

  outputGain,

  decaySeconds,
  oscSync,
  fmIndex,
  saturationGain,

  randomizeFmIndex,

  seed,
  transposeOctave,
  transposeSemitone,
  transposeCent,
  tuning,

  polyphonic,
  release,

  polynomialPointX0,
  polynomialPointY0 = polynomialPointX0 + nPolyOscControl,

  arpeggioSwitch = polynomialPointY0 + nPolyOscControl,
  arpeggioNotesPerBeat,
  arpeggioLoopLengthInBeat,
  arpeggioDurationVariation, // TODO
  arpeggioRestChance,        // TODO
  arpeggioScale,
  arpeggioPicthDriftCent,
  arpeggioOctave,

  reservedParameter0,
  reservedGuiParameter0 = reservedParameter0 + nReservedParameter,

  ID_ENUM_LENGTH = reservedGuiParameter0 + nReservedGuiParameter,
  // ID_ENUM_GUI_START = reservedGuiParameter0,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::DecibelScale<double> decaySeconds;
  static SomeDSP::DecibelScale<double> fmIndex;
  static SomeDSP::LinearScale<double> randomizeFmIndex;

  static SomeDSP::LinearScale<double> polynomialPointY;

  static SomeDSP::UIntScale<double> transposeOctave;
  static SomeDSP::UIntScale<double> transposeSemitone;
  static SomeDSP::LinearScale<double> transposeCent;
  static SomeDSP::UIntScale<double> tuning;

  static SomeDSP::UIntScale<double> arpeggioNotesPerBeat;
  static SomeDSP::UIntScale<double> arpeggioLoopLengthInBeat;
  static SomeDSP::UIntScale<double> arpeggioDurationVariation;
  static SomeDSP::UIntScale<double> arpeggioScale;
  static SomeDSP::LinearScale<double> arpeggioPicthDriftCent;
  static SomeDSP::UIntScale<double> arpeggioOctave;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using NegativeDecibelValue = DoubleValue<SomeDSP::NegativeDecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);

    value[ID::decaySeconds] = std::make_unique<DecibelValue>(
      Scales::decaySeconds.invmap(1.0), Scales::decaySeconds, "decaySeconds",
      Info::kCanAutomate);
    value[ID::oscSync] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(1.0), Scales::defaultScale, "oscSync",
      Info::kCanAutomate);
    value[ID::fmIndex] = std::make_unique<DecibelValue>(
      Scales::fmIndex.invmap(0.0), Scales::fmIndex, "fmIndex", Info::kCanAutomate);
    value[ID::saturationGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "saturationGain", Info::kCanAutomate);

    value[ID::randomizeFmIndex] = std::make_unique<LinearValue>(
      Scales::randomizeFmIndex.invmap(0.0), Scales::randomizeFmIndex, "randomizeFmIndex",
      Info::kCanAutomate);

    value[ID::seed]
      = std::make_unique<UIntValue>(0, Scales::seed, "seed", Info::kCanAutomate);
    value[ID::transposeOctave] = std::make_unique<UIntValue>(
      transposeOctaveOffset, Scales::transposeOctave, "transposeOctave",
      Info::kCanAutomate);
    value[ID::transposeSemitone] = std::make_unique<UIntValue>(
      transposeSemitoneOffset, Scales::transposeSemitone, "transposeSemitone",
      Info::kCanAutomate);
    value[ID::transposeCent] = std::make_unique<LinearValue>(
      Scales::transposeCent.invmap(0.0), Scales::transposeCent, "transposeCent",
      Info::kCanAutomate);
    value[ID::tuning]
      = std::make_unique<UIntValue>(0, Scales::tuning, "tuning", Info::kCanAutomate);

    value[ID::polyphonic] = std::make_unique<UIntValue>(
      1, Scales::boolScale, "polyphonic", Info::kCanAutomate);
    value[ID::release]
      = std::make_unique<UIntValue>(1, Scales::boolScale, "release", Info::kCanAutomate);

    for (size_t idx = 0; idx < nPolyOscControl; ++idx) {
      auto indexStr = std::to_string(idx);
      auto ratio = double(idx + 1) / double(nPolyOscControl + 1);
      value[ID::polynomialPointX0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(ratio), Scales::defaultScale,
        ("polynomialPointX" + indexStr).c_str(), Info::kCanAutomate);
      value[ID::polynomialPointY0 + idx] = std::make_unique<LinearValue>(
        Scales::polynomialPointY.invmap(0.5 * std::sin(SomeDSP::twopi * ratio)),
        Scales::polynomialPointY, ("polynomialPointY" + indexStr).c_str(),
        Info::kCanAutomate);
    }

    value[ID::arpeggioSwitch] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "arpeggioSwitch", Info::kCanAutomate);
    value[ID::arpeggioNotesPerBeat] = std::make_unique<UIntValue>(
      3, Scales::arpeggioNotesPerBeat, "arpeggioNotesPerBeat", Info::kCanAutomate);
    value[ID::arpeggioLoopLengthInBeat] = std::make_unique<UIntValue>(
      4, Scales::arpeggioLoopLengthInBeat, "arpeggioLoopLengthInBeat",
      Info::kCanAutomate);
    value[ID::arpeggioDurationVariation] = std::make_unique<UIntValue>(
      0, Scales::arpeggioDurationVariation, "arpeggioDurationVariation",
      Info::kCanAutomate);
    value[ID::arpeggioRestChance] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.0), Scales::defaultScale, "arpeggioRestChance",
      Info::kCanAutomate);
    value[ID::arpeggioScale] = std::make_unique<UIntValue>(
      0, Scales::arpeggioScale, "arpeggioScale", Info::kCanAutomate);
    value[ID::arpeggioPicthDriftCent] = std::make_unique<LinearValue>(
      Scales::arpeggioPicthDriftCent.invmap(0), Scales::arpeggioPicthDriftCent,
      "arpeggioPicthDriftCent", Info::kCanAutomate);
    value[ID::arpeggioOctave] = std::make_unique<UIntValue>(
      1, Scales::arpeggioOctave, "arpeggioOctave", Info::kCanAutomate);

    for (size_t idx = 0; idx < nReservedParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedParameter" + indexStr).c_str(), Info::kIsHidden);
    }

    for (size_t idx = 0; idx < nReservedGuiParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedGuiParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(1.0), Scales::defaultScale,
        ("reservedGuiParameter" + indexStr).c_str(), Info::kIsHidden);
    }

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
