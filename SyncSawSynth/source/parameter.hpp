// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstparameters.h"

#include "dsp/scale.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

enum ParameterID : Vst::ParamID {
  bypass,
  osc1Gain,
  osc1Semi,
  osc1Cent,
  osc1Sync,
  osc1SyncType,
  osc1PTROrder,
  osc1Phase,
  osc1PhaseLock,

  osc2Gain,
  osc2Semi,
  osc2Cent,
  osc2Sync,
  osc2SyncType,
  osc2PTROrder,
  osc2Invert,
  osc2Phase,
  osc2PhaseLock,

  fmOsc1ToSync1,
  fmOsc1ToFreq2,
  fmOsc2ToSync1,

  gain,
  gainA,
  gainD,
  gainS,
  gainR,
  gainEnvelopeCurve,

  filterCutoff,
  filterResonance,
  filterFeedback,
  filterSaturation,
  filterDirty,
  filterType,
  filterShaper,
  filterA,
  filterD,
  filterS,
  filterR,
  filterCutoffAmount,
  filterResonanceAmount,
  filterKeyToCutoff,
  filterKeyToFeedback,

  modEnvelopeA,
  modEnvelopeCurve,
  modEnvelopeToFreq1,
  modEnvelopeToSync1,
  modEnvelopeToFreq2,
  modEnvelopeToSync2,
  modLFOFrequency,
  modLFONoiseMix,
  modLFOToFreq1,
  modLFOToSync1,
  modLFOToFreq2,
  modLFOToSync2,

  pitchBend,
};

struct GlobalParameter {
  bool bypass = false;

  Vst::ParamValue osc1Gain = 1.0;
  Vst::ParamValue osc1Semi = 0.5;
  Vst::ParamValue osc1Cent = 0.5;
  Vst::ParamValue osc1Sync = 1.0 / 16.0;
  int32 osc1SyncType = 0;
  int32 osc1PTROrder = 7;
  Vst::ParamValue osc1Phase = 0.0;
  bool osc1PhaseLock = false;

  Vst::ParamValue osc2Gain = 1.0;
  Vst::ParamValue osc2Semi = 0.5;
  Vst::ParamValue osc2Cent = 0.5;
  Vst::ParamValue osc2Sync = 1.0 / 16.0;
  int32 osc2SyncType = 0;
  int32 osc2PTROrder = 7;
  Vst::ParamValue osc2Phase = 0.0;
  bool osc2PhaseLock = false;
  bool osc2Invert = false;

  Vst::ParamValue fmOsc1ToSync1 = 0.0;
  Vst::ParamValue fmOsc1ToFreq2 = 0.0;
  Vst::ParamValue fmOsc2ToSync1 = 0.0;

  Vst::ParamValue gain = 0.5;
  Vst::ParamValue gainA = 0.05;
  Vst::ParamValue gainD = 0.5;
  Vst::ParamValue gainS = 0.5;
  Vst::ParamValue gainR = 0.1;
  Vst::ParamValue gainEnvelopeCurve = 0.0;

  Vst::ParamValue filterCutoff = 1.0;
  Vst::ParamValue filterResonance = 0.5;
  Vst::ParamValue filterFeedback = 0.0;
  Vst::ParamValue filterSaturation = 0.3;
  bool filterDirty = false;
  int32 filterType = 0;
  int32 filterShaper = 0;
  Vst::ParamValue filterA = 0.0;
  Vst::ParamValue filterD = 0.5;
  Vst::ParamValue filterS = 0.0;
  Vst::ParamValue filterR = 0.5;
  Vst::ParamValue filterCutoffAmount = 0.5;
  Vst::ParamValue filterResonanceAmount = 0.0;
  Vst::ParamValue filterKeyToCutoff = 0.5;
  Vst::ParamValue filterKeyToFeedback = 0.5;

  Vst::ParamValue modEnvelopeA = 0.0;
  Vst::ParamValue modEnvelopeCurve = 0.5;
  Vst::ParamValue modEnvelopeToFreq1 = 0.0;
  Vst::ParamValue modEnvelopeToSync1 = 0.0;
  Vst::ParamValue modEnvelopeToFreq2 = 0.0;
  Vst::ParamValue modEnvelopeToSync2 = 0.0;
  Vst::ParamValue modLFOFrequency = 0.5;
  Vst::ParamValue modLFONoiseMix = 0.01;
  Vst::ParamValue modLFOToFreq1 = 0.0;
  Vst::ParamValue modLFOToSync1 = 0.0;
  Vst::ParamValue modLFOToFreq2 = 0.0;
  Vst::ParamValue modLFOToSync2 = 0.0;

  Vst::ParamValue pitchBend = 0.5;

  static LogScale<Vst::ParamValue> scaleOscGain;
  static LinearScale<Vst::ParamValue> scaleSemi;
  static LinearScale<Vst::ParamValue> scaleCent;
  static LinearScale<Vst::ParamValue> scaleSync;

