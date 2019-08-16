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
#include "pluginterfaces/vst/vsttypes.h"

#include "dsp/scale.hpp"

namespace Steinberg {
namespace SevenDelay {

constexpr Vst::ParamValue maxDelayTime = 8.0;
constexpr Vst::ParamValue maxToneFrequency = 20000.0;
constexpr Vst::ParamValue minDCKillFrequency = 1.0;

enum ParameterID : Vst::ParamID {
  bypass,
  time,
  feedback,
  offset,
  wetMix,
  dryMix,
  tempoSync,
  negativeFeedback,
  lfoAmount,
  lfoFrequency,
  lfoShape,
  lfoInitialPhase,
  lfoHold,
  smoothness,
  inSpread,
  inPan,
  outSpread,
  outPan,
  tone,
  dckill,
};

struct GlobalParameter {
  bool bypass = false;
  Vst::ParamValue time = 0.5;
  Vst::ParamValue feedback = 0.625;
  Vst::ParamValue offset = 0.5;
  Vst::ParamValue wetMix = 0.75;
  Vst::ParamValue dryMix = 1.0;
  bool tempoSync = false;
  bool negativeFeedback = false;
  Vst::ParamValue lfoAmount = 0.0;
  Vst::ParamValue lfoFrequency = 0.5;
  Vst::ParamValue lfoShape = 0.5;
  Vst::ParamValue lfoInitialPhase = 0.0;
  bool lfoHold = false;
  Vst::ParamValue smoothness = 0.3;
  Vst::ParamValue inSpread = 0.0; // 0.0: normal, 0.5: mono, 1.0: inverse.
  Vst::ParamValue inPan = 0.5;    // 0.0: left, 0.5: center, 1.0 right.
  Vst::ParamValue outSpread = 0.0;
  Vst::ParamValue outPan = 0.5;
  Vst::ParamValue tone = 1.0;
  Vst::ParamValue dckill = 0.0;

  static LogScale<Vst::ParamValue> scaleTime;
  static SPolyScale<Vst::ParamValue> scaleOffset;
  static LogScale<Vst::ParamValue> scaleLfoAmount;
  static LogScale<Vst::ParamValue> scaleLfoFrequency;
  static LogScale<Vst::ParamValue> scaleLfoShape;
  static LinearScale<Vst::ParamValue> scaleLfoInitialPhase;
  static LogScale<Vst::ParamValue> scaleSmoothness;
  static LogScale<Vst::ParamValue> scaleTone;
  static LogScale<Vst::ParamValue> scaleToneMix; // internal
  static LogScale<Vst::ParamValue> scaleDCKill;
  static LogScale<Vst::ParamValue> scaleDCKillMix; // internal

  tresult setState(IBStream *stream)
  {
    IBStreamer s(stream, kLittleEndian);

    if (!s.readBool(bypass)) return kResultFalse;
    if (!s.readDouble(time)) return kResultFalse;
    if (!s.readDouble(feedback)) return kResultFalse;
    if (!s.readDouble(offset)) return kResultFalse;
    if (!s.readDouble(wetMix)) return kResultFalse;
    if (!s.readDouble(dryMix)) return kResultFalse;
    if (!s.readBool(tempoSync)) return kResultFalse;
    if (!s.readBool(negativeFeedback)) return kResultFalse;
    if (!s.readDouble(lfoAmount)) return kResultFalse;
    if (!s.readDouble(lfoFrequency)) return kResultFalse;
    if (!s.readDouble(lfoShape)) return kResultFalse;
    if (!s.readDouble(lfoInitialPhase)) return kResultFalse;
    if (!s.readBool(lfoHold)) return kResultFalse;
    if (!s.readDouble(smoothness)) return kResultFalse;
    if (!s.readDouble(inSpread)) return kResultFalse;
    if (!s.readDouble(inPan)) return kResultFalse;
    if (!s.readDouble(outSpread)) return kResultFalse;
    if (!s.readDouble(outPan)) return kResultFalse;
    if (!s.readDouble(tone)) return kResultFalse;
    if (!s.readDouble(dckill)) return kResultFalse;

    // Add parameter here.

    return kResultOk;
  }

  tresult getState(IBStream *stream)
  {
    IBStreamer s(stream, kLittleEndian);

    if (!s.writeBool(bypass)) return kResultFalse;
    if (!s.writeDouble(time)) return kResultFalse;
    if (!s.writeDouble(feedback)) return kResultFalse;
    if (!s.writeDouble(offset)) return kResultFalse;
    if (!s.writeDouble(wetMix)) return kResultFalse;
    if (!s.writeDouble(dryMix)) return kResultFalse;
    if (!s.writeBool(tempoSync)) return kResultFalse;
    if (!s.writeBool(negativeFeedback)) return kResultFalse;
    if (!s.writeDouble(lfoAmount)) return kResultFalse;
    if (!s.writeDouble(lfoFrequency)) return kResultFalse;
    if (!s.writeDouble(lfoShape)) return kResultFalse;
    if (!s.writeDouble(lfoInitialPhase)) return kResultFalse;
    if (!s.writeBool(lfoHold)) return kResultFalse;
    if (!s.writeDouble(smoothness)) return kResultFalse;
    if (!s.writeDouble(inSpread)) return kResultFalse;
    if (!s.writeDouble(inPan)) return kResultFalse;
    if (!s.writeDouble(outSpread)) return kResultFalse;
    if (!s.writeDouble(outPan)) return kResultFalse;
    if (!s.writeDouble(tone)) return kResultFalse;
    if (!s.writeDouble(dckill)) return kResultFalse;

    // Add parameter here.

    return kResultOk;
  }
};

} // namespace SevenDelay

namespace Vst {

template <typename ParameterScale>
class ScaledParameter : public Parameter {
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
} // namespace Steinberg
