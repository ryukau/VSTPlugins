// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../common/dsp/scale.hpp"

#include <string>

using int32 = long;

namespace Steinberg {

namespace Vst {
using ParamID = unsigned long;

struct ParameterInfo {
  enum ParameterFlags {
    kNoFlags = 0,
    kCanAutomate = 1 << 0,
    kIsReadOnly = 1 << 1,
    kIsWrapAround = 1 << 2,
    kIsList = 1 << 3,
    kIsHidden = 1 << 4,
    kIsProgramChange = 1 << 15,
    kIsBypass = 1 << 16
  };
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
  void setId(Vst::ParamID) {}
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
};

} // namespace Steinberg
