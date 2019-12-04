#pragma once

#include <vector>

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
public:
  void setup(double sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1, 0.0);

    setTime(time);
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

protected:
  double sampleRate = 44100.0;
  Sample rFraction = 0.0;
  Sample w1 = 0.0;
  size_t wptr = 0;
  size_t rptr = 0;
  std::vector<Sample> buf{2};
};

} // namespace SomeDSP
