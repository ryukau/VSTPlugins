// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

constexpr size_t nReservedParameter = 256;
constexpr size_t nReservedGuiParameter = 64;

namespace SomeDSP {
enum class TransformType : int {
  fft,
  fwht,
  haar,
  reserved3,
  reserved4,
  reserved5,
  reserved6,
  reserved7,
  LENGTH
};
enum class MaskWaveform : int {
  cosine,
  square,
  sawtoothUp,
  sawtoothDown,
  noise,
  reserved5,
  reserved6,
  reserved7,
  LENGTH
};
enum class LfoWaveform : int {
  sine,
  triSaw,
  reserved2,
  reserved3,
  reserved4,
  reserved5,
  reserved6,
  reserved7,
  LENGTH
};
} // namespace SomeDSP

namespace Steinberg {
namespace Synth {

constexpr size_t maxFrameSizeLog2 = 16;
constexpr size_t maxFrameSizeStart = 2; // Start from 2^(maxFrameSizeStart).

namespace ParameterID {
enum ID {
  bypass,

  parameterSmoothingSecond,

  outputGain,
  dryWetMix,
  sideChainSwitch,
  reportLatency,

  transform,
  frameSize,
  feedback,

  maskWaveform,
  maskMix,
  maskPhase,
  maskFreq,
  maskThreshold,
  maskRotation,
  spectralShift,
  octaveDown,

  lfoWaveform,
  lfoWaveMod,
  lfoTempoUpper,
  lfoTempoLower,
  lfoRate,
  lfoStereoPhaseOffset,
  lfoInitialPhase,

  lfoToMaskMix,
  lfoToMaskPhase,
  lfoToMaskFreq,
  lfoToMaskThreshold,
  lfoToMaskRotation,
  lfoToSpectralShift,
  lfoToOctaveDown,

  reservedParameter0,
  reservedGuiParameter0 = reservedParameter0 + nReservedParameter,

  ID_ENUM_LENGTH = reservedGuiParameter0 + nReservedGuiParameter,
  // ID_ENUM_GUI_START = ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;
  static SomeDSP::LinearScale<double> bipolarScale;

  static SomeDSP::DecibelScale<double> parameterSmoothingSecond;

  static SomeDSP::DecibelScale<double> gain;

  static SomeDSP::UIntScale<double> transform;
  static SomeDSP::UIntScale<double> frameSize;
  static SomeDSP::DecibelScale<double> feedback;

  static SomeDSP::UIntScale<double> maskWaveform;
  static SomeDSP::DecibelScale<double> maskFreq;

  static SomeDSP::UIntScale<double> lfoWaveform;
  static SomeDSP::UIntScale<double> lfoTempoSync;
  static SomeDSP::DecibelScale<double> lfoRate;
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

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::parameterSmoothingSecond] = std::make_unique<DecibelValue>(
      Scales::parameterSmoothingSecond.invmap(0.1), Scales::parameterSmoothingSecond,
      "parameterSmoothingSecond", Info::kCanAutomate);

    value[ID::outputGain] = std::make_unique<DecibelValue>(
      Scales::gain.invmap(1.0), Scales::gain, "outputGain", Info::kCanAutomate);
    value[ID::dryWetMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0.75), Scales::defaultScale, "dryWetMix",
      Info::kCanAutomate);
    value[ID::sideChainSwitch] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "sideChainSwitch", Info::kCanAutomate);
    value[ID::reportLatency] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "reportLatency", Info::kCanAutomate);

    value[ID::transform] = std::make_unique<UIntValue>(
      0, Scales::transform, "transform", Info::kCanAutomate);
    value[ID::frameSize] = std::make_unique<UIntValue>(
      10, Scales::frameSize, "frameSize", Info::kCanAutomate);
    value[ID::feedback] = std::make_unique<DecibelValue>(
      Scales::feedback.invmap(0.5), Scales::feedback, "feedback", Info::kCanAutomate);

    value[ID::maskWaveform] = std::make_unique<UIntValue>(
      0, Scales::maskWaveform, "maskWaveform", Info::kCanAutomate);
    value[ID::maskMix] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(1), Scales::defaultScale, "maskMix",
      Info::kCanAutomate);
    value[ID::maskPhase] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "maskPhase",
      Info::kCanAutomate);
    value[ID::maskFreq] = std::make_unique<DecibelValue>(
      Scales::maskFreq.invmap(8), Scales::maskFreq, "maskFreq", Info::kCanAutomate);
    value[ID::maskThreshold] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "maskThreshold",
      Info::kCanAutomate);
    value[ID::maskRotation] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "maskRotation",
      Info::kCanAutomate);
    value[ID::spectralShift] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "spectralShift",
      Info::kCanAutomate);
    value[ID::octaveDown] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "octaveDown",
      Info::kCanAutomate);

    value[ID::lfoWaveform] = std::make_unique<UIntValue>(
      0, Scales::lfoWaveform, "lfoWaveform", Info::kCanAutomate);
    value[ID::lfoWaveMod] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoWaveMod",
      Info::kCanAutomate);
    value[ID::lfoTempoUpper] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoSync, "lfoTempoUpper", Info::kCanAutomate);
    value[ID::lfoTempoLower] = std::make_unique<UIntValue>(
      0, Scales::lfoTempoSync, "lfoTempoLower", Info::kCanAutomate);
    value[ID::lfoRate] = std::make_unique<DecibelValue>(
      Scales::lfoRate.invmap(1.0), Scales::lfoRate, "lfoRate", Info::kCanAutomate);
    value[ID::lfoStereoPhaseOffset] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "lfoStereoPhaseOffset",
      Info::kCanAutomate);
    value[ID::lfoInitialPhase] = std::make_unique<LinearValue>(
      Scales::defaultScale.invmap(0), Scales::defaultScale, "lfoInitialPhase",
      Info::kCanAutomate);

    value[ID::lfoToMaskMix] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToMaskMix",
      Info::kCanAutomate);
    value[ID::lfoToMaskPhase] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToMaskPhase",
      Info::kCanAutomate);
    value[ID::lfoToMaskFreq] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToMaskFreq",
      Info::kCanAutomate);
    value[ID::lfoToMaskThreshold] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToMaskThreshold",
      Info::kCanAutomate);
    value[ID::lfoToMaskRotation] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToMaskRotation",
      Info::kCanAutomate);
    value[ID::lfoToSpectralShift] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToSpectralShift",
      Info::kCanAutomate);
    value[ID::lfoToOctaveDown] = std::make_unique<LinearValue>(
      Scales::bipolarScale.invmap(0), Scales::bipolarScale, "lfoToOctaveDown",
      Info::kCanAutomate);

    for (size_t idx = 0; idx < nReservedParameter; ++idx) {
      auto indexStr = std::to_string(idx);
      value[ID::reservedParameter0 + idx] = std::make_unique<LinearValue>(
        Scales::defaultScale.invmap(0.0), Scales::defaultScale,
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