  static LogScale<Vst::ParamValue> scaleFMToSync;
  static LogScale<Vst::ParamValue> scaleFMToFreq;

  static LogScale<Vst::ParamValue> scaleGain;

  static LogScale<Vst::ParamValue> scaleEnvelopeA;
  static LogScale<Vst::ParamValue> scaleEnvelopeD;
  static LogScale<Vst::ParamValue> scaleEnvelopeS;
  static LogScale<Vst::ParamValue> scaleEnvelopeR;

  static LogScale<Vst::ParamValue> scaleFilterCutoff;
  static LogScale<Vst::ParamValue> scaleFilterResonance;
  static LogScale<Vst::ParamValue> scaleFilterFeedback;
  static LogScale<Vst::ParamValue> scaleFilterSaturation;
  static LinearScale<Vst::ParamValue> scaleFilterCutoffAmount;
  static LinearScale<Vst::ParamValue> scaleFilterKeyMod;

  static LogScale<Vst::ParamValue> scaleModEnvelopeA;
  static LogScale<Vst::ParamValue> scaleModEnvelopeCurve;
  static LogScale<Vst::ParamValue> scaleModLFOFrequency;
  static LogScale<Vst::ParamValue> scaleModToFreq;
  static LogScale<Vst::ParamValue> scaleModToSync;

  tresult setState(IBStream *stream)
  {
    IBStreamer s(stream, kLittleEndian);

    if (!s.readBool(bypass)) return kResultFalse;

    if (!s.readDouble(osc1Gain)) return kResultFalse;
    if (!s.readDouble(osc1Semi)) return kResultFalse;
    if (!s.readDouble(osc1Cent)) return kResultFalse;
    if (!s.readDouble(osc1Sync)) return kResultFalse;
    if (!s.readInt32(osc1SyncType)) return kResultFalse;
    if (!s.readInt32(osc1PTROrder)) return kResultFalse;
    if (!s.readDouble(osc1Phase)) return kResultFalse;
    if (!s.readBool(osc1PhaseLock)) return kResultFalse;

    if (!s.readDouble(osc2Gain)) return kResultFalse;
    if (!s.readDouble(osc2Semi)) return kResultFalse;
    if (!s.readDouble(osc2Cent)) return kResultFalse;
    if (!s.readDouble(osc2Sync)) return kResultFalse;
    if (!s.readInt32(osc2SyncType)) return kResultFalse;
    if (!s.readInt32(osc2PTROrder)) return kResultFalse;
    if (!s.readDouble(osc2Phase)) return kResultFalse;
    if (!s.readBool(osc2PhaseLock)) return kResultFalse;
    if (!s.readBool(osc2Invert)) return kResultFalse;

    if (!s.readDouble(fmOsc1ToSync1)) return kResultFalse;
    if (!s.readDouble(fmOsc1ToFreq2)) return kResultFalse;
    if (!s.readDouble(fmOsc2ToSync1)) return kResultFalse;

    if (!s.readDouble(gain)) return kResultFalse;
    if (!s.readDouble(gainA)) return kResultFalse;
    if (!s.readDouble(gainD)) return kResultFalse;
    if (!s.readDouble(gainS)) return kResultFalse;
    if (!s.readDouble(gainR)) return kResultFalse;
    if (!s.readDouble(gainEnvelopeCurve)) return kResultFalse;

    if (!s.readDouble(filterCutoff)) return kResultFalse;
    if (!s.readDouble(filterResonance)) return kResultFalse;
    if (!s.readDouble(filterFeedback)) return kResultFalse;
    if (!s.readDouble(filterSaturation)) return kResultFalse;
    if (!s.readBool(filterDirty)) return kResultFalse;
    if (!s.readInt32(filterType)) return kResultFalse;
    if (!s.readInt32(filterShaper)) return kResultFalse;
    if (!s.readDouble(filterA)) return kResultFalse;
    if (!s.readDouble(filterD)) return kResultFalse;
    if (!s.readDouble(filterS)) return kResultFalse;
    if (!s.readDouble(filterR)) return kResultFalse;
    if (!s.readDouble(filterCutoffAmount)) return kResultFalse;
    if (!s.readDouble(filterResonanceAmount)) return kResultFalse;
    if (!s.readDouble(filterKeyToCutoff)) return kResultFalse;
    if (!s.readDouble(filterKeyToFeedback)) return kResultFalse;

    if (!s.readDouble(modEnvelopeA)) return kResultFalse;
    if (!s.readDouble(modEnvelopeCurve)) return kResultFalse;
    if (!s.readDouble(modEnvelopeToFreq1)) return kResultFalse;
    if (!s.readDouble(modEnvelopeToSync1)) return kResultFalse;
    if (!s.readDouble(modEnvelopeToFreq2)) return kResultFalse;
    if (!s.readDouble(modEnvelopeToSync2)) return kResultFalse;
    if (!s.readDouble(modLFOFrequency)) return kResultFalse;
    if (!s.readDouble(modLFONoiseMix)) return kResultFalse;
    if (!s.readDouble(modLFOToFreq1)) return kResultFalse;
    if (!s.readDouble(modLFOToSync1)) return kResultFalse;
    if (!s.readDouble(modLFOToFreq2)) return kResultFalse;
    if (!s.readDouble(modLFOToSync2)) return kResultFalse;

    if (!s.readDouble(pitchBend)) return kResultFalse;

    // Add parameter here.

    return kResultOk;
  }

