// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <array>

namespace SomeDSP {

/**
Highpass filter to reduce direct current (DC).

```python
import numpy
from scipy import signal
sos = signal.ellip(4, 0.01, 10, "highpass", output="sos", fs=48000)
```
*/
template<typename Sample> class DCKiller {
public:
  void reset()
  {
    x0.fill(0);
    x1.fill(0);
    x2.fill(0);
    y0.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  Sample process(Sample input)
  {
    x0[0] = input;
    x0[1] = y0[0];

    y0[0] = co[0][0] * x0[0] + co[0][1] * x1[0] + co[0][2] * x2[0] - co[0][3] * y1[0]
      - co[0][4] * y2[0];
    y0[1] = co[1][0] * x0[1] + co[1][1] * x1[1] + co[1][2] * x2[1] - co[1][3] * y1[1]
      - co[1][4] * y2[1];

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0[1];
  }

  std::array<Sample, 2> x0{};
  std::array<Sample, 2> x1{};
  std::array<Sample, 2> x2{};
  std::array<Sample, 2> y0{};
  std::array<Sample, 2> y1{};
  std::array<Sample, 2> y2{};
  constexpr static std::array<std::array<Sample, 5>, 2> co{{
    {Sample(0.9975855039491213), Sample(-1.9951710074517641), Sample(0.997585503949121),
     Sample(-1.9980038700285063), Sample(0.9980051856573467)},
    {Sample(1.0), Sample(-1.99999999739308), Sample(1.0), Sample(-1.999463865157682),
     Sample(0.9994647194968778)},
  }};
};

} // namespace SomeDSP
