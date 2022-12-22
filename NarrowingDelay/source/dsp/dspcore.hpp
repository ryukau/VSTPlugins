// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/lfo.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "filter.hpp"

using namespace SomeDSP;
using namespace Steinberg::Synth;

class DSPCore {
public:
  DSPCore() {}

  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;
  double timeSigUpper = 1.0;
  double timeSigLower = 4.0;

  void setup(double sampleRate);
  void reset();
  void startup();
  size_t getLatency();
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);

private:
  double getTempoSyncInterval();

  static constexpr size_t upFold = 2;

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  RotarySmoother<double> lfoPhaseConstant;
  ExpSmoother<double> lfoPhaseOffset;
  ExpSmoother<double> lfoShapeClip;
  ExpSmoother<double> lfoShapeSkew;
  ExpSmoother<double> outputGain;
  ExpSmoother<double> dryGain;
  ExpSmoother<double> wetGain;
  ExpSmoother<double> feedback;
  ExpSmoother<double> delayTimeSamples;
  ExpSmoother<double> shiftPitch;
  ExpSmoother<double> shiftFreq;
  ExpSmoother<double> lfoToPrimaryDelayTime;
  ExpSmoother<double> lfoToPrimaryShiftPitch;
  ExpSmoother<double> lfoToPrimaryShiftHz;

  LinearTempoSynchronizer<double, 32768> synchronizer;

  std::array<double, 2> previousInput{};
  std::array<double, 2> feedbackBuffer{};
  std::array<double, 2> secondaryBuffer{};
  std::array<std::array<double, 2>, 2> frame{};
  std::array<SVF<double>, 2> feedbackHighpass;
  std::array<SVF<double>, 2> feedbackLowpass;
  std::array<AMFrequencyShifter<double>, 2> frequencyShifter;
  std::array<PitchShiftDelay<double>, 2> pitchShifter;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
