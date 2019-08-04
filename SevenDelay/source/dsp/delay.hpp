// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <stdlib.h>
#include <vector>

template <typename Sample>
class FractionalDelayLinear {
public:
  void reset()
  {
    xd = 0;
    diff = 0;
  }

  void push(Sample input)
  {
    diff = input - xd;
    xd = input;
  }

  Sample at(Sample fraction)
  {
    return xd - fraction * diff;
  }

  Sample process(Sample input, Sample fraction)
  {
    push(input);
    return at(fraction);
  }

private:
  Sample xd = 0;
  Sample diff = 0;
};

template <typename Sample, unsigned char Order>
class FractionalDelayLagrange {
public:
  void reset()
  {
    xd.fill(0.0);
    diff.fill(0.0);
  }

  void push(Sample input)
  {
    diff[0] = input - xd[0];
    xd[0] = input;
    for (size_t i = 1; i < Order; ++i) {
      diff[i] = diff[i - 1] - xd[i];
      xd[i] = diff[i - 1];
    }
  }

  Sample at(Sample fraction)
  {
    Sample delta = fraction + (Order - 1) / 2;
    Sample sig = 0.0;

    size_t i = Order;
    while (i > 0) {
      size_t next = i - 1;
      sig = (next - delta) / i * (diff[next] + sig);
      i = next;
    }

    return sig + xd[0];
  }

  Sample process(Sample input, Sample fraction)
  {
    push(input);
    return at(fraction);
  }

private:
  std::array<Sample, Order> xd{};
  std::array<Sample, Order> diff{};
};

template <typename Sample, unsigned char Order>
class DelayLagrange {
public:
  DelayLagrange(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = overSample * sampleRate;

    buf.resize((size_t)(maxTime * this->sampleRate * overSample) + 1, 0.0);

    setTime(time);
  }

  void setTime(Sample value)
  {
    Sample timeInSample = std::max<Sample>(
      fix, std::min<Sample>((Sample)sampleRate * value, (Sample)buf.size()));

    int32_t time_int = (int32_t)timeInSample;
    rFraction = timeInSample - (Sample)time_int;

    rptr = wptr - time_int + fix;
    while (rptr < 0) rptr += (int32_t)buf.size();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), 0.0);
    wInterp.reset();
  }

  Sample process(Sample input)
  {
    wInterp.push(input);

    for (size_t i = 1; i <= overSample; ++i) {
      Sample wFraction = (overSample - (Sample)i) / overSample;

      buf[wptr] = wInterp.at(wFraction);
      wptr += 1;
      while ((size_t)wptr >= buf.size()) wptr -= (int32_t)buf.size();
    }

    rptr += overSample;
    while (rptr >= buf.size()) rptr -= (int32_t)buf.size();
    int32_t i0 = rptr - 1;
    while (i0 < 0) i0 += (int32_t)buf.size();
    int32_t i1 = rptr - 2;
    while (i1 < 0) i1 += (int32_t)buf.size();

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }

private:
  static const uint8_t overSample = Order;
  static const size_t fix = (overSample * (Order - 1)) / 2;
  Sample sampleRate = 44100.0;
  Sample rFraction = 0.0;
  std::vector<Sample> buf;
  int32_t wptr = 0;
  int32_t rptr = 0;
  FractionalDelayLagrange<Sample, Order> wInterp;
};

template <typename Sample>
class Delay {
public:
  // length = _sampleRate * 2 * _maxTime + 1;
  Delay(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;
    buf.resize((size_t)(maxTime * this->sampleRate) * 2 + 1, 0.0);

    setTime(time);

    w1 = 0.0;
  }

  void setTime(Sample value)
  {
    Sample timeInSample = std::max<Sample>(
      0, std::min<Sample>((Sample)sampleRate * value, (Sample)buf.size()));

    int32_t time_int = (int32_t)timeInSample;
    rFraction = timeInSample - (Sample)time_int;

    rptr = wptr - time_int;
    while (rptr < 0) rptr += (int32_t)buf.size();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), 0.0);
    w1 = 0.0;
  }

  Sample process(Sample input)
  {
    // Write to buffer.
    buf[wptr] = input - 0.5 * (input - w1);
    wptr += 1;
    if (wptr >= buf.size()) wptr -= (int32_t)buf.size();

    buf[wptr] = input;
    wptr += 1;
    if (wptr >= buf.size()) wptr -= (int32_t)buf.size();

    w1 = input;

    // Read from buffer.
    const int32_t i1 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= (int32_t)buf.size();

    const int32_t i0 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= (int32_t)buf.size();

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }

private:
  Sample sampleRate = 44100.0;
  Sample rFraction = 0.0;
  std::vector<Sample> buf;
  Sample w1 = 0.0;
  int32_t wptr = 0;
  int32_t rptr = 0;
};

template <typename Sample, unsigned char Order>
class DelayReference {
public:
  DelayReference(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = overSample * sampleRate;

    buf.resize((size_t)(maxTime * this->sampleRate * overSample) + 1, 0.0);

    setTime(time);
  }

  void setTime(Sample value)
  {
    Sample timeInSample = std::max<Sample>(
      fix, std::min<Sample>((Sample)sampleRate * value, (Sample)buf.size()));

    int32_t time_int = (int32_t)timeInSample;
    rFraction = timeInSample - (Sample)time_int;

    rptr = wptr - time_int + fix;
    while (rptr < 0) rptr += (int32_t)buf.size();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), 0.0);
    rInterp.reset();
    wInterp.reset();
  }

  Sample process(Sample input)
  {
    wInterp.push(input);

    for (size_t i = 1; i <= overSample; ++i) {
      Sample wFraction = (overSample - (Sample)i) / overSample;

      buf[wptr] = wInterp.at(wFraction);
      wptr += 1;
      while (wptr >= (int32_t)buf.size()) wptr -= (int32_t)buf.size();

      rInterp.push(buf[rptr]);
      rptr += 1;
      while (rptr >= (int32_t)buf.size()) rptr -= (int32_t)buf.size();
    }

    return rInterp.at(rFraction);
  }

private:
  static const uint8_t overSample = Order;
  static const size_t fix = (Order - 1 + overSample * (Order - 1)) / 2;
  double sampleRate = 44100.0;
  Sample rFraction = 0.0;
  std::vector<Sample> buf;
  int32_t wptr = 0;
  int32_t rptr = 0;
  FractionalDelayLagrange<Sample, Order> rInterp;
  FractionalDelayLagrange<Sample, Order> wInterp;
};
