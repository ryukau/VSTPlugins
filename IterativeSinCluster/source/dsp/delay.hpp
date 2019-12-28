#pragma once

#include <vector>

#include "constants.hpp"
#include "smoother.hpp"

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
public:
  Sample sampleRate = 44100.0;
  Sample rFraction = 0.0;
  Sample w1 = 0.0;
  size_t wptr = 0;
  size_t rptr = 0;
  std::vector<Sample> buf;

  void setup(double sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1, 0.0);

    setTime(time);
    reset();
  }

  void setTime(Sample seconds)
  {
    auto timeInSample
      = std::max<Sample>(0, std::min<Sample>(sampleRate * seconds, (Sample)buf.size()));

    size_t timeInt = size_t(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    // Possible negative overflow. rptr must be in [0, buf.size()).
    rptr = wptr - timeInt;
    while (rptr >= buf.size()) rptr += buf.size();
  }

  Sample getTime()
  {
    auto diff = rptr < wptr ? wptr - rptr : wptr + buf.size() - rptr;
    return diff / sampleRate;
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), 0);
    w1 = 0.0;
  }

  Sample process(const Sample input)
  {
    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
    wptr += 1;
    if (wptr >= buf.size()) wptr -= buf.size();

    buf[wptr] = input;
    wptr += 1;
    if (wptr >= buf.size()) wptr -= buf.size();

    w1 = input;

    // Read from buffer.
    const size_t i1 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= buf.size();

    const size_t i0 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= buf.size();

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
  PController<Sample> delayTimeLowpass;

  void setup(double sampleRate, Sample time, Sample maxTime)
  {
    delay.setup(sampleRate, time, maxTime);
    delayTimeLowpass.setup(sampleRate, 0.1);
    interpPhase.setRange(twopi);
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

  void reset()
  {
    delay.reset();
    phase = 0;
    feedbackBuffer = 0;
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

    const Sample lfo = Sample(0.5) * (Sample(1) + somesin<Sample>(phase + phaseDelta));
    delay.setTime(delayTimeLowpass.process(
      interpMinDelayTime.process() + lfo * interpDelayTimeRange.process()));
    feedbackBuffer = delay.process(input + interpFeedback.process() * feedbackBuffer);

    const Sample lfoDepth
      = Sample(0.5) * (Sample(1) + somesin<Sample>(phase + phaseDelta + Sample(pi / 2)));
    const auto depth = interpDepth.process();
    return {feedbackBuffer * (Sample(1) - depth * lfoDepth),
            feedbackBuffer * (Sample(1) - depth * (Sample(1) - lfoDepth))};
  }
};

} // namespace SomeDSP
