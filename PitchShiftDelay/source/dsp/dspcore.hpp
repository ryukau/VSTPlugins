// (c) 2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "lfo.hpp"
#include "pitchshiftdelay.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

using OverSampler = OverSampler16<float>;

class DSPCore {
public:
  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

private:
  float getTempoSyncInterval();

  float sampleRate = 44100.0f;
  float phaseSyncCutoffKp = 1e-5f;

  ExpSmoother<float> interpPitchMain;
  ExpSmoother<float> interpPitchUnison;
  ExpSmoother<float> interpLfoStereoOffset;
  ExpSmoother<float> interpLfoUnisonOffset;
  ExpSmoother<float> interpLfoToPitch;
  ExpSmoother<float> interpLfoToUnison;
  ExpSmoother<float> interpDelayTime;
  ExpSmoother<float> interpStereoLean;
  ExpSmoother<float> interpFeedback;
  ExpSmoother<float> interpHighpassCutoffKp;
  ExpSmoother<float> interpPitchCross;
  ExpSmoother<float> interpStereoCross;
  ExpSmoother<float> interpUnisonMix;
  ExpSmoother<float> interpDry;
  ExpSmoother<float> interpWet;

  LightTempoSynchronizer<float> synchronizer;
  TableLFO<float, nLfoWavetable, 2048, 4> lfo;
  std::array<float, 2> shifterMainOut{};
  std::array<float, 2> shifterUnisonOut{};
  std::array<OverSampler, 2> overSampler;
  std::array<PitchShiftDelay<float>, 2> shifterMain;
  std::array<PitchShiftDelay<float>, 2> shifterUnison;
};
