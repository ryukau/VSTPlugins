// (c) 2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace SomeDSP {

/**
Lowpass filter specialized for 16x oversampling.

```python
import numpy
from scipy import signal
sos = signal.ellip(16, 0.01, 20500, "low", output="sos", fs=48000 * 16)
```
*/
template<typename Sample> class DecimationLowpass16 {
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

  void push(Sample input)
  {
    x0[0] = input;
    x0[1] = y0[0];
    x0[2] = y0[1];
    x0[3] = y0[2];
    x0[4] = y0[3];
    x0[5] = y0[4];
    x0[6] = y0[5];
    x0[7] = y0[6];

    y0[0] = co[0][0] * x0[0] + co[0][1] * x1[0] + co[0][2] * x2[0] - co[0][3] * y1[0]
      - co[0][4] * y2[0];
    y0[1] = co[1][0] * x0[1] + co[1][1] * x1[1] + co[1][2] * x2[1] - co[1][3] * y1[1]
      - co[1][4] * y2[1];
    y0[2] = co[2][0] * x0[2] + co[2][1] * x1[2] + co[2][2] * x2[2] - co[2][3] * y1[2]
      - co[2][4] * y2[2];
    y0[3] = co[3][0] * x0[3] + co[3][1] * x1[3] + co[3][2] * x2[3] - co[3][3] * y1[3]
      - co[3][4] * y2[3];
    y0[4] = co[4][0] * x0[4] + co[4][1] * x1[4] + co[4][2] * x2[4] - co[4][3] * y1[4]
      - co[4][4] * y2[4];
    y0[5] = co[5][0] * x0[5] + co[5][1] * x1[5] + co[5][2] * x2[5] - co[5][3] * y1[5]
      - co[5][4] * y2[5];
    y0[6] = co[6][0] * x0[6] + co[6][1] * x1[6] + co[6][2] * x2[6] - co[6][3] * y1[6]
      - co[6][4] * y2[6];
    y0[7] = co[7][0] * x0[7] + co[7][1] * x1[7] + co[7][2] * x2[7] - co[7][3] * y1[7]
      - co[7][4] * y2[7];

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;
  }

  inline Sample output() { return y0[7]; }

  std::array<Sample, 8> x0{};
  std::array<Sample, 8> x1{};
  std::array<Sample, 8> x2{};
  std::array<Sample, 8> y0{};
  std::array<Sample, 8> y1{};
  std::array<Sample, 8> y2{};
  constexpr static std::array<std::array<Sample, 5>, 8> co{{
    {Sample(1.325527960537483e-06), Sample(-1.0486296880714946e-06),
     Sample(1.3255279605374831e-06), Sample(-1.8869513625870566),
     Sample(0.8907702524266276)},
    {Sample(1.0), Sample(-1.7990799255799657), Sample(0.9999999999999999),
     Sample(-1.8984243919196817), Sample(0.90603953110456)},
    {Sample(1.0), Sample(-1.911565142173381), Sample(1.0), Sample(-1.9156790452684018),
     Sample(0.9289810487694654)},
    {Sample(1.0), Sample(-1.943108432116689), Sample(1.0), Sample(-1.9325800033745015),
     Sample(0.9513949185576018)},
    {Sample(1.0), Sample(-1.9556189334610117), Sample(1.0000000000000002),
     Sample(-1.9460569896516668), Sample(0.9691513322359767)},
    {Sample(1.0), Sample(-1.9614032628799634), Sample(1.0), Sample(-1.955820788205649),
     Sample(0.9818014494533305)},
    {Sample(1.0), Sample(-1.9641798599952223), Sample(1.0), Sample(-1.9628792094377048),
     Sample(0.9905808098567532)},
    {Sample(1.0), Sample(-1.965322766624071), Sample(0.9999999999999999),
     Sample(-1.968577366856729), Sample(0.9971005403967146)},
  }};
};

} // namespace SomeDSP
