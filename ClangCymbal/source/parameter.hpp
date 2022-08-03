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

#include <memory>
#include <string>
#include <vector>

#include "../../common/dsp/constants.hpp"
#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr size_t fdnMatrixSize = 64;
constexpr size_t nModEnvelopeWavetable = 128;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  gain,
  gateAttackSecond,
  gateReleaseSecond,

  octave,
  semitone,
  milli,
  equalTemperament,
  pitchA4Hz,
  pitchBend,
  pitchBendRange,

  resetAtNoteOn,
  smoothingTimeSecond,

  impulseGain,
  oscGain,
  oscNoisePulseRatio,
  oscAttack,
  oscDecay,
  oscDensityHz,
  oscDensityKeyFollow,
  oscNoiseDecay,
  oscBounce,
  oscBounceCurve,
  oscJitter,
  oscPulseGainRandomness,
  oscLowpassCutoffSemi,
  oscLowpassQ,
  oscLowpassKeyFollow,

  fdnEnable,
  fdnMatrixIdentityAmount,
  fdnFeedback,
  fdnOvertoneAdd,
  fdnOvertoneMul,
  fdnOvertoneOffset,
  fdnOvertoneModulo,
  fdnOvertoneRandomness,
  fdnInterpRate,
  fdnInterpLowpassSecond,
  fdnSeed,
  fdnRandomizeRatio,

  fdnLowpassCutoffSemiOffset0,
  fdnLowpassQOffset0 = fdnLowpassCutoffSemiOffset0 + fdnMatrixSize,
  fdnHighpassCutoffSemiOffset0 = fdnLowpassQOffset0 + fdnMatrixSize,
  fdnHighpassQOffset0 = fdnHighpassCutoffSemiOffset0 + fdnMatrixSize,
  fdnLowpassCutoffSemi = fdnHighpassQOffset0 + fdnMatrixSize,
  fdnLowpassCutoffSlope,
  fdnLowpassQ,
  fdnLowpassQSlope,
  fdnLowpassKeyFollow,
  fdnHighpassCutoffSemi,
  fdnHighpassCutoffSlope,
  fdnHighpassQ,
  fdnHighpassQSlope,
  fdnHighpassKeyFollow,

  modEnvelopeWavetable0,
  modEnvelopeInterpolation = modEnvelopeWavetable0 + nModEnvelopeWavetable,
  modEnvelopeTime,
  modEnvelopeToFdnLowpassCutoff,
  modEnvelopeToFdnHighpassCutoff,
  modEnvelopeToFdnPitch,
  modEnvelopeToFdnOvertoneAdd,
  modEnvelopeToOscJitter,
  modEnvelopeToOscNoisePulseRatio,

  tremoloMix,
  tremoloDepth,
  tremoloDelayTime,
  tremoloModulationToDelayTimeOffset,
  tremoloModulationSmoothingHz,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;
  static SomeDSP::UIntScale<double> seed;

  static SomeDSP::DecibelScale<double> gain;
  static SomeDSP::DecibelScale<double> gateAttackSecond;
  static SomeDSP::DecibelScale<double> gateReleaseSecond;

  static SomeDSP::UIntScale<double> octave;
  static SomeDSP::UIntScale<double> semitone;
  static SomeDSP::UIntScale<double> milli;
  static SomeDSP::UIntScale<double> equalTemperament;
  static SomeDSP::UIntScale<double> pitchA4Hz;
  static SomeDSP::LinearScale<double> pitchBend;
  static SomeDSP::LinearScale<double> pitchBendRange;

  static SomeDSP::DecibelScale<double> smoothingTimeSecond;

  static SomeDSP::LinearScale<double> oscOvertone;
  static SomeDSP::DecibelScale<double> oscAttack;
  static SomeDSP::DecibelScale<double> oscDecay;
  static SomeDSP::DecibelScale<double> oscDensityHz;
  static SomeDSP::LinearScale<double> oscDensityKeyFollow;
  static SomeDSP::DecibelScale<double> oscBounceCurve;
  static SomeDSP::DecibelScale<double> oscLowpassQ;

  static SomeDSP::DecibelScale<double> impulseGain;

  static SomeDSP::DecibelScale<double> fdnMatrixIdentityAmount;
  static SomeDSP::NegativeDecibelScale<double> fdnFeedback;
  static SomeDSP::DecibelScale<double> fdnOvertoneAdd;
  static SomeDSP::DecibelScale<double> fdnOvertoneMul;
  static SomeDSP::LinearScale<double> fdnOvertoneOffset;
  static SomeDSP::DecibelScale<double> fdnOvertoneModulo;
  static SomeDSP::DecibelScale<double> fdnInterpRate;
  static SomeDSP::DecibelScale<double> fdnInterpLowpassSecond;

  static SomeDSP::LinearScale<double> filterCutoffSemi;
  static SomeDSP::LinearScale<double> filterCutoffSlope;
  static SomeDSP::LinearScale<double> filterQ;
  static SomeDSP::LinearScale<double> filterQSlope;
  static SomeDSP::LinearScale<double> filterCutoffSemiOffset;
  static SomeDSP::LinearScale<double> filterQOffset;

  static SomeDSP::LinearScale<double> wavetableAmp;
  static SomeDSP::UIntScale<double> wavetableInterpolation;

  static SomeDSP::LinearScale<double> lfoToPitchAmount;
  static SomeDSP::DecibelScale<double> modEnvelopeTime;

  static SomeDSP::LinearScale<double> tremoloDepth;
  static SomeDSP::DecibelScale<double> tremoloDelayTime;
  static SomeDSP::DecibelScale<double> tremoloModulationSmoothingHz;
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
    using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;
    using NegativeDecibelValue = DoubleValue<SomeDSP::NegativeDecibelScale<double>>;
    using SemitoneValue = DoubleValue<SomeDSP::SemitoneScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::gain] = std::make_unique<DecibelValue>(
      Scales::gain.invmapDB(0.0), Scales::gain, "gain", Info::kCanAutomate);
    value[ID::gateAttackSecond] = std::make_unique<DecibelValue>(
      Scales::gateAttackSecond.invmap(0.01), Scales::gateAttackSecond, "gateAttackSecond",
      Info::kCanAutomate);
    value[ID::gateReleaseSecond] = std::make_unique<DecibelValue>(
      Scales::gateReleaseSecond.invmap(0.01), Scales::gateReleaseSecond,
      "gateReleaseSecond", Info::kCanAutomate);

    value[ID::octave]
      = std::make_unique<UIntValue>(12, Scales::octave, "octave", Info::kCanAutomate);
    value[ID::semitone] = std::make_unique<UIntValue>(
      120, Scales::semitone, "semitone", Info::kCanAutomate);
    value[ID::milli]
      = std::make_unique<UIntValue>(1000, Scales::milli, "milli", Info::kCanAutomate);
    value[ID::equalTemperament] = std::make_unique<UIntValue>(
      11, Scales::equalTemperament, "equalTemperament", Info::kCanAutomate);
    value[ID::pitchA4Hz] = std::make_unique<UIntValue>(
      340, Scales::pitchA4Hz, "pitchA4Hz", Info::kCanAutomate);
    value[ID::pitchBend] = std::make_unique<LinearValue>(
      0.5, Scales::pitchBend, "pitchBend", Info::kCanAutomate);
    value[ID::pitchBendRange] = std::make_unique<LinearValue>(
      Scales::pitchBendRange.invmap(2.0), Scales::pitchBendRange, "pitchBendRange",
      Info::kCanAutomate);

    value[ID::resetAtNoteOn] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "bypass", Info::kCanAutomate);
    value[ID::smoothingTimeSecond] = std::make_unique<DecibelValue>(
      Scales::smoothingTimeSecond.invmap(0.02), Scales::smoothingTimeSecond,
      "smoothingTimeSecond", Info::kCanAutomate);

    value[ID::impulseGain] = std::make_unique<DecibelValue>(
      1.0, Scales::impulseGain, "impulseGain", Info::kCanAutomate);
    value[ID::oscGain] = std::make_unique<DecibelValue>(
      Scales::impulseGain.invmapDB(0.0), Scales::impulseGain, "oscGain",
      Info::kCanAutomate);
    value[ID::oscNoisePulseRatio] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "oscNoisePulseRatio", Info::kCanAutomate);
    value[ID::oscAttack] = std::make_unique<DecibelValue>(
      Scales::oscAttack.invmap(0.001), Scales::oscAttack, "oscAttack",
      Info::kCanAutomate);
    value[ID::oscDecay] = std::make_unique<DecibelValue>(
      Scales::oscDecay.invmap(1.0), Scales::oscDecay, "oscDecay", Info::kCanAutomate);
    value[ID::oscDensityHz] = std::make_unique<DecibelValue>(
      Scales::oscDensityHz.invmap(7000.0), Scales::oscDensityHz, "oscDensityHz",
      Info::kCanAutomate);
    value[ID::oscDensityKeyFollow] = std::make_unique<LinearValue>(
      Scales::oscDensityKeyFollow.invmap(1.0), Scales::oscDensityKeyFollow,
      "oscDensityKeyFollow", Info::kCanAutomate);
    value[ID::oscNoiseDecay] = std::make_unique<DecibelValue>(
      Scales::oscDecay.invmap(1.0), Scales::oscDecay, "oscNoiseDecay",
      Info::kCanAutomate);
    value[ID::oscBounce] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "oscBounce", Info::kCanAutomate);
    value[ID::oscBounceCurve] = std::make_unique<DecibelValue>(
      Scales::oscBounceCurve.invmap(1.0), Scales::oscBounceCurve, "oscBounceCurve",
      Info::kCanAutomate);
    value[ID::oscJitter] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "oscJitter", Info::kCanAutomate);
    value[ID::oscPulseGainRandomness] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "oscPulseGainRandomness", Info::kCanAutomate);
    value[ID::oscLowpassCutoffSemi] = std::make_unique<LinearValue>(
      1.0, Scales::filterCutoffSemi, "oscLowpassCutoffSemi", Info::kCanAutomate);
    value[ID::oscLowpassQ] = std::make_unique<DecibelValue>(
      Scales::oscLowpassQ.invmap(SomeDSP::halfSqrt2), Scales::oscLowpassQ, "oscLowpassQ",
      Info::kCanAutomate);
    value[ID::oscLowpassKeyFollow] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "oscLowpassKeyFollow", Info::kCanAutomate);

    value[ID::fdnEnable] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "fdnEnable", Info::kCanAutomate);
    value[ID::fdnMatrixIdentityAmount] = std::make_unique<DecibelValue>(
      Scales::fdnMatrixIdentityAmount.invmap(0.1), Scales::fdnMatrixIdentityAmount,
      "fdnMatrixIdentityAmount", Info::kCanAutomate);
    value[ID::fdnFeedback] = std::make_unique<NegativeDecibelValue>(
      1.0, Scales::fdnFeedback, "fdnFeedback", Info::kCanAutomate);
    value[ID::fdnOvertoneAdd] = std::make_unique<DecibelValue>(
      Scales::fdnOvertoneAdd.invmap(1.0), Scales::fdnOvertoneAdd, "fdnOvertoneAdd",
      Info::kCanAutomate);
    value[ID::fdnOvertoneMul] = std::make_unique<DecibelValue>(
      Scales::fdnOvertoneMul.invmap(1.0), Scales::fdnOvertoneMul, "fdnOvertoneMul",
      Info::kCanAutomate);
    value[ID::fdnOvertoneOffset] = std::make_unique<LinearValue>(
      Scales::fdnOvertoneOffset.invmap(0.0), Scales::fdnOvertoneOffset,
      "fdnOvertoneOffset", Info::kCanAutomate);
    value[ID::fdnOvertoneModulo] = std::make_unique<DecibelValue>(
      Scales::fdnOvertoneModulo.invmap(0.0), Scales::fdnOvertoneModulo,
      "fdnOvertoneModulo", Info::kCanAutomate);
    value[ID::fdnOvertoneRandomness] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "fdnOvertoneRandomness", Info::kCanAutomate);
    value[ID::fdnInterpRate] = std::make_unique<DecibelValue>(
      Scales::fdnInterpRate.invmapDB(0.0), Scales::fdnInterpRate, "fdnInterpRate",
      Info::kCanAutomate);
    value[ID::fdnInterpLowpassSecond] = std::make_unique<DecibelValue>(
      Scales::fdnInterpLowpassSecond.invmap(0.005), Scales::fdnInterpLowpassSecond,
      "fdnInterpLowpassSecond", Info::kCanAutomate);
    value[ID::fdnSeed]
      = std::make_unique<UIntValue>(0, Scales::seed, "fdnSeed", Info::kCanAutomate);
    value[ID::fdnRandomizeRatio] = std::make_unique<LinearValue>(
      0.2, Scales::defaultScale, "fdnRandomizeRatio", Info::kCanAutomate);

    std::string fdnLowpassCutoffSemiLabel("lowpassCutoffSemi");
    std::string fdnLowpassQLabel("fdnLowpassQ");
    std::string fdnHighpassCutoffSemiLabel("fdnHighpassCutoffSemi");
    std::string fdnHighpassQLabel("fdnHighpassQ");
    for (size_t idx = 0; idx < fdnMatrixSize; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::fdnLowpassCutoffSemiOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::filterCutoffSemiOffset.invmap(0.0), Scales::filterCutoffSemiOffset,
        (fdnLowpassCutoffSemiLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::fdnLowpassQOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQOffset.invmap(0.0), Scales::filterQOffset,
        (fdnLowpassQLabel + indexStr).c_str(), Info::kCanAutomate);

      value[ID::fdnHighpassCutoffSemiOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::filterCutoffSemiOffset.invmap(0.0), Scales::filterCutoffSemiOffset,
        (fdnHighpassCutoffSemiLabel + indexStr).c_str(), Info::kCanAutomate);
      value[ID::fdnHighpassQOffset0 + idx] = std::make_unique<LinearValue>(
        Scales::filterQOffset.invmap(0.0), Scales::filterQOffset,
        (fdnHighpassQLabel + indexStr).c_str(), Info::kCanAutomate);
    }
    value[ID::fdnLowpassCutoffSemi] = std::make_unique<LinearValue>(
      1.0, Scales::filterCutoffSemi, "fdnLowpassCutoffSemi", Info::kCanAutomate);
    value[ID::fdnLowpassCutoffSlope] = std::make_unique<LinearValue>(
      Scales::filterCutoffSlope.invmap(0.0), Scales::filterCutoffSlope,
      "fdnLowpassCutoffSlope", Info::kCanAutomate);
    value[ID::fdnLowpassQ] = std::make_unique<LinearValue>(
      1.0, Scales::filterQ, "fdnLowpassQ", Info::kCanAutomate);
    value[ID::fdnLowpassQSlope] = std::make_unique<LinearValue>(
      Scales::filterQSlope.invmap(0.0), Scales::filterQSlope, "fdnLowpassQSlope",
      Info::kCanAutomate);
    value[ID::fdnLowpassKeyFollow] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "fdnLowpassKeyFollow", Info::kCanAutomate);

    value[ID::fdnHighpassCutoffSemi] = std::make_unique<LinearValue>(
      Scales::filterCutoffSemi.invmap(-12.0), Scales::filterCutoffSemi,
      "fdnHighpassCutoffSemi", Info::kCanAutomate);
    value[ID::fdnHighpassCutoffSlope] = std::make_unique<LinearValue>(
      Scales::filterCutoffSlope.invmap(0.0), Scales::filterCutoffSlope,
      "fdnHighpassCutoffSlope", Info::kCanAutomate);
    value[ID::fdnHighpassQ] = std::make_unique<LinearValue>(
      1.0, Scales::filterQ, "fdnHighpassQ", Info::kCanAutomate);
    value[ID::fdnHighpassQSlope] = std::make_unique<LinearValue>(
      Scales::filterQSlope.invmap(0.0), Scales::filterQSlope, "fdnHighpassQSlope",
      Info::kCanAutomate);
    value[ID::fdnHighpassKeyFollow] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "fdnHighpassKeyFollow", Info::kCanAutomate);

    std::string modEnvelopeWavetableLabel("modEnvelopeWavetable");
    for (size_t idx = 0; idx < nModEnvelopeWavetable; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::modEnvelopeWavetable0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(0.0), Scales::defaultScale,
        (modEnvelopeWavetableLabel + indexStr).c_str(), Info::kCanAutomate);
    }
    value[ID::modEnvelopeInterpolation] = std::make_unique<UIntValue>(
      2, Scales::wavetableInterpolation, "modEnvelopeInterpolation", Info::kCanAutomate);
    value[ID::modEnvelopeTime] = std::make_unique<DecibelValue>(
      Scales::modEnvelopeTime.invmap(1.0), Scales::modEnvelopeTime, "modEnvelopeTime",
      Info::kCanAutomate);

    value[ID::modEnvelopeToFdnLowpassCutoff] = std::make_unique<LinearValue>(
      Scales::lfoToPitchAmount.invmap(0.0), Scales::lfoToPitchAmount,
      "modEnvelopeToFdnLowpassCutoff", Info::kCanAutomate);
    value[ID::modEnvelopeToFdnHighpassCutoff] = std::make_unique<LinearValue>(
      Scales::lfoToPitchAmount.invmap(0.0), Scales::lfoToPitchAmount,
      "modEnvelopeToFdnHighpassCutoff", Info::kCanAutomate);
    value[ID::modEnvelopeToFdnPitch] = std::make_unique<LinearValue>(
      Scales::lfoToPitchAmount.invmap(0.0), Scales::lfoToPitchAmount,
      "modEnvelopeToFdnPitch", Info::kCanAutomate);
    value[ID::modEnvelopeToFdnOvertoneAdd] = std::make_unique<DecibelValue>(
      0.0, Scales::fdnOvertoneAdd, "modEnvelopeToFdnOvertoneAdd", Info::kCanAutomate);
    value[ID::modEnvelopeToOscJitter] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale, "modEnvelopeToOscJitter",
      Info::kCanAutomate);
    value[ID::modEnvelopeToOscNoisePulseRatio] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0.0), Scales::bipolarScale,
      "modEnvelopeToOscNoisePulseRatio", Info::kCanAutomate);

    value[ID::tremoloMix] = std::make_unique<LinearValue>(
      0.0, Scales::defaultScale, "tremoloMix", Info::kCanAutomate);
    value[ID::tremoloDepth] = std::make_unique<LinearValue>(
      Scales::tremoloDepth.invmap(2.0), Scales::tremoloDepth, "tremoloDepth",
      Info::kCanAutomate);
    value[ID::tremoloDelayTime] = std::make_unique<DecibelValue>(
      Scales::tremoloDelayTime.invmapDB(-60.0), Scales::tremoloDelayTime,
      "tremoloDelayTime", Info::kCanAutomate);
    value[ID::tremoloModulationToDelayTimeOffset] = std::make_unique<LinearValue>(
      0.1, Scales::defaultScale, "tremoloModulationToDelayTimeOffset",
      Info::kCanAutomate);
    value[ID::tremoloModulationSmoothingHz] = std::make_unique<DecibelValue>(
      Scales::tremoloModulationSmoothingHz.invmap(10.0),
      Scales::tremoloModulationSmoothingHz, "tremoloModulationSmoothingHz",
      Info::kCanAutomate);

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
