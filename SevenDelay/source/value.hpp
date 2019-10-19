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

#include <type_traits>

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstparameters.h"

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
  virtual double getRaw() const = 0;
  virtual double getNormalized() = 0;
  virtual double getDefaultNormalized() = 0;
  virtual void setFromRaw(double value) = 0;
  virtual void setFromNormalized(double value) = 0;

  virtual tresult setState(IBStreamer &streamer) = 0;
  virtual tresult getState(IBStreamer &streamer) = 0;
  virtual tresult addParameter(Vst::ParameterContainer &parameters) = 0;
  virtual Vst::ParamID getId() = 0;
};

template<typename Scale, typename ValueType>
struct InternalValue : public ValueInterface {
  double defaultNormalized;
  double raw;
  Scale &scale;

  Vst::ParamID id;
  const char *name;
  const char *unit = nullptr;
  int32 parameterFlags;

  InternalValue(
    double defaultNormalized,
    Scale &scale,
    Vst::ParamID id,
    const char *name,
    int32 parameterFlags)
    : defaultNormalized(defaultNormalized)
    , raw(scale.map(defaultNormalized))
    , scale(scale)
    , id(id)
    , name(name)
    , parameterFlags(parameterFlags)
  {
  }

  inline double getRaw() const override { return raw; }
  double getNormalized() override { return scale.invmap(raw); }
  inline double getDefaultNormalized() override { return defaultNormalized; }

  void setFromRaw(double value) override
  {
    value = value < scale.getMin() ? scale.getMin()
                                   : value > scale.getMax() ? scale.getMax() : value;
    raw = value;
  }

  void setFromNormalized(double value) override
  {
    value = value < 0.0 ? 0.0 : value > 1.0 ? 1.0 : value;
    raw = scale.map(value);
  }

  tresult setState(IBStreamer &streamer)
  {
    if constexpr (std::is_same<double, ValueType>::value) {
      double value;
      if (!streamer.readDouble(value)) return kResultFalse;
      setFromNormalized(value);
    } else if constexpr (std::is_same<bool, ValueType>::value) {
      bool value;
      if (!streamer.readBool(value)) return kResultFalse;
      setFromNormalized(value);
    }
    return kResultOk;
  }

  tresult getState(IBStreamer &streamer)
  {
    if constexpr (std::is_same<double, ValueType>::value) {
      if (!streamer.writeDouble(getNormalized())) return kResultFalse;
    } else if constexpr (std::is_same<bool, ValueType>::value) {
      if (!streamer.writeBool(getNormalized())) return kResultFalse;
    }
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters)
  {
    if constexpr (std::is_same<double, ValueType>::value) {
      auto par = parameters.addParameter(new Vst::ScaledParameter<Scale>(
        USTRING(name), id, scale, defaultNormalized, USTRING(unit), parameterFlags));
      if (par == nullptr) return kResultFalse;
    } else if constexpr (std::is_same<bool, ValueType>::value) {
      auto par = parameters.addParameter(
        USTRING(name), USTRING(unit), 1, defaultNormalized, parameterFlags, id);
      if (par == nullptr) return kResultFalse;
    }
    return kResultOk;
  }

  Vst::ParamID getId() { return id; }
};

} // namespace Steinberg
