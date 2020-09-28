// (c) 2020 Takamitsu Endo
//
// This file is part of ModuloShaper.
//
// ModuloShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ModuloShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ModuloShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "../../../common/dsp/decimationLowpass.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> inline Sample getImmediateAmplitude(std::array<Sample, 2> input)
{
  return std::max(std::fabs(input[0]), std::fabs(input[1]));
}

/*
Integer delay with fixed size buffer. Specialized for use in limiter.
Max delay time is around 21 ms when sampling rate is 192kHz.
*/
template<typename Sample> class Delay {
public:
  constexpr static int bufEnd = 4095; // 2^12 - 1.

  std::array<std::array<Sample, 2>, bufEnd + 1> buf{};
  int wptr = 0;
  int rptr = 0;

  void reset() { buf.fill({0, 0}); }

  void setFrames(uint32_t delayFrames)
  {
    if (delayFrames >= buf.size()) delayFrames = bufEnd;
    rptr = wptr - int(delayFrames);
    if (rptr < 0) rptr += int(buf.size());
  }

  Sample findPeak()
  {
    Sample peak = getImmediateAmplitude(buf[rptr]);
    int index = rptr;
    while (index != wptr) {
      index = (index + 1) & bufEnd;
      peak = std::max(peak, getImmediateAmplitude(buf[index]));
    }
    return peak;
  }

  std::array<Sample, 2> process(std::array<Sample, 2> input)
  {
    // Write to buffer.
    wptr = (wptr + 1) & bufEnd;
    buf[wptr] = input;

    // Read from buffer.
    rptr = (rptr + 1) & bufEnd;
    return buf[rptr];
  }
};

/*
Non-blickwall stereo limiter using 1-pole lowpass filter.
Overshoot is around 3% of input amplitude.

Note that latency is introduced.
 */
template<typename Sample> struct EasyLimiter {
  Sample threshold = Sample(0.1);
  Sample targetGain = 1;
  Sample peak = 1;
  Sample releaseCoefficient = Sample(1.0001);
  uint32_t attackFrames = 0;
  int32_t counter = -1;

  EMAFilter<Sample> smoother;
  Delay<Sample> delay;

  uint32_t latency() { return attackFrames; }

  void prepare(
    Sample sampleRate, Sample attackSeconds, Sample releaseSeconds, Sample threshold)
  {
    smoother.setCutoff(sampleRate, Sample(1) / attackSeconds);
    attackFrames = uint32_t(sampleRate * attackSeconds);
    delay.setFrames(attackFrames);

    releaseCoefficient
      = std::pow(Sample(1.0 / 1e-5), Sample(1.0) / (releaseSeconds * sampleRate));

    this->threshold = threshold;
  }

  void reset()
  {
    targetGain = Sample(1);
    peak = threshold;
    counter = -1;
    smoother.reset(Sample(1));
    delay.reset();
  }

  inline void setPeak(Sample newPeak)
  {
    peak = newPeak;
    counter = attackFrames;
    targetGain = threshold / peak;
  }

  std::array<Sample, 2> process(std::array<Sample, 2> input)
  {
    auto inAbs = getImmediateAmplitude(input);

    if (inAbs > peak) setPeak(inAbs); // Line (1). Referred in later comment.

    if (counter < 0) {
      /*
      When attack is finished, limiter searches a new peak in buffer. If the peak in
      buffer is below threshold, limiter is finally start releasing. This is required
      in case of the signal is constantly decaying, and peak amplitude still exceeds
      the threshold.

      In naive implementation, the new peak is set at line (1) at next process() call.
      However, the peak from line (1) may be smaller than the peak still in buffer. If
      this happens, the output badly exceeds threshold.
      */
      auto peakInBuffer = delay.findPeak();
      if (peakInBuffer > threshold) {
        setPeak(peakInBuffer);
      } else {
        peak = threshold;
        targetGain *= releaseCoefficient;
        if (targetGain > Sample(1)) {
          targetGain = Sample(1);
        }
      }
    } else {
      --counter;
    }

    auto gain = smoother.process(targetGain);
    auto out = delay.process(input);
    return {gain * out[0], gain * out[1]};
  }
};

} // namespace SomeDSP
