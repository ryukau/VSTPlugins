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

#include <algorithm>
#include <cmath>

namespace SomeDSP {

template<typename Sample> inline Sample safeClip(Sample input)
{
  return std::isfinite(input) ? std::clamp<Sample>(input, Sample(-128), Sample(128)) : 0;
}

template<typename Sample> class Butter8Lowpass {
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

  void setCutoff(Sample sampleRate, Sample cutoffHz)
  {
    Sample oc1 = 4 * Sample(twopi) * cutoffHz / sampleRate; // oc for ω_c.
    Sample oc2 = oc1 * oc1;
    Sample mid_oc = Sample(2.8284271247461903) * oc1; // 2^(3/2) = 2.8284271247461903

    co[0] = oc2;                                    // b0
    co[1] = Sample(2) * oc2;                        // b1
    co[2] = oc2;                                    // b2
    co[3] = Sample(2) * oc2 - Sample(8);            // a1
    co[4] = oc2 - mid_oc + Sample(4);               // a2
    co[5] = Sample(1) / (oc2 + mid_oc + Sample(4)); // a0
  }

  Sample process(Sample input)
  {
    x0[0] = input;
    x0[1] = y0[0];
    x0[2] = y0[1];
    x0[3] = y0[2];

    y0[0] = co[5]
      * (co[0] * x0[0] + co[1] * x1[0] + co[2] * x2[0] - co[3] * y1[0] - co[4] * y2[0]);
    y0[1] = co[5]
      * (co[0] * x0[1] + co[1] * x1[1] + co[2] * x2[1] - co[3] * y1[1] - co[4] * y2[1]);
    y0[2] = co[5]
      * (co[0] * x0[2] + co[1] * x1[2] + co[2] * x2[2] - co[3] * y1[2] - co[4] * y2[2]);
    y0[3] = co[5]
      * (co[0] * x0[3] + co[1] * x1[3] + co[2] * x2[3] - co[3] * y1[3] - co[4] * y2[3]);

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0[3];
  }

  std::array<Sample, 4> x0{};
  std::array<Sample, 4> x1{};
  std::array<Sample, 4> x2{};
  std::array<Sample, 4> y0{};
  std::array<Sample, 4> y1{};
  std::array<Sample, 4> y2{};
  std::array<Sample, 6> co{};
};

template<typename Sample> struct ModuloShaper {
  Sample gain = 1;
  Sample add = 1;
  Sample mul = 1;
  bool hardclip = true;

  Sample x1 = 0;
  DecimationLowpass16<Sample> lowpass;

  void reset()
  {
    x1 = 0;
    lowpass.reset();
  }

  Sample process(Sample x0)
  {
    if (hardclip) x0 = std::clamp(x0, Sample(-1), Sample(1));
    Sample sign = std::copysign(Sample(1), x0);
    x0 = std::fabs(x0 * gain);
    Sample floored = std::floor(x0);
    Sample height = std::pow(add, floored);
    Sample out
      = sign * ((x0 - floored) * std::pow(mul, floored) * height + Sample(1) - height);
    return safeClip(out);
  }

  float process16x(Sample x0)
  {
    if (hardclip) x0 = std::clamp(x0, Sample(-1), Sample(1));
    Sample diff = x0 - x1;
    for (size_t i = 0; i < 16; ++i) lowpass.push(process(x1 + i / Sample(16) * diff));
    x1 = x0;
    if (std::isfinite(lowpass.output())) return lowpass.output();

    reset();
    return 0;
  }
};

template<typename Sample> class ModuloShaperPolyBLEP {
private:
  Sample x0 = 0;
  Sample x1 = 0;
  Sample x2 = 0;
  Sample x3 = 0;
  Sample x4 = 0;
  Sample x5 = 0;
  Sample x6 = 0;
  Sample x7 = 0;

  Sample f0 = 0;
  Sample f1 = 0;
  Sample f2 = 0;
  Sample f3 = 0;

  Sample lastInt = 0;

public:
  Sample gain = 1;
  Sample add = 1;
  Sample mul = 1;
  bool hardclip = true;

  void reset()
  {
    x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
    f0 = f1 = f2 = f3 = 0;
    lastInt = 0;
  }

  Sample process4(Sample input)
  {
    if (hardclip) input = std::clamp(input, Sample(-1), Sample(1));
    Sample sign = std::copysign(Sample(1), input);
    input = std::fabs(input * gain);
    Sample floored = std::floor(input);
    Sample height = std::pow(add, floored);
    input
      = sign * ((input - floored) * std::pow(mul, floored) * height + Sample(1) - height);

    if (f1 != 0) {
      Sample x2_abs = std::fabs(x2);
      Sample t = std::clamp<Sample>(
        (Sample(1) - x2_abs) / (std::fabs(x1) + Sample(1) - x2_abs), Sample(0),
        Sample(1));

      Sample polyBlepResidual4_0 = -t * t * t * t / Sample(24) + t * t * t / Sample(6)
        - t * t / Sample(4) + t / Sample(6) - Sample(1) / Sample(24);
      Sample polyBlepResidual4_1 = t * t * t * t / Sample(8) - t * t * t / Sample(3)
        + Sample(2) * t / Sample(3) - Sample(1) / Sample(2);
      Sample polyBlepResidual4_2 = -t * t * t * t / Sample(8) + t * t * t / Sample(6)
        + t * t / Sample(4) + t / Sample(6) + Sample(1) / Sample(24);
      Sample polyBlepResidual4_3 = t * t * t * t / Sample(24);

      x0 += f1 * polyBlepResidual4_0;
      x1 += f1 * polyBlepResidual4_1;
      x2 += f1 * polyBlepResidual4_2;
      x3 += f1 * polyBlepResidual4_3;
    }

    f1 = f0;

    if (floored != lastInt) {
      if (floored < lastInt) height *= Sample(0.5);
      f0 = height * sign * std::copysign(Sample(1), lastInt - floored);
      lastInt = floored;
    } else {
      f0 = 0;
    }

    x3 = x2;
    x2 = x1;
    x1 = x0;
    x0 = input;

    return safeClip(x3);
  }

  Sample process8(Sample input)
  {
    if (hardclip) input = std::clamp(input, Sample(-1), Sample(1));
    Sample sign = std::copysign(Sample(1), input);
    input = std::fabs(input * gain);
    Sample floored = std::floor(input);
    Sample height = std::pow(add, floored);
    input
      = sign * ((input - floored) * std::pow(mul, floored) * height + Sample(1) - height);

    if (f3 != 0) {
      Sample x4_abs = std::fabs(x4);
      Sample t = std::clamp<Sample>(
        (Sample(1) - x4_abs) / (std::fabs(x3) + Sample(1) - x4_abs), Sample(0),
        Sample(1));

      Sample polyBlepResidual8_0 = -t * t * t * t * t * t * t * t / Sample(40320)
        + t * t * t * t * t * t * t / Sample(5040) - t * t * t * t * t * t / Sample(1440)
        + t * t * t * t * t / Sample(720) - t * t * t * t / Sample(576)
        + t * t * t / Sample(720) - t * t / Sample(1440) + t / Sample(5040)
        - Sample(1) / Sample(40320);
      Sample polyBlepResidual8_1 = t * t * t * t * t * t * t * t / Sample(5760)
        - t * t * t * t * t * t * t / Sample(840) + t * t * t * t * t * t / Sample(360)
        - t * t * t * t / Sample(72) + t * t * t / Sample(30)
        - Sample(7) * t * t / Sample(180) + t / Sample(42) - Sample(31) / Sample(5040);
      Sample polyBlepResidual8_2 = -t * t * t * t * t * t * t * t / Sample(1920)
        + t * t * t * t * t * t * t / Sample(336) - t * t * t * t * t * t / Sample(288)
        - t * t * t * t * t / Sample(80) + t * t * t * t * float(19) / Sample(576)
        + t * t * t / Sample(48) - Sample(49) * t * t / Sample(288)
        + Sample(397) * t / Sample(1680) - Sample(4541) / Sample(40320);
      Sample polyBlepResidual8_3 = t * t * t * t * t * t * t * t / Sample(1152)
        - t * t * t * t * t * t * t / Sample(252) + t * t * t * t * t / Sample(45)
        - t * t * t / Sample(9) + Sample(151) * t / Sample(315) - Sample(1) / Sample(2);
      Sample polyBlepResidual8_4 = -t * t * t * t * t * t * t * t / Sample(1152)
        + t * t * t * t * t * t * t / Sample(336) + t * t * t * t * t * t / Sample(288)
        - t * t * t * t * t / Sample(80) - t * t * t * t * float(19) / Sample(576)
        + t * t * t / Sample(48) + Sample(49) * t * t / Sample(288)
        + Sample(397) * t / Sample(1680) + Sample(4541) / Sample(40320);
      Sample polyBlepResidual8_5 = t * t * t * t * t * t * t * t / Sample(1920)
        - t * t * t * t * t * t * t / Sample(840) - t * t * t * t * t * t / Sample(360)
        + t * t * t * t / Sample(72) + t * t * t / Sample(30)
        + Sample(7) * t * t / Sample(180) + t / Sample(42) + Sample(31) / Sample(5040);
      Sample polyBlepResidual8_6 = -t * t * t * t * t * t * t * t / Sample(5760)
        + t * t * t * t * t * t * t / Sample(5040) + t * t * t * t * t * t / Sample(1440)
        + t * t * t * t * t / Sample(720) + t * t * t * t / Sample(576)
        + t * t * t / Sample(720) + t * t / Sample(1440) + t / Sample(5040)
        + Sample(1) / Sample(40320);
      Sample polyBlepResidual8_7 = t * t * t * t * t * t * t * t / Sample(40320);

      x0 += f3 * polyBlepResidual8_0;
      x1 += f3 * polyBlepResidual8_1;
      x2 += f3 * polyBlepResidual8_2;
      x3 += f3 * polyBlepResidual8_3;
      x4 += f3 * polyBlepResidual8_4;
      x5 += f3 * polyBlepResidual8_5;
      x6 += f3 * polyBlepResidual8_6;
      x7 += f3 * polyBlepResidual8_7;
    }

    f3 = f2;
    f2 = f1;
    f1 = f0;

    if (floored != lastInt) {
      if (floored < lastInt) height *= Sample(0.5);
      f0 = height * sign * std::copysign(Sample(1), lastInt - floored);
      lastInt = floored;
    } else {
      f0 = 0;
    }

    x7 = x6;
    x6 = x5;
    x5 = x4;
    x4 = x3;
    x3 = x2;
    x2 = x1;
    x1 = x0;
    x0 = input;

    return safeClip(x7);
  }
};

} // namespace SomeDSP
