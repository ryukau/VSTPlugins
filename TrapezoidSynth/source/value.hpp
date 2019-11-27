// (c) 2019 Takamitsu Endo
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

#pragma once

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstparameters.h"

#include "dsp/scale.hpp"

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

  virtual double getFloat() const = 0;
  virtual uint32_t getInt() const = 0;
  virtual double getNormalized() = 0;
  virtual double getDefaultNormalized() = 0;
  virtual void setFromInt(uint32_t value) = 0;
  virtual void setFromFloat(double value) = 0;
  virtual void setFromNormalized(double value) = 0;

  virtual tresult setState(IBStreamer &streamer) = 0;
  virtual tresult getState(IBStreamer &streamer) = 0;
  virtual tresult addParameter(Vst::ParameterContainer &parameters) = 0;
  virtual Vst::ParamID getId() = 0;
};

struct IntValue : public ValueInterface {
  SomeDSP::IntScale<double> &scale;
  double defaultNormalized;
  uint32_t raw;

  const char *name;
  const char *unit = nullptr;
  int32 parameterFlags;
  Vst::ParamID id;

  IntValue(
    uint32_t defaultRaw,
    SomeDSP::IntScale<double> &scale,
    const char *name,
    int32 parameterFlags,
    Vst::ParamID id)
    : scale(scale)
    , defaultNormalized(scale.invmap(defaultRaw))
    , raw(defaultRaw <= scale.getMax() ? defaultRaw : 0)
    , name(name)
    , parameterFlags(parameterFlags)
    , id(id)
  {
  }

  inline uint32_t getInt() const override { return raw; }
  inline double getFloat() const override { return raw; }
  double getNormalized() override { return scale.invmap(raw); }
  inline double getDefaultNormalized() override { return defaultNormalized; }

  void setFromInt(uint32_t value) override
  {
    raw = value < scale.getMin() ? scale.getMin()
                                 : value > scale.getMax() ? scale.getMax() : value;
  }

  void setFromFloat(double valueFloat) override
  {
    uint32_t value = uint32_t(valueFloat);
    raw = value < scale.getMin() ? scale.getMin()
                                 : value > scale.getMax() ? scale.getMax() : value;
  }

  void setFromNormalized(double value) override
  {
    raw = scale.map(value < 0.0 ? 0.0 : value > 1.0 ? 1.0 : value);
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
    auto par = parameters.addParameter(
      USTRING(name), USTRING(unit), scale.getMax(), defaultNormalized, parameterFlags,
      id);
    return par == nullptr ? kResultFalse : kResultOk;
  }

  Vst::ParamID getId() override { return id; }
};

template<typename Scale> struct FloatValue : public ValueInterface {
  double defaultNormalized;
  double raw;
  Scale &scale;

  const char *name;
  const char *unit = nullptr;
  int32 parameterFlags;
  Vst::ParamID id;

  FloatValue(
    double defaultNormalized,
    Scale &scale,
    const char *name,
    int32 parameterFlags,
    Vst::ParamID id)
    : defaultNormalized(defaultNormalized)
    , raw(scale.map(defaultNormalized))
    , scale(scale)
    , name(name)
    , parameterFlags(parameterFlags)
    , id(id)
  {
  }

  inline uint32_t getInt() const override { return uint32_t(raw); }
  inline double getFloat() const override { return raw; }
  double getNormalized() override { return scale.invmap(raw); }
  inline double getDefaultNormalized() override { return defaultNormalized; }

  void setFromInt(uint32_t value) override
  {
    raw = value < scale.getMin() ? scale.getMin()
                                 : value > scale.getMax() ? scale.getMax() : value;
  }

  void setFromFloat(double value) override
  {
    raw = value < scale.getMin() ? scale.getMin()
                                 : value > scale.getMax() ? scale.getMax() : value;
  }

  void setFromNormalized(double value) override
  {
    raw = scale.map(value < 0.0 ? 0.0 : value > 1.0 ? 1.0 : value);
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
      USTRING(name), id, scale, defaultNormalized, USTRING(unit), parameterFlags));
    if (par == nullptr) return kResultFalse;
    return par == nullptr ? kResultFalse : kResultOk;
  }

  Vst::ParamID getId() override { return id; }
};

} // namespace Steinberg
