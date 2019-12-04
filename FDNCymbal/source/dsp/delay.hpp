#pragma once

#include <memory>
#include <numeric>
#include <vector>

#include "smoother.hpp"

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
public:
  void setup(double sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1);

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

protected:
  Sample sampleRate = 44100.0;
  Sample rFraction = 0.0;
  Sample w1 = 0.0;
  size_t wptr = 0;
  size_t rptr = 0;
  std::vector<Sample> buf{2};
};

template<typename Sample, size_t matrixSize> class FeedbackDelayNetwork {
public:
  Sample sampleRate;
  std::array<Delay<Sample>, matrixSize> delay;
  std::array<LinearSmoother<Sample>, matrixSize> delayTime;
  std::array<Sample, matrixSize> gain{};
  std::array<Sample, matrixSize> buffer{};
  std::array<Sample, matrixSize> delayOut{};
  std::array<std::array<Sample, matrixSize>, matrixSize> matrix;

  void setup(Sample sampleRate, Sample maxTime = 0.5)
  {
    this->sampleRate = sampleRate;
    for (auto &dly : delay) dly.setup(sampleRate, maxTime, maxTime);
    for (auto dlyTime : delayTime) dlyTime.reset(maxTime);
    reset();
  }

  void reset()
  {
    for (auto &dly : delay) dly.reset();

    gain.fill(1);
    buffer.fill(0);
    delayOut.fill(0);

    for (size_t i = 0; i < matrixSize; ++i) {
      for (size_t j = 0; j < matrixSize; ++j) {
        if (i == j)
          matrix[i][j] = 1;
        else
          matrix[i][j] = 0;
      }
    }
  }

  Sample process(Sample input)
  {
    buffer.fill(0);

    for (size_t i = 0; i < matrixSize; ++i) {
      for (size_t j = 0; j < matrixSize; ++j) buffer[i] += matrix[i][j] * delayOut[j];
    }

    for (size_t i = 0; i < matrixSize; ++i) {
      delay[i].setTime(delayTime[i].process());
      delayOut[i] = delay[i].process(gain[i] * (buffer[i] + input));
    }

    return std::accumulate(delayOut.begin(), delayOut.end(), Sample(0));
  }
};

// Schroeder allpass filter
// https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
//
// Transfer function H(z):
// H(z) = (gain + z^{-M}) / (1 + gain * z^{-M})
template<typename Sample> class LongAllpass {
public:
  Sample gain = 1;
  Sample buffer = 0;
  Delay<Sample> delay;
  LinearSmoother<Sample> delayTime;

  void setup(Sample sampleRate, Sample maxTime)
  {
    delay.setup(sampleRate, maxTime, maxTime);
    delayTime.reset(maxTime);
  }

  void set(Sample gain, Sample timeSec)
  {
    this->gain = gain;
    delayTime.push(timeSec);
  }

  void reset()
  {
    buffer = 0;
    delay.reset();
  }

  Sample process(Sample input)
  {
    delay.setTime(delayTime.process());

    input += gain * buffer;
    auto output = buffer - gain * input;
    buffer = delay.process(input);
    return output;
  }
};

template<typename Sample, size_t nAllpass> class SerialAllpass {
public:
  std::array<LongAllpass<Sample>, nAllpass> allpass;

  void setup(Sample sampleRate, Sample maxTime)
  {
    for (auto &ap : allpass) ap.setup(sampleRate, maxTime);
  }

  void reset()
  {
    for (auto &ap : allpass) ap.reset();
  }

  Sample process(Sample input)
  {
    for (auto &ap : allpass) input = ap.process(input);
    return input;
  }
};

} // namespace SomeDSP
