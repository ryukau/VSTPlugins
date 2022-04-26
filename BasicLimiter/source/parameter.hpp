// (c) 2021 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr float maxAttackSeconds = 1.0f;

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  limiterThreshold,
  limiterGate,
  limiterAttack,
  limiterRelease,
  limiterSustain,
  limiterStereoLink,

  truePeak,

  ID_ENUM_LENGTH,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::DecibelScale<double> limiterThreshold;
  static SomeDSP::DecibelScale<double> limiterGate;
  static SomeDSP::LogScale<double> limiterAttack;
  static SomeDSP::LogScale<double> limiterRelease;
  static SomeDSP::LogScale<double> limiterSustain;
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

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::limiterThreshold] = std::make_unique<DecibelValue>(
      Scales::limiterThreshold.invmap(1.0), Scales::limiterThreshold, "limiterThreshold",
      Info::kCanAutomate);
    value[ID::limiterGate] = std::make_unique<DecibelValue>(
      0.0, Scales::limiterGate, "limiterGate", Info::kCanAutomate);
    value[ID::limiterAttack] = std::make_unique<LogValue>(
      Scales::limiterAttack.invmap(0.001333333333333333), Scales::limiterAttack,
      "limiterAttack", Info::kCanAutomate); // Default 1.3 ms = 64 sample / 48000 Hz.
    value[ID::limiterRelease] = std::make_unique<LogValue>(
      Scales::limiterRelease.invmap(0.0), Scales::limiterRelease, "limiterRelease",
      Info::kCanAutomate);
    value[ID::limiterSustain] = std::make_unique<LogValue>(
      Scales::limiterSustain.invmap(0.0016666666666666666), Scales::limiterSustain,
      "limiterSustain", Info::kCanAutomate);
    value[ID::limiterStereoLink] = std::make_unique<LinearValue>(
      1.0, Scales::defaultScale, "limiterStereoLink", Info::kCanAutomate);

    value[ID::truePeak]
      = std::make_unique<UIntValue>(0, Scales::boolScale, "truePeak", Info::kCanAutomate);

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
