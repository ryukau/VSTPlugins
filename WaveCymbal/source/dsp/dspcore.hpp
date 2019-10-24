// (c) 2019 Takamitsu Endo
//
// This file is part of WaveCymbal.
//
// WaveCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// WaveCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with WaveCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../parameter.hpp"
#include "constants.hpp"
#include "ksstring.hpp"
#include "smoother.hpp"

#include <array>
#include <cmath>
#include <memory>

using namespace SomeDSP;
using namespace Steinberg::Synth;

struct NoteInfo {
  int32_t id;
  float frequency;
  float velocity;
};

class DSPCore {
public:
  static const size_t maxVoice = 32;
  GlobalParameter param;

  void setup(double sampleRate);
  void free();    // Release memory.
  void reset();   // Stop sounds.
  void startup(); // Reset phase, random seed etc.
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);
  void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity);
  void noteOff(int32_t noteId);

private:
  void setSystem();

  float sampleRate = 44100.0f;

  // Top of this stack is current note.
  std::vector<NoteInfo> noteStack;

  Pulsar<float> pulsar{44100.0f, 0};
  VelvetNoise<float> velvetNoise{44100.0f, 100.0f, 0};
  Brown<float> brownNoise{0};

  Random<float> rnd{0};
  Excitor<float> excitor;
  std::unique_ptr<WaveHat<float>> cymbal;

  // debug
  bool trigger = false;

  LinearSmoother<float> interpMasterGain;
  LinearSmoother<float> interpPitch;
};
