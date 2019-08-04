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

template <typename Sample, unsigned char Order>
class FractionalDelayLagrange {
public:
  FractionalDelayLagrange(){};

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

template <typename Sample, typename FractionalDelay, unsigned char Order>
class DelayReference {
public:
  ~DelayReference()
  {
    if (buf != nullptr) free(buf);
  }

  void initialize(const double &_sampleRate, const Sample &_time)
  {
    sampleRate = overSample * _sampleRate;
    length = _sampleRate * overSample * 8; // Max 8 seconds.

    if (buf != nullptr) free(buf);
    buf = (Sample *)calloc(length + 2, sizeof(Sample));

    wptr = 0;
    setTime(_time);
  }

  void setTime(const Sample &value)
  {
    Sample timeInSample = std::max<Sample>(
      fix, std::min<Sample>((Sample)sampleRate * value, (Sample)length));

    int32_t time_int = (int32_t)timeInSample;
    rFraction = timeInSample - (Sample)time_int;

    rptr = wptr - time_int + fix;
    while (rptr < 0) rptr += length;
  }

  void reset()
  {
    memset(buf, 0.0, length * sizeof(Sample));
    rInterp.reset();
    wInterp.reset();
  }

  Sample process(const Sample &input)
  {
    if (buf == nullptr) return 0.0;

    wInterp.push(input);

    for (size_t i = 1; i <= overSample; ++i) {
      Sample wFraction = (overSample - (Sample)i) / overSample;

      buf[wptr] = wInterp.at(wFraction);
      wptr += 1;
      while (wptr >= length) wptr -= length;

      rInterp.push(buf[rptr]);
      rptr += 1;
      while (rptr >= length) rptr -= length;
    }

    return rInterp.at(rFraction);
  }

private:
  static const uint8_t overSample = Order;
  static const size_t fix = (Order - 1 + overSample * (Order - 1)) / 2;
  double sampleRate = 44100.0;
  Sample rFraction = 0.0;
  int32_t length = 0;
  Sample *buf = nullptr;
  int32_t wptr = 0;
  int32_t rptr = 0;
  FractionalDelay rInterp;
  FractionalDelay wInterp;
};