  tresult getState(IBStream *stream)
  {
    IBStreamer s(stream, kLittleEndian);

    if (!s.writeBool(bypass)) return kResultFalse;

    if (!s.writeDouble(osc1Gain)) return kResultFalse;
    if (!s.writeDouble(osc1Semi)) return kResultFalse;
    if (!s.writeDouble(osc1Cent)) return kResultFalse;
    if (!s.writeDouble(osc1Sync)) return kResultFalse;
    if (!s.writeInt32(osc1SyncType)) return kResultFalse;
    if (!s.writeInt32(osc1PTROrder)) return kResultFalse;
    if (!s.writeDouble(osc1Phase)) return kResultFalse;
    if (!s.writeBool(osc1PhaseLock)) return kResultFalse;

    if (!s.writeDouble(osc2Gain)) return kResultFalse;
    if (!s.writeDouble(osc2Semi)) return kResultFalse;
    if (!s.writeDouble(osc2Cent)) return kResultFalse;
    if (!s.writeDouble(osc2Sync)) return kResultFalse;
    if (!s.writeInt32(osc2SyncType)) return kResultFalse;
    if (!s.writeInt32(osc2PTROrder)) return kResultFalse;
    if (!s.writeDouble(osc2Phase)) return kResultFalse;
    if (!s.writeBool(osc2PhaseLock)) return kResultFalse;
    if (!s.writeBool(osc2Invert)) return kResultFalse;

    if (!s.writeDouble(fmOsc1ToSync1)) return kResultFalse;
    if (!s.writeDouble(fmOsc1ToFreq2)) return kResultFalse;
    if (!s.writeDouble(fmOsc2ToSync1)) return kResultFalse;

    if (!s.writeDouble(gain)) return kResultFalse;
    if (!s.writeDouble(gainA)) return kResultFalse;
    if (!s.writeDouble(gainD)) return kResultFalse;
    if (!s.writeDouble(gainS)) return kResultFalse;
    if (!s.writeDouble(gainR)) return kResultFalse;
    if (!s.writeDouble(gainEnvelopeCurve)) return kResultFalse;

    if (!s.writeDouble(filterCutoff)) return kResultFalse;
    if (!s.writeDouble(filterResonance)) return kResultFalse;
    if (!s.writeDouble(filterFeedback)) return kResultFalse;
    if (!s.writeDouble(filterSaturation)) return kResultFalse;
    if (!s.writeBool(filterDirty)) return kResultFalse;
    if (!s.writeInt32(filterType)) return kResultFalse;
    if (!s.writeInt32(filterShaper)) return kResultFalse;
    if (!s.writeDouble(filterA)) return kResultFalse;
    if (!s.writeDouble(filterD)) return kResultFalse;
    if (!s.writeDouble(filterS)) return kResultFalse;
    if (!s.writeDouble(filterR)) return kResultFalse;
    if (!s.writeDouble(filterCutoffAmount)) return kResultFalse;
    if (!s.writeDouble(filterResonanceAmount)) return kResultFalse;
    if (!s.writeDouble(filterKeyToCutoff)) return kResultFalse;
    if (!s.writeDouble(filterKeyToFeedback)) return kResultFalse;

    if (!s.writeDouble(modEnvelopeA)) return kResultFalse;
    if (!s.writeDouble(modEnvelopeCurve)) return kResultFalse;
    if (!s.writeDouble(modEnvelopeToFreq1)) return kResultFalse;
    if (!s.writeDouble(modEnvelopeToSync1)) return kResultFalse;
    if (!s.writeDouble(modEnvelopeToFreq2)) return kResultFalse;
    if (!s.writeDouble(modEnvelopeToSync2)) return kResultFalse;
    if (!s.writeDouble(modLFOFrequency)) return kResultFalse;
    if (!s.writeDouble(modLFONoiseMix)) return kResultFalse;
    if (!s.writeDouble(modLFOToFreq1)) return kResultFalse;
    if (!s.writeDouble(modLFOToSync1)) return kResultFalse;
    if (!s.writeDouble(modLFOToFreq2)) return kResultFalse;
    if (!s.writeDouble(modLFOToSync2)) return kResultFalse;

    if (!s.writeDouble(pitchBend)) return kResultFalse;

    // Add parameter here.

    return kResultOk;
  }
};

} // namespace Synth

namespace Vst {

template<typename ParameterScale> class ScaledParameter : public Parameter {
public:
  ScaledParameter(const TChar *title,
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
