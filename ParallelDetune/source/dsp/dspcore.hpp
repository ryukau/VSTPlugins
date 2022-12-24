// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelDetune.
//
// ParallelDetune is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelDetune is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelDetune.  If not, see <https://www.gnu.org/licenses/>.

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
  std::array<double, 2> processFrame(double in0, double in1);
  double getTempoSyncInterval();

  static constexpr size_t upFold = 2;

  double sampleRate = 44100;
  double upRate = upFold * 44100;

  std::array<std::array<double, nShifter>, 2> spreadGain{};

  RotarySmoother<double> lfoPhaseConstant;
  ExpSmoother<double> lfoPhaseOffset;
  ExpSmoother<double> outputGain;
  ExpSmoother<double> dryGain;
  ExpSmoother<double> wetGain;
  ExpSmoother<double> panSpread;
  ExpSmoother<double> lfoToPan;
  ExpSmoother<double> tremoloMix;
  ExpSmoother<double> tremoloLean;
  ExpSmoother<double> feed;
  ExpSmoother<double> lfoToDelayTime;
  ExpSmoother<double> lfoToShiftPitch;

  ParallelExpSmoother<double, nShifter> shiftPitch;
  ParallelExpSmoother<double, nShifter> delayTimeSamples;
  ParallelExpSmoother<double, nShifter> shifterGain;

  ParallelExpSmoother<double, nShifter> highpassG;
  ParallelExpSmoother<double, nShifter> lowpassG;

  LinearTempoSynchronizer<double, 32768> synchronizer;
  TableLFO<double, nLfoWavetable, 2048, 2> lfo;

  std::array<std::array<double, nShifter>, 2> feedbackBuffer{};
  std::array<std::array<double, nShifter>, 2> mixBuffer{};
  std::array<CubicUpSampler<double, upFold>, 2> upSampler;
  std::array<ParallelSVF<double, nShifter>, 2> feedbackHighpass;
  std::array<std::array<ParallelSVF<double, nShifter>, 2>, 2> feedbackLowpass;
  std::array<ParallelPitchShiftDelay<double, nShifter>, 2> pitchShifter;
  std::array<HalfBandIIR<double, HalfBandCoefficient<double>>, 2> halfbandIir;
};
