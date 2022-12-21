// (c) 2022 Takamitsu Endo
//
// This file is part of LongPhaser.
//
// LongPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LongPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LongPhaser.  If not, see <https://www.gnu.org/licenses/>.

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
  ExpSmoother<double> outputGain;
  ExpSmoother<double> mix;
  ExpSmoother<double> outerFeed;
  ExpSmoother<double> delayTimeSpread;
  ExpSmoother<double> delayTimeCenterSamples;
  ExpSmoother<double> delayTimeRateLimit;
  ExpSmoother<double> delayTimeModAmount;
  ExpSmoother<double> innerFeed;
  ExpSmoother<double> lfoToInnerFeed;

  size_t currentAllpassStage = 0;
  size_t previousAllpassStage = 0;
  size_t transitionSamples = 2048;
  size_t transitionCounter = 0;

  LinearTempoSynchronizer<double, 32768> synchronizer;
  TableLFO<double, nLfoWavetable, 2048, 2> lfo;

  std::array<double, 2> previousInput{};
  std::array<double, 2> feedbackBuffer{};
  std::array<std::array<double, 2>, 2> frame{};
  double phase = 0;
  std::array<std::array<LongAllpass<double>, maxAllpass>, 2> allpass;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
