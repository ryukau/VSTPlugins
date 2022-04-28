// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstparameters.h"

#include "dsp/scale.hpp"

#include <string>

namespace Steinberg {
namespace Vst {

template<typename ParameterScale> class ScaledParameter : public Parameter {
public:
  ScaledParameter(
    const TChar *title,
    ParamID tag,
    ParameterScale &scale,
    ParamValue defaultValue = 0.0,
    const TChar *units = nullptr,
    int32 flags = ParameterInfo::kCanAutomate,
    UnitID unitID = kRootUnitId)
    : Parameter(title, tag, units, defaultValue, 0, flags, unitID), scale(scale)
  {
    precision = 16;
  }

  virtual void toString(ParamValue normalized, String128 string) const SMTG_OVERRIDE
  {
    UString128 wrapper;
    wrapper.printFloat(toPlain(normalized), precision);
    wrapper.copyTo(string, 128);
  }

  virtual bool fromString(const TChar *string, ParamValue &normalized) const SMTG_OVERRIDE
  {
    UString wrapper((TChar *)string, strlen16(string));
    if (wrapper.scanFloat(normalized)) {
      normalized = toNormalized(normalized);
      return true;
    }
    return false;
  }

  virtual ParamValue toPlain(ParamValue normalized) const SMTG_OVERRIDE
  {
    return scale.map(normalized);
  }

  virtual ParamValue toNormalized(ParamValue plain) const SMTG_OVERRIDE
  {
    return scale.invmap(plain);
  }

  OBJ_METHODS(ScaledParameter, Parameter)

protected:
  ParameterScale &scale;
  ParamValue multiplier;
};

} // namespace Vst

struct ValueInterface {
  virtual ~ValueInterface() {}

  virtual uint32_t getInt() const = 0;
  virtual float getFloat() const = 0;
  virtual double getDouble() const = 0;
  virtual double getNormalized() = 0;
  virtual double getDefaultNormalized() = 0;
  virtual void setFromInt(uint32_t value) = 0;
  virtual void setFromFloat(double value) = 0;
  virtual void setFromNormalized(double value) = 0;

  virtual tresult setState(IBStreamer &streamer) = 0;
  virtual tresult getState(IBStreamer &streamer) = 0;
  virtual tresult addParameter(Vst::ParameterContainer &parameters) = 0;
  virtual Vst::ParamID getId() = 0;
  virtual void setId(Vst::ParamID id) = 0;
};

struct UIntValue : public ValueInterface {
  SomeDSP::UIntScale<double> &scale;
  double defaultNormalized;
  uint32_t raw;

  std::string name;
  std::string unit;
  int32 parameterFlags;
  Vst::ParamID id;

  UIntValue(
    uint32_t defaultRaw,
    SomeDSP::UIntScale<double> &scale,
    std::string name,
    int32 parameterFlags)
    : scale(scale)
    , defaultNormalized(scale.invmap(defaultRaw))
    , raw(defaultRaw <= scale.getMax() ? defaultRaw : 0)
    , name(name)
    , parameterFlags(parameterFlags)
  {
  }

  inline uint32_t getInt() const override { return raw; }
  inline float getFloat() const override { return float(raw); }
  inline double getDouble() const override { return double(raw); }
  double getNormalized() override { return scale.invmap(raw); }
  inline double getDefaultNormalized() override { return defaultNormalized; }

  void setFromInt(uint32_t value) override
  {
    raw = std::clamp<uint32_t>(value, scale.getMin(), scale.getMax());
  }

  void setFromFloat(double valueFloat) override
  {
    raw = std::clamp<uint32_t>(uint32_t(valueFloat), scale.getMin(), scale.getMax());
  }

  void setFromNormalized(double value) override
  {
    raw = scale.map(std::clamp<double>(value, 0.0, 1.0));
  }

  tresult setState(IBStreamer &streamer) override
  {
    uint32 value;
    if (!streamer.readInt32u(value)) return kResultFalse;
    setFromInt(value);
    return kResultOk;
  }

  tresult getState(IBStreamer &streamer) override
  {
    if (!streamer.writeInt32u(raw)) return kResultFalse;
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters) override
  {
    auto par = parameters.addParameter(new Vst::ScaledParameter(
      USTRING(name.c_str()), id, scale, defaultNormalized, USTRING(unit.c_str()),
      parameterFlags));
    return par == nullptr ? kResultFalse : kResultOk;
  }

  Vst::ParamID getId() override { return id; }
  void setId(Vst::ParamID id) override { this->id = id; }
};

template<typename Scale> struct DoubleValue : public ValueInterface {
  double defaultNormalized;
  double raw;
  Scale &scale;

  std::string name;
  std::string unit;
  int32 parameterFlags;
  Vst::ParamID id;

  DoubleValue(
    double defaultNormalized, Scale &scale, std::string name, int32 parameterFlags)
    : defaultNormalized(defaultNormalized)
    , raw(scale.map(defaultNormalized))
    , scale(scale)
    , name(name)
    , parameterFlags(parameterFlags)
  {
  }

  inline uint32_t getInt() const override { return uint32_t(raw); }
  inline float getFloat() const override { return float(raw); }
  inline double getDouble() const override { return double(raw); }
  double getNormalized() override { return scale.invmap(raw); }
  inline double getDefaultNormalized() override { return defaultNormalized; }

  void setFromInt(uint32_t value) override
  {
    raw = std::clamp<double>(value, scale.getMin(), scale.getMax());
  }

  void setFromFloat(double value) override
  {
    raw = std::clamp<double>(value, scale.getMin(), scale.getMax());
  }

  void setFromNormalized(double value) override
  {
    raw = scale.map(std::clamp<double>(value, 0.0, 1.0));
  }

  tresult setState(IBStreamer &streamer) override
  {
    double normalized;
    if (!streamer.readDouble(normalized)) return kResultFalse;
    setFromNormalized(normalized);
    return kResultOk;
  }

  tresult getState(IBStreamer &streamer) override
  {
    if (!streamer.writeDouble(getNormalized())) return kResultFalse;
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters) override
  {
    auto par = parameters.addParameter(new Vst::ScaledParameter<Scale>(
      USTRING(name.c_str()), id, scale, defaultNormalized, USTRING(unit.c_str()),
      parameterFlags));
    return par == nullptr ? kResultFalse : kResultOk;
  }

  Vst::ParamID getId() override { return id; }
  void setId(Vst::ParamID id) override { this->id = id; }
};

} // namespace Steinberg
