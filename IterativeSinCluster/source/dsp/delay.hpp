// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <algorithm>
#include <vector>

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
private:
  Sample rFraction = 0.0;
  Sample w1 = 0.0;
  int wptr = 0;
  int rptr = 0;
  std::vector<Sample> buf;

public:
  Sample sampleRate = 44100.0;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1, 0.0);

    setTime(time);
    reset();
  }

  void setTime(Sample seconds)
  {
    auto timeInSample = std::clamp<Sample>(sampleRate * seconds, 0, buf.size());

    int timeInt = int(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    rptr = wptr - timeInt;
    if (rptr < 0) rptr += int(buf.size());
  }

  Sample getTime()
  {
    auto diff = rptr < wptr ? wptr - rptr : wptr + buf.size() - rptr;
    return diff / sampleRate;
  }

  void reset()
  {
    w1 = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  Sample process(const Sample input)
  {
    const int size = int(buf.size());

    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
    if (++wptr >= size) wptr -= size;

    buf[wptr] = input;
    if (++wptr >= size) wptr -= size;

    w1 = input;

    // Read from buffer.
    const int i1 = rptr;
    if (++rptr >= size) rptr -= size;

    const int i0 = rptr;
    if (++rptr >= size) rptr -= size;

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

template<typename Sample> class Chorus {
public:
  Delay<Sample> delay;
  Sample phase = 0;
  Sample feedbackBuffer = 0;
  LinearSmoother<Sample> interpTick;
  RotarySmoother<Sample> interpPhase;
  LinearSmoother<Sample> interpFeedback;
  LinearSmoother<Sample> interpDepth;
  LinearSmoother<Sample> interpDelayTimeRange;
  LinearSmoother<Sample> interpMinDelayTime;
  EMAFilter<Sample> delayTimeLowpass;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    delay.setup(sampleRate, time, maxTime);
    delayTimeLowpass.setP(Sample(0.1)); // Fixed P.
    interpPhase.setRange(Sample(twopi));
  }

  // frequency can be negative.
  void setParam(
    Sample frequency,
    Sample phase,
    Sample feedback,
    Sample depth,
    Sample delayTimeRange,
    Sample minDelayTime)
  {
    interpTick.push(Sample(twopi) * frequency / delay.sampleRate);
    interpPhase.push(phase);
    interpFeedback.push(feedback);
    interpDepth.push(depth);
    interpDelayTimeRange.push(delayTimeRange);
    interpMinDelayTime.push(minDelayTime);
  }

  void reset(
    Sample frequency,
    Sample phase,
    Sample feedback,
    Sample depth,
    Sample delayTimeRange,
    Sample minDelayTime)
  {
    delay.reset();
    this->phase = 0;
    feedbackBuffer = 0;

    interpTick.reset(Sample(twopi) * frequency / delay.sampleRate);
    interpPhase.reset(phase);
    interpFeedback.reset(feedback);
    interpDepth.reset(depth);
    interpDelayTimeRange.reset(delayTimeRange);
    interpMinDelayTime.reset(minDelayTime);

    delayTimeLowpass.reset();
  }

  std::array<Sample, 2> process(Sample input)
  {
    phase += interpTick.process();
    if (phase >= Sample(twopi))
      phase -= Sample(twopi);
    else if (phase <= 0)
      phase += Sample(twopi);

    const auto phaseDelta = interpPhase.process();

    const Sample lfo = Sample(0.5) * (Sample(1) + std::sin(phase + phaseDelta));
    delay.setTime(delayTimeLowpass.process(
      interpMinDelayTime.process() + lfo * interpDelayTimeRange.process()));
    feedbackBuffer = delay.process(input + interpFeedback.process() * feedbackBuffer);

    const Sample lfoDepth
      = Sample(0.5) * (Sample(1) + std::sin(phase + phaseDelta + Sample(pi / 2)));
    const auto depth = interpDepth.process();
    return {
      feedbackBuffer * (Sample(1) - depth * lfoDepth),
      feedbackBuffer * (Sample(1) - depth * (Sample(1) - lfoDepth))};
  }
};

} // namespace SomeDSP
