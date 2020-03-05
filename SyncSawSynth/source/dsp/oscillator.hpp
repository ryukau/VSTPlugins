// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/somemath.hpp"

namespace SomeDSP {

template<typename Sample> class PTRSyncSaw {
public:
  PTRSyncSaw(Sample sampleRate, Sample oscFreq, Sample syncFreq) : sampleRate(sampleRate)
  {
    setOscFreq(oscFreq);
    setSyncFreq(syncFreq);
  }

  void setOscFreq(Sample hz) { oscTick = hz / sampleRate; }
  void setSyncFreq(Sample hz) { syncTick = hz / sampleRate; }
  void setOrder(uint32_t order) { this->order = order; }

  void setPhase(Sample phase) { oscPhase = phase - somefloor<Sample>(phase); }
  void addPhase(Sample phase)
  {
    oscPhase += phase - somefloor<Sample>(phase);
    if (oscPhase > 1.0) oscPhase -= 1.0;
  }

  void ptr()
  {
    switch (order) {
      case 0:
        lastSig = ptrSaw0(oscPhase, oscTick, height);
        break;

      case 1:
        lastSig = ptrSaw1(oscPhase, oscTick, height);
        break;

      case 2:
        lastSig = ptrSaw2(oscPhase, oscTick, height);
        break;

      case 3:
        lastSig = ptrSaw3(oscPhase, oscTick, height);
        break;

      case 4:
        lastSig = ptrSaw4(oscPhase, oscTick, height);
        break;

      case 5:
        lastSig = ptrSaw5(oscPhase, oscTick, height);
        break;

      case 6:
        lastSig = ptrSaw6(oscPhase, oscTick, height);
        break;

      default:
      case 7:
        lastSig = ptrSaw7(oscPhase, oscTick, height);
        break;

      case 8:
        lastSig = ptrSaw8(oscPhase, oscTick, height);
        break;

      case 9:
        lastSig = ptrSaw9(oscPhase, oscTick, height);
        break;

      case 10:
        lastSig = ptrSaw10(oscPhase, oscTick, height);
        break;

      case 11:
        lastSig = -somesin<Sample>(oscPhase * Sample(2.0) * Sample(pi));
        break;

      case 12:
        lastSig = ptrSaw6Double(oscPhase, oscTick, height);
        break;

      case 13:
        lastSig = ptrSaw7Double(oscPhase, oscTick, height);
        break;

      case 14:
        lastSig = ptrSaw8Double(oscPhase, oscTick, height);
        break;

      case 15:
        lastSig = ptrSaw9Double(oscPhase, oscTick, height);
        break;

      case 16:
        lastSig = ptrSaw10Double(oscPhase, oscTick, height);
        break;
    }
  }

  Sample process(Sample modOsc = 0.0, Sample modSync = 0.0)
  {
    syncPhase += syncTick + modSync;
    if (syncPhase >= Sample(1.0) || syncPhase < 0.0) {
      syncPhase -= somefloor<Sample>(syncPhase);

      oscPhase = syncPhase;
      if (syncTick != 0.0) {
        auto ratio = oscTick / syncTick;
        height = ratio - somefloor<Sample>(ratio);
      } else {
        height = lastSig;
        ptr();
        lastSig = lastSig > Sample(1.0) ? Sample(1.0)
                                        : lastSig < Sample(-1.0) ? Sample(-1.0) : lastSig;
        return lastSig;
      }
    } else {
      oscPhase += oscTick + modOsc;
      if (oscPhase >= Sample(1.0) || oscPhase < 0.0) {
        height = Sample(1.0);
        oscPhase -= somefloor<Sample>(oscPhase);
      }
    }
    ptr();
    if (!std::isfinite(lastSig)) lastSig = 0.0;
    return lastSig;
  }

protected:
  uint32_t order = 7;
  Sample sampleRate;
  Sample oscPhase = 0.0; // Range in [0, 1)
  Sample oscTick = 0.0;  // sec/sample
  Sample height = 1.0;   // Range in [0, 1]. Sample value at phase reset of hardsync.
  Sample syncPhase = 0.0;
  Sample syncTick = 0.0;
  Sample lastSig = 0.0;

  float ptrSaw0(float phi, float T, float h = 1.0)
  {
    return float(2) * T * phi / T - float(1);
  }

  float ptrSaw1(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(0)) return float(2) * T * n - T - float(1);
    if (n < float(1))
      return -float(2) * h * n + float(2) * T * n + float(2) * h - T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw2(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(1)) return float(2) * T * n - float(2) * T - float(1);
    if (n < float(1))
      return -h * n * n + float(2) * T * n + float(2) * h - float(2) * T - float(1);
    if (n < float(2))
      return h * n * n - float(4) * h * n + float(2) * T * n + float(4) * h - float(2) * T
        - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw3(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(2)) return float(2) * T * n - float(3) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n) / float(3) + float(2) * T * n + float(2) * h - float(3) * T
        - float(1);
    if (n < float(2))
      return (float(2) * h * n * n * n) / float(3) - float(3) * h * n * n
        + float(3) * h * n + float(2) * T * n + h - float(3) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n) / float(3) + float(3) * h * n * n - float(9) * h * n
        + float(2) * T * n + float(9) * h - float(3) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw4(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(3)) return float(2) * T * n - float(4) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n) / float(12) + float(2) * T * n + float(2) * h
        - float(4) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n) / float(4) - (float(4) * h * n * n * n) / float(3)
        + float(2) * h * n * n - (float(4) * h * n) / float(3) + float(2) * T * n
        + (float(7) * h) / float(3) - float(4) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n) / float(4) + (float(8) * h * n * n * n) / float(3)
        - float(10) * h * n * n + (float(44) * h * n) / float(3) + float(2) * T * n
        - (float(17) * h) / float(3) - float(4) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n) / float(12) - (float(4) * h * n * n * n) / float(3)
        + float(8) * h * n * n - (float(64) * h * n) / float(3) + float(2) * T * n
        + (float(64) * h) / float(3) - float(4) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw5(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(4)) return float(2) * T * n - float(5) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n) / float(60) + float(2) * T * n + float(2) * h
        - float(5) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n) / float(15)
        - (float(5) * h * n * n * n * n) / float(12)
        + (float(5) * h * n * n * n) / float(6) - (float(5) * h * n * n) / float(6)
        + (float(5) * h * n) / float(12) + float(2) * T * n + (float(23) * h) / float(12)
        - float(5) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n) / float(10)
        + (float(5) * h * n * n * n * n) / float(4)
        - (float(35) * h * n * n * n) / float(6) + (float(25) * h * n * n) / float(2)
        - (float(155) * h * n) / float(12) + float(2) * T * n + (float(29) * h) / float(4)
        - float(5) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n) / float(15)
        - (float(5) * h * n * n * n * n) / float(4)
        + (float(55) * h * n * n * n) / float(6) - (float(65) * h * n * n) / float(2)
        + (float(655) * h * n) / float(12) + float(2) * T * n
        - (float(133) * h) / float(4) - float(5) * T - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n) / float(60)
        + (float(5) * h * n * n * n * n) / float(12)
        - (float(25) * h * n * n * n) / float(6) + (float(125) * h * n * n) / float(6)
        - (float(625) * h * n) / float(12) + float(2) * T * n
        + (float(625) * h) / float(12) - float(5) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw6(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(5)) return float(2) * T * n - float(6) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n * n) / float(360) + float(2) * T * n + float(2) * h
        - float(6) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n * n) / float(72) - (h * n * n * n * n * n) / float(10)
        + (h * n * n * n * n) / float(4) - (h * n * n * n) / float(3)
        + (h * n * n) / float(4) - (h * n) / float(10) + float(2) * T * n
        + (float(121) * h) / float(60) - float(6) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n * n) / float(36)
        + (float(2) * h * n * n * n * n * n) / float(5)
        - (float(9) * h * n * n * n * n) / float(4)
        + (float(19) * h * n * n * n) / float(3) - (float(39) * h * n * n) / float(4)
        + (float(79) * h * n) / float(10) + float(2) * T * n - (float(13) * h) / float(20)
        - float(6) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n * n) / float(36)
        - (float(3) * h * n * n * n * n * n) / float(5)
        + (float(21) * h * n * n * n * n) / float(4)
        - (float(71) * h * n * n * n) / float(3) + (float(231) * h * n * n) / float(4)
        - (float(731) * h * n) / float(10) + float(2) * T * n
        + (float(797) * h) / float(20) - float(6) * T - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n * n) / float(72)
        + (float(2) * h * n * n * n * n * n) / float(5)
        - (float(19) * h * n * n * n * n) / float(4)
        + (float(89) * h * n * n * n) / float(3) - (float(409) * h * n * n) / float(4)
        + (float(1829) * h * n) / float(10) + float(2) * T * n
        - (float(7849) * h) / float(60) - float(6) * T - float(1);
    if (n < float(6))
      return (h * n * n * n * n * n * n) / float(360)
        - (h * n * n * n * n * n) / float(10) + (float(3) * h * n * n * n * n) / float(2)
        - float(12) * h * n * n * n + float(54) * h * n * n
        - (float(648) * h * n) / float(5) + float(2) * T * n + (float(648) * h) / float(5)
        - float(6) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw7(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(6)) return float(2) * T * n - float(7) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n * n * n) / float(2520) + float(2) * T * n
        + float(2) * h - float(7) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n * n * n) / float(420)
        - (float(7) * h * n * n * n * n * n * n) / float(360)
        + (float(7) * h * n * n * n * n * n) / float(120)
        - (float(7) * h * n * n * n * n) / float(72)
        + (float(7) * h * n * n * n) / float(72) - (float(7) * h * n * n) / float(120)
        + (float(7) * h * n) / float(360) + float(2) * T * n
        + (float(719) * h) / float(360) - float(7) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n * n * n) / float(168)
        + (float(7) * h * n * n * n * n * n * n) / float(72)
        - (float(77) * h * n * n * n * n * n) / float(120)
        + (float(161) * h * n * n * n * n) / float(72)
        - (float(329) * h * n * n * n) / float(72) + (float(133) * h * n * n) / float(24)
        - (float(1337) * h * n) / float(360) + float(2) * T * n
        + (float(1103) * h) / float(360) - float(7) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n * n * n) / float(126)
        - (float(7) * h * n * n * n * n * n * n) / float(36)
        + (float(119) * h * n * n * n * n * n) / float(60)
        - (float(98) * h * n * n * n * n) / float(9)
        + (float(1253) * h * n * n * n) / float(36) - (float(196) * h * n * n) / float(3)
        + (float(12089) * h * n) / float(180) + float(2) * T * n
        - (float(1229) * h) / float(45) - float(7) * T - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n * n * n) / float(168)
        + (float(7) * h * n * n * n * n * n * n) / float(36)
        - (float(161) * h * n * n * n * n * n) / float(60)
        + (float(182) * h * n * n * n * n) / float(9)
        - (float(3227) * h * n * n * n) / float(36) + (float(700) * h * n * n) / float(3)
        - (float(59591) * h * n) / float(180) + float(2) * T * n
        + (float(9011) * h) / float(45) - float(7) * T - float(1);
    if (n < float(6))
      return (h * n * n * n * n * n * n * n) / float(420)
        - (float(7) * h * n * n * n * n * n * n) / float(72)
        + (float(203) * h * n * n * n * n * n) / float(120)
        - (float(1169) * h * n * n * n * n) / float(72)
        + (float(6671) * h * n * n * n) / float(72)
        - (float(7525) * h * n * n) / float(24) + (float(208943) * h * n) / float(360)
        + float(2) * T * n - (float(162287) * h) / float(360) - float(7) * T - float(1);
    if (n < float(7))
      return -(h * n * n * n * n * n * n * n) / float(2520)
        + (float(7) * h * n * n * n * n * n * n) / float(360)
        - (float(49) * h * n * n * n * n * n) / float(120)
        + (float(343) * h * n * n * n * n) / float(72)
        - (float(2401) * h * n * n * n) / float(72)
        + (float(16807) * h * n * n) / float(120) - (float(117649) * h * n) / float(360)
        + float(2) * T * n + (float(117649) * h) / float(360) - float(7) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw8(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(7)) return float(2) * T * n - float(8) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n * n * n * n) / float(20160) + float(2) * T * n
        + float(2) * h - float(8) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n * n * n * n) / float(2880)
        - (h * n * n * n * n * n * n * n) / float(315)
        + (h * n * n * n * n * n * n) / float(90) - (h * n * n * n * n * n) / float(45)
        + (h * n * n * n * n) / float(36) - (h * n * n * n) / float(45)
        + (h * n * n) / float(90) - (h * n) / float(315) + float(2) * T * n
        + (float(5041) * h) / float(2520) - float(8) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n * n * n * n) / float(960)
        + (float(2) * h * n * n * n * n * n * n * n) / float(105)
        - (float(13) * h * n * n * n * n * n * n) / float(90)
        + (float(3) * h * n * n * n * n * n) / float(5)
        - (float(55) * h * n * n * n * n) / float(36)
        + (float(37) * h * n * n * n) / float(15) - (float(223) * h * n * n) / float(90)
        + (float(149) * h * n) / float(105) + float(2) * T * n
        + (float(829) * h) / float(504) - float(8) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n * n * n * n) / float(576)
        - (h * n * n * n * n * n * n * n) / float(21)
        + (float(5) * h * n * n * n * n * n * n) / float(9)
        - (float(18) * h * n * n * n * n * n) / float(5)
        + (float(128) * h * n * n * n * n) / float(9)
        - (float(106) * h * n * n * n) / float(3) + (float(488) * h * n * n) / float(9)
        - (float(4954) * h * n) / float(105) + float(2) * T * n
        + (float(6259) * h) / float(315) - float(8) * T - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n * n * n * n) / float(576)
        + (float(4) * h * n * n * n * n * n * n * n) / float(63)
        - h * n * n * n * n * n * n + (float(398) * h * n * n * n * n * n) / float(45)
        - float(48) * h * n * n * n * n + (float(1474) * h * n * n * n) / float(9)
        - float(344) * h * n * n + (float(128498) * h * n) / float(315) + float(2) * T * n
        - (float(7269) * h) / float(35) - float(8) * T - float(1);
    if (n < float(6))
      return (h * n * n * n * n * n * n * n * n) / float(960)
        - (h * n * n * n * n * n * n * n) / float(21)
        + (float(17) * h * n * n * n * n * n * n) / float(18)
        - (float(53) * h * n * n * n * n * n) / float(5)
        + (float(2647) * h * n * n * n * n) / float(36)
        - (float(967) * h * n * n * n) / float(3) + (float(15683) * h * n * n) / float(18)
        - (float(139459) * h * n) / float(105) + float(2) * T * n
        + (float(2211007) * h) / float(2520) - float(8) * T - float(1);
    if (n < float(7))
      return -(h * n * n * n * n * n * n * n * n) / float(2880)
        + (float(2) * h * n * n * n * n * n * n * n) / float(105)
        - (float(41) * h * n * n * n * n * n * n) / float(90)
        + (float(31) * h * n * n * n * n * n) / float(5)
        - (float(1889) * h * n * n * n * n) / float(36)
        + (float(4237) * h * n * n * n) / float(15)
        - (float(84881) * h * n * n) / float(90) + (float(187133) * h * n) / float(105)
        + float(2) * T * n - (float(3667649) * h) / float(2520) - float(8) * T - float(1);
    if (n < float(8))
      return (h * n * n * n * n * n * n * n * n) / float(20160)
        - (h * n * n * n * n * n * n * n) / float(315)
        + (float(4) * h * n * n * n * n * n * n) / float(45)
        - (float(64) * h * n * n * n * n * n) / float(45)
        + (float(128) * h * n * n * n * n) / float(9)
        - (float(4096) * h * n * n * n) / float(45)
        + (float(16384) * h * n * n) / float(45) - (float(262144) * h * n) / float(315)
        + float(2) * T * n + (float(262144) * h) / float(315) - float(8) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw9(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(8)) return float(2) * T * n - float(9) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n * n * n * n * n) / float(181440) + float(2) * T * n
        + float(2) * h - float(9) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n * n * n * n * n) / float(22680)
        - (h * n * n * n * n * n * n * n * n) / float(2240)
        + (h * n * n * n * n * n * n * n) / float(560)
        - (h * n * n * n * n * n * n) / float(240) + (h * n * n * n * n * n) / float(160)
        - (h * n * n * n * n) / float(160) + (h * n * n * n) / float(240)
        - (h * n * n) / float(560) + (h * n) / float(2240) + float(2) * T * n
        + (float(40319) * h) / float(20160) - float(9) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n * n * n * n * n) / float(6480)
        + (h * n * n * n * n * n * n * n * n) / float(320)
        - (float(3) * h * n * n * n * n * n * n * n) / float(112)
        + (float(31) * h * n * n * n * n * n * n) / float(240)
        - (float(63) * h * n * n * n * n * n) / float(160)
        + (float(127) * h * n * n * n * n) / float(160)
        - (float(17) * h * n * n * n) / float(16) + (float(73) * h * n * n) / float(80)
        - (float(1023) * h * n) / float(2240) + float(2) * T * n
        + (float(42367) * h) / float(20160) - float(9) * T - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n * n * n * n * n) / float(3240)
        - (float(3) * h * n * n * n * n * n * n * n * n) / float(320)
        + (float(69) * h * n * n * n * n * n * n * n) / float(560)
        - (float(221) * h * n * n * n * n * n * n) / float(240)
        + (float(693) * h * n * n * n * n * n) / float(160)
        - (float(2141) * h * n * n * n * n) / float(160)
        + (float(2183) * h * n * n * n) / float(80)
        - (float(2843) * h * n * n) / float(80) + (float(60213) * h * n) / float(2240)
        + float(2) * T * n - (float(141341) * h) / float(20160) - float(9) * T - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n * n * n * n * n) / float(2592)
        + (h * n * n * n * n * n * n * n * n) / float(64)
        - (float(31) * h * n * n * n * n * n * n * n) / float(112)
        + (float(45) * h * n * n * n * n * n * n) / float(16)
        - (float(2891) * h * n * n * n * n * n) / float(160)
        + (float(2439) * h * n * n * n * n) / float(32)
        - (float(10159) * h * n * n * n) / float(48)
        + (float(5985) * h * n * n) / float(16) - (float(857291) * h * n) / float(2240)
        + float(2) * T * n + (float(78415) * h) / float(448) - float(9) * T - float(1);
    if (n < float(6))
      return (h * n * n * n * n * n * n * n * n * n) / float(3240)
        - (h * n * n * n * n * n * n * n * n) / float(64)
        + (float(39) * h * n * n * n * n * n * n * n) / float(112)
        - (float(215) * h * n * n * n * n * n * n) / float(48)
        + (float(5859) * h * n * n * n * n * n) / float(160)
        - (float(6311) * h * n * n * n * n) / float(32)
        + (float(11197) * h * n * n * n) / float(16)
        - (float(25265) * h * n * n) / float(16) + (float(4611459) * h * n) / float(2240)
        + float(2) * T * n - (float(4763015) * h) / float(4032) - float(9) * T - float(1);
    if (n < float(7))
      return -(h * n * n * n * n * n * n * n * n * n) / float(6480)
        + (float(3) * h * n * n * n * n * n * n * n * n) / float(320)
        - (float(141) * h * n * n * n * n * n * n * n) / float(560)
        + (float(941) * h * n * n * n * n * n * n) / float(240)
        - (float(6237) * h * n * n * n * n * n) / float(160)
        + (float(41021) * h * n * n * n * n) / float(160)
        - (float(89167) * h * n * n * n) / float(80)
        + (float(246923) * h * n * n) / float(80)
        - (float(11064957) * h * n) / float(2240) + float(2) * T * n
        + (float(70243421) * h) / float(20160) - float(9) * T - float(1);
    if (n < float(8))
      return (h * n * n * n * n * n * n * n * n * n) / float(22680)
        - (h * n * n * n * n * n * n * n * n) / float(320)
        + (float(11) * h * n * n * n * n * n * n * n) / float(112)
        - (float(431) * h * n * n * n * n * n * n) / float(240)
        + (float(3367) * h * n * n * n * n * n) / float(160)
        - (float(26207) * h * n * n * n * n) / float(160)
        + (float(40619) * h * n * n * n) / float(48)
        - (float(223673) * h * n * n) / float(80)
        + (float(11994247) * h * n) / float(2240) + float(2) * T * n
        - (float(91171007) * h) / float(20160) - float(9) * T - float(1);
    if (n < float(9))
      return -(h * n * n * n * n * n * n * n * n * n) / float(181440)
        + (h * n * n * n * n * n * n * n * n) / float(2240)
        - (float(9) * h * n * n * n * n * n * n * n) / float(560)
        + (float(27) * h * n * n * n * n * n * n) / float(80)
        - (float(729) * h * n * n * n * n * n) / float(160)
        + (float(6561) * h * n * n * n * n) / float(160)
        - (float(19683) * h * n * n * n) / float(80)
        + (float(531441) * h * n * n) / float(560)
        - (float(4782969) * h * n) / float(2240) + float(2) * T * n
        + (float(4782969) * h) / float(2240) - float(9) * T - float(1);
    return 0.0; // Just in case.
  }

  float ptrSaw10(float phi, float T, float h = 1.0)
  {
    float n = phi / T;
    if (n >= float(9)) return float(2) * T * n - float(10) * T - float(1);
    if (n < float(1))
      return -(h * n * n * n * n * n * n * n * n * n * n) / float(1814400)
        + float(2) * T * n + float(2) * h - float(10) * T - float(1);
    if (n < float(2))
      return (h * n * n * n * n * n * n * n * n * n * n) / float(201600)
        - (h * n * n * n * n * n * n * n * n * n) / float(18144)
        + (h * n * n * n * n * n * n * n * n) / float(4032)
        - (h * n * n * n * n * n * n * n) / float(1512)
        + (h * n * n * n * n * n * n) / float(864) - (h * n * n * n * n * n) / float(720)
        + (h * n * n * n * n) / float(864) - (h * n * n * n) / float(1512)
        + (h * n * n) / float(4032) - (h * n) / float(18144) + float(2) * T * n
        + (float(362881) * h) / float(181440) - float(10) * T - float(1);
    if (n < float(3))
      return -(h * n * n * n * n * n * n * n * n * n * n) / float(50400)
        + (h * n * n * n * n * n * n * n * n * n) / float(2268)
        - (float(17) * h * n * n * n * n * n * n * n * n) / float(4032)
        + (float(5) * h * n * n * n * n * n * n * n) / float(216)
        - (float(71) * h * n * n * n * n * n * n) / float(864)
        + (float(143) * h * n * n * n * n * n) / float(720)
        - (float(287) * h * n * n * n * n) / float(864)
        + (float(575) * h * n * n * n) / float(1512)
        - (float(1151) * h * n * n) / float(4032) + (float(329) * h * n) / float(2592)
        + float(2) * T * n + (float(358273) * h) / float(181440) - float(10) * T
        - float(1);
    if (n < float(4))
      return (h * n * n * n * n * n * n * n * n * n * n) / float(21600)
        - (h * n * n * n * n * n * n * n * n * n) / float(648)
        + (float(13) * h * n * n * n * n * n * n * n * n) / float(576)
        - (float(289) * h * n * n * n * n * n * n * n) / float(1512)
        + (float(901) * h * n * n * n * n * n * n) / float(864)
        - (float(2773) * h * n * n * n * n * n) / float(720)
        + (float(8461) * h * n * n * n * n) / float(864)
        - (float(3667) * h * n * n * n) / float(216)
        + (float(11083) * h * n * n) / float(576) - (float(233893) * h * n) / float(18144)
        + float(2) * T * n + (float(1066861) * h) / float(181440) - float(10) * T
        - float(1);
    if (n < float(5))
      return -(h * n * n * n * n * n * n * n * n * n * n) / float(14400)
        + (h * n * n * n * n * n * n * n * n * n) / float(324)
        - (float(35) * h * n * n * n * n * n * n * n * n) / float(576)
        + (float(1055) * h * n * n * n * n * n * n * n) / float(1512)
        - (float(4475) * h * n * n * n * n * n * n) / float(864)
        + (float(18731) * h * n * n * n * n * n) / float(720)
        - (float(77555) * h * n * n * n * n) / float(864)
        + (float(45485) * h * n * n * n) / float(216)
        - (float(185525) * h * n * n) / float(576)
        + (float(5271131) * h * n) / float(18144) + float(2) * T * n
        - (float(4190647) * h) / float(36288) - float(10) * T - float(1);
    if (n < float(6))
      return (h * n * n * n * n * n * n * n * n * n * n) / float(14400)
        - (float(5) * h * n * n * n * n * n * n * n * n * n) / float(1296)
        + (float(55) * h * n * n * n * n * n * n * n * n) / float(576)
        - (float(2095) * h * n * n * n * n * n * n * n) / float(1512)
        + (float(11275) * h * n * n * n * n * n * n) / float(864)
        - (float(60019) * h * n * n * n * n * n) / float(720)
        + (float(316195) * h * n * n * n * n) / float(864)
        - (float(235765) * h * n * n * n) / float(216)
        + (float(1220725) * h * n * n) / float(576)
        - (float(43947619) * h * n) / float(18144) + float(2) * T * n
        + (float(45028103) * h) / float(36288) - float(10) * T - float(1);
    if (n < float(7))
      return -(h * n * n * n * n * n * n * n * n * n * n) / float(21600)
        + (h * n * n * n * n * n * n * n * n * n) / float(324)
        - (float(53) * h * n * n * n * n * n * n * n * n) / float(576)
        + (float(2441) * h * n * n * n * n * n * n * n) / float(1512)
        - (float(15941) * h * n * n * n * n * n * n) / float(864)
        + (float(103277) * h * n * n * n * n * n) / float(720)
        - (float(663581) * h * n * n * n * n) / float(864)
        + (float(604043) * h * n * n * n) / float(216)
        - (float(3818123) * h * n * n) / float(576)
        + (float(167683997) * h * n) / float(18144) + float(2) * T * n
        - (float(1044649181) * h) / float(181440) - float(10) * T - float(1);
    if (n < float(8))
      return (h * n * n * n * n * n * n * n * n * n * n) / float(50400)
        - (h * n * n * n * n * n * n * n * n * n) / float(648)
        + (float(31) * h * n * n * n * n * n * n * n * n) / float(576)
        - (float(1675) * h * n * n * n * n * n * n * n) / float(1512)
        + (float(12871) * h * n * n * n * n * n * n) / float(864)
        - (float(98407) * h * n * n * n * n * n) / float(720)
        + (float(748207) * h * n * n * n * n) / float(864)
        - (float(807745) * h * n * n * n) / float(216)
        + (float(6064393) * h * n * n) / float(576)
        - (float(316559287) * h * n) / float(18144) + float(2) * T * n
        + (float(2345053807) * h) / float(181440) - float(10) * T - float(1);
    if (n < float(9))
      return -(h * n * n * n * n * n * n * n * n * n * n) / float(201600)
        + (h * n * n * n * n * n * n * n * n * n) / float(2268)
        - (float(71) * h * n * n * n * n * n * n * n * n) / float(4032)
        + (float(629) * h * n * n * n * n * n * n * n) / float(1512)
        - (float(5561) * h * n * n * n * n * n * n) / float(864)
        + (float(49049) * h * n * n * n * n * n) / float(720)
        - (float(431441) * h * n * n * n * n) / float(864)
        + (float(3782969) * h * n * n * n) / float(1512)
        - (float(33046721) * h * n * n) / float(4032)
        + (float(287420489) * h * n) / float(18144) + float(2) * T * n
        - (float(2486784401) * h) / float(181440) - float(10) * T - float(1);
    if (n < float(10))
      return (h * n * n * n * n * n * n * n * n * n * n) / float(1814400)
        - (h * n * n * n * n * n * n * n * n * n) / float(18144)
        + (float(5) * h * n * n * n * n * n * n * n * n) / float(2016)
        - (float(25) * h * n * n * n * n * n * n * n) / float(378)
        + (float(125) * h * n * n * n * n * n * n) / float(108)
        - (float(125) * h * n * n * n * n * n) / float(9)
        + (float(3125) * h * n * n * n * n) / float(27)
        - (float(125000) * h * n * n * n) / float(189)
        + (float(156250) * h * n * n) / float(63) - (float(3125000) * h * n) / float(567)
        + float(2) * T * n + (float(3125000) * h) / float(567) - float(10) * T - float(1);
    return 0.0; // Just in case.
  }

  double ptrSaw6Double(double phi, double T, double h = 1.0)
  {
    double n = phi / T;
    if (n >= double(5)) return double(2) * T * n - double(6) * T - double(1);
    if (n < double(1))
      return -(h * n * n * n * n * n * n) / double(360) + double(2) * T * n
        + double(2) * h - double(6) * T - double(1);
    if (n < double(2))
      return (h * n * n * n * n * n * n) / double(72)
        - (h * n * n * n * n * n) / double(10) + (h * n * n * n * n) / double(4)
        - (h * n * n * n) / double(3) + (h * n * n) / double(4) - (h * n) / double(10)
        + double(2) * T * n + (double(121) * h) / double(60) - double(6) * T - double(1);
    if (n < double(3))
      return -(h * n * n * n * n * n * n) / double(36)
        + (double(2) * h * n * n * n * n * n) / double(5)
        - (double(9) * h * n * n * n * n) / double(4)
        + (double(19) * h * n * n * n) / double(3) - (double(39) * h * n * n) / double(4)
        + (double(79) * h * n) / double(10) + double(2) * T * n
        - (double(13) * h) / double(20) - double(6) * T - double(1);
    if (n < double(4))
      return (h * n * n * n * n * n * n) / double(36)
        - (double(3) * h * n * n * n * n * n) / double(5)
        + (double(21) * h * n * n * n * n) / double(4)
        - (double(71) * h * n * n * n) / double(3) + (double(231) * h * n * n) / double(4)
        - (double(731) * h * n) / double(10) + double(2) * T * n
        + (double(797) * h) / double(20) - double(6) * T - double(1);
    if (n < double(5))
      return -(h * n * n * n * n * n * n) / double(72)
        + (double(2) * h * n * n * n * n * n) / double(5)
        - (double(19) * h * n * n * n * n) / double(4)
        + (double(89) * h * n * n * n) / double(3) - (double(409) * h * n * n) / double(4)
        + (double(1829) * h * n) / double(10) + double(2) * T * n
        - (double(7849) * h) / double(60) - double(6) * T - double(1);
    if (n < double(6))
      return (h * n * n * n * n * n * n) / double(360)
        - (h * n * n * n * n * n) / double(10)
        + (double(3) * h * n * n * n * n) / double(2) - double(12) * h * n * n * n
        + double(54) * h * n * n - (double(648) * h * n) / double(5) + double(2) * T * n
        + (double(648) * h) / double(5) - double(6) * T - double(1);
    return 0.0; // Just in case.
  }

  double ptrSaw7Double(double phi, double T, double h = 1.0)
  {
    double n = phi / T;
    if (n >= double(6)) return double(2) * T * n - double(7) * T - double(1);
    if (n < double(1))
      return -(h * n * n * n * n * n * n * n) / double(2520) + double(2) * T * n
        + double(2) * h - double(7) * T - double(1);
    if (n < double(2))
      return (h * n * n * n * n * n * n * n) / double(420)
        - (double(7) * h * n * n * n * n * n * n) / double(360)
        + (double(7) * h * n * n * n * n * n) / double(120)
        - (double(7) * h * n * n * n * n) / double(72)
        + (double(7) * h * n * n * n) / double(72) - (double(7) * h * n * n) / double(120)
        + (double(7) * h * n) / double(360) + double(2) * T * n
        + (double(719) * h) / double(360) - double(7) * T - double(1);
    if (n < double(3))
      return -(h * n * n * n * n * n * n * n) / double(168)
        + (double(7) * h * n * n * n * n * n * n) / double(72)
        - (double(77) * h * n * n * n * n * n) / double(120)
        + (double(161) * h * n * n * n * n) / double(72)
        - (double(329) * h * n * n * n) / double(72)
        + (double(133) * h * n * n) / double(24) - (double(1337) * h * n) / double(360)
        + double(2) * T * n + (double(1103) * h) / double(360) - double(7) * T
        - double(1);
    if (n < double(4))
      return (h * n * n * n * n * n * n * n) / double(126)
        - (double(7) * h * n * n * n * n * n * n) / double(36)
        + (double(119) * h * n * n * n * n * n) / double(60)
        - (double(98) * h * n * n * n * n) / double(9)
        + (double(1253) * h * n * n * n) / double(36)
        - (double(196) * h * n * n) / double(3) + (double(12089) * h * n) / double(180)
        + double(2) * T * n - (double(1229) * h) / double(45) - double(7) * T - double(1);
    if (n < double(5))
      return -(h * n * n * n * n * n * n * n) / double(168)
        + (double(7) * h * n * n * n * n * n * n) / double(36)
        - (double(161) * h * n * n * n * n * n) / double(60)
        + (double(182) * h * n * n * n * n) / double(9)
        - (double(3227) * h * n * n * n) / double(36)
        + (double(700) * h * n * n) / double(3) - (double(59591) * h * n) / double(180)
        + double(2) * T * n + (double(9011) * h) / double(45) - double(7) * T - double(1);
    if (n < double(6))
      return (h * n * n * n * n * n * n * n) / double(420)
        - (double(7) * h * n * n * n * n * n * n) / double(72)
        + (double(203) * h * n * n * n * n * n) / double(120)
        - (double(1169) * h * n * n * n * n) / double(72)
        + (double(6671) * h * n * n * n) / double(72)
        - (double(7525) * h * n * n) / double(24) + (double(208943) * h * n) / double(360)
        + double(2) * T * n - (double(162287) * h) / double(360) - double(7) * T
        - double(1);
    if (n < double(7))
      return -(h * n * n * n * n * n * n * n) / double(2520)
        + (double(7) * h * n * n * n * n * n * n) / double(360)
        - (double(49) * h * n * n * n * n * n) / double(120)
        + (double(343) * h * n * n * n * n) / double(72)
        - (double(2401) * h * n * n * n) / double(72)
        + (double(16807) * h * n * n) / double(120)
        - (double(117649) * h * n) / double(360) + double(2) * T * n
        + (double(117649) * h) / double(360) - double(7) * T - double(1);
    return 0.0; // Just in case.
  }

  double ptrSaw8Double(double phi, double T, double h = 1.0)
  {
    double n = phi / T;
    if (n >= double(7)) return double(2) * T * n - double(8) * T - double(1);
    if (n < double(1))
      return -(h * n * n * n * n * n * n * n * n) / double(20160) + double(2) * T * n
        + double(2) * h - double(8) * T - double(1);
    if (n < double(2))
      return (h * n * n * n * n * n * n * n * n) / double(2880)
        - (h * n * n * n * n * n * n * n) / double(315)
        + (h * n * n * n * n * n * n) / double(90) - (h * n * n * n * n * n) / double(45)
        + (h * n * n * n * n) / double(36) - (h * n * n * n) / double(45)
        + (h * n * n) / double(90) - (h * n) / double(315) + double(2) * T * n
        + (double(5041) * h) / double(2520) - double(8) * T - double(1);
    if (n < double(3))
      return -(h * n * n * n * n * n * n * n * n) / double(960)
        + (double(2) * h * n * n * n * n * n * n * n) / double(105)
        - (double(13) * h * n * n * n * n * n * n) / double(90)
        + (double(3) * h * n * n * n * n * n) / double(5)
        - (double(55) * h * n * n * n * n) / double(36)
        + (double(37) * h * n * n * n) / double(15)
        - (double(223) * h * n * n) / double(90) + (double(149) * h * n) / double(105)
        + double(2) * T * n + (double(829) * h) / double(504) - double(8) * T - double(1);
    if (n < double(4))
      return (h * n * n * n * n * n * n * n * n) / double(576)
        - (h * n * n * n * n * n * n * n) / double(21)
        + (double(5) * h * n * n * n * n * n * n) / double(9)
        - (double(18) * h * n * n * n * n * n) / double(5)
        + (double(128) * h * n * n * n * n) / double(9)
        - (double(106) * h * n * n * n) / double(3)
        + (double(488) * h * n * n) / double(9) - (double(4954) * h * n) / double(105)
        + double(2) * T * n + (double(6259) * h) / double(315) - double(8) * T
        - double(1);
    if (n < double(5))
      return -(h * n * n * n * n * n * n * n * n) / double(576)
        + (double(4) * h * n * n * n * n * n * n * n) / double(63)
        - h * n * n * n * n * n * n + (double(398) * h * n * n * n * n * n) / double(45)
        - double(48) * h * n * n * n * n + (double(1474) * h * n * n * n) / double(9)
        - double(344) * h * n * n + (double(128498) * h * n) / double(315)
        + double(2) * T * n - (double(7269) * h) / double(35) - double(8) * T - double(1);
    if (n < double(6))
      return (h * n * n * n * n * n * n * n * n) / double(960)
        - (h * n * n * n * n * n * n * n) / double(21)
        + (double(17) * h * n * n * n * n * n * n) / double(18)
        - (double(53) * h * n * n * n * n * n) / double(5)
        + (double(2647) * h * n * n * n * n) / double(36)
        - (double(967) * h * n * n * n) / double(3)
        + (double(15683) * h * n * n) / double(18)
        - (double(139459) * h * n) / double(105) + double(2) * T * n
        + (double(2211007) * h) / double(2520) - double(8) * T - double(1);
    if (n < double(7))
      return -(h * n * n * n * n * n * n * n * n) / double(2880)
        + (double(2) * h * n * n * n * n * n * n * n) / double(105)
        - (double(41) * h * n * n * n * n * n * n) / double(90)
        + (double(31) * h * n * n * n * n * n) / double(5)
        - (double(1889) * h * n * n * n * n) / double(36)
        + (double(4237) * h * n * n * n) / double(15)
        - (double(84881) * h * n * n) / double(90)
        + (double(187133) * h * n) / double(105) + double(2) * T * n
        - (double(3667649) * h) / double(2520) - double(8) * T - double(1);
    if (n < double(8))
      return (h * n * n * n * n * n * n * n * n) / double(20160)
        - (h * n * n * n * n * n * n * n) / double(315)
        + (double(4) * h * n * n * n * n * n * n) / double(45)
        - (double(64) * h * n * n * n * n * n) / double(45)
        + (double(128) * h * n * n * n * n) / double(9)
        - (double(4096) * h * n * n * n) / double(45)
        + (double(16384) * h * n * n) / double(45)
        - (double(262144) * h * n) / double(315) + double(2) * T * n
        + (double(262144) * h) / double(315) - double(8) * T - double(1);
    return 0.0; // Just in case.
  }

  double ptrSaw9Double(double phi, double T, double h = 1.0)
  {
    double n = phi / T;
    if (n >= double(8)) return double(2) * T * n - double(9) * T - double(1);
    if (n < double(1))
      return -(h * n * n * n * n * n * n * n * n * n) / double(181440) + double(2) * T * n
        + double(2) * h - double(9) * T - double(1);
    if (n < double(2))
      return (h * n * n * n * n * n * n * n * n * n) / double(22680)
        - (h * n * n * n * n * n * n * n * n) / double(2240)
        + (h * n * n * n * n * n * n * n) / double(560)
        - (h * n * n * n * n * n * n) / double(240)
        + (h * n * n * n * n * n) / double(160) - (h * n * n * n * n) / double(160)
        + (h * n * n * n) / double(240) - (h * n * n) / double(560)
        + (h * n) / double(2240) + double(2) * T * n + (double(40319) * h) / double(20160)
        - double(9) * T - double(1);
    if (n < double(3))
      return -(h * n * n * n * n * n * n * n * n * n) / double(6480)
        + (h * n * n * n * n * n * n * n * n) / double(320)
        - (double(3) * h * n * n * n * n * n * n * n) / double(112)
        + (double(31) * h * n * n * n * n * n * n) / double(240)
        - (double(63) * h * n * n * n * n * n) / double(160)
        + (double(127) * h * n * n * n * n) / double(160)
        - (double(17) * h * n * n * n) / double(16)
        + (double(73) * h * n * n) / double(80) - (double(1023) * h * n) / double(2240)
        + double(2) * T * n + (double(42367) * h) / double(20160) - double(9) * T
        - double(1);
    if (n < double(4))
      return (h * n * n * n * n * n * n * n * n * n) / double(3240)
        - (double(3) * h * n * n * n * n * n * n * n * n) / double(320)
        + (double(69) * h * n * n * n * n * n * n * n) / double(560)
        - (double(221) * h * n * n * n * n * n * n) / double(240)
        + (double(693) * h * n * n * n * n * n) / double(160)
        - (double(2141) * h * n * n * n * n) / double(160)
        + (double(2183) * h * n * n * n) / double(80)
        - (double(2843) * h * n * n) / double(80) + (double(60213) * h * n) / double(2240)
        + double(2) * T * n - (double(141341) * h) / double(20160) - double(9) * T
        - double(1);
    if (n < double(5))
      return -(h * n * n * n * n * n * n * n * n * n) / double(2592)
        + (h * n * n * n * n * n * n * n * n) / double(64)
        - (double(31) * h * n * n * n * n * n * n * n) / double(112)
        + (double(45) * h * n * n * n * n * n * n) / double(16)
        - (double(2891) * h * n * n * n * n * n) / double(160)
        + (double(2439) * h * n * n * n * n) / double(32)
        - (double(10159) * h * n * n * n) / double(48)
        + (double(5985) * h * n * n) / double(16)
        - (double(857291) * h * n) / double(2240) + double(2) * T * n
        + (double(78415) * h) / double(448) - double(9) * T - double(1);
    if (n < double(6))
      return (h * n * n * n * n * n * n * n * n * n) / double(3240)
        - (h * n * n * n * n * n * n * n * n) / double(64)
        + (double(39) * h * n * n * n * n * n * n * n) / double(112)
        - (double(215) * h * n * n * n * n * n * n) / double(48)
        + (double(5859) * h * n * n * n * n * n) / double(160)
        - (double(6311) * h * n * n * n * n) / double(32)
        + (double(11197) * h * n * n * n) / double(16)
        - (double(25265) * h * n * n) / double(16)
        + (double(4611459) * h * n) / double(2240) + double(2) * T * n
        - (double(4763015) * h) / double(4032) - double(9) * T - double(1);
    if (n < double(7))
      return -(h * n * n * n * n * n * n * n * n * n) / double(6480)
        + (double(3) * h * n * n * n * n * n * n * n * n) / double(320)
        - (double(141) * h * n * n * n * n * n * n * n) / double(560)
        + (double(941) * h * n * n * n * n * n * n) / double(240)
        - (double(6237) * h * n * n * n * n * n) / double(160)
        + (double(41021) * h * n * n * n * n) / double(160)
        - (double(89167) * h * n * n * n) / double(80)
        + (double(246923) * h * n * n) / double(80)
        - (double(11064957) * h * n) / double(2240) + double(2) * T * n
        + (double(70243421) * h) / double(20160) - double(9) * T - double(1);
    if (n < double(8))
      return (h * n * n * n * n * n * n * n * n * n) / double(22680)
        - (h * n * n * n * n * n * n * n * n) / double(320)
        + (double(11) * h * n * n * n * n * n * n * n) / double(112)
        - (double(431) * h * n * n * n * n * n * n) / double(240)
        + (double(3367) * h * n * n * n * n * n) / double(160)
        - (double(26207) * h * n * n * n * n) / double(160)
        + (double(40619) * h * n * n * n) / double(48)
        - (double(223673) * h * n * n) / double(80)
        + (double(11994247) * h * n) / double(2240) + double(2) * T * n
        - (double(91171007) * h) / double(20160) - double(9) * T - double(1);
    if (n < double(9))
      return -(h * n * n * n * n * n * n * n * n * n) / double(181440)
        + (h * n * n * n * n * n * n * n * n) / double(2240)
        - (double(9) * h * n * n * n * n * n * n * n) / double(560)
        + (double(27) * h * n * n * n * n * n * n) / double(80)
        - (double(729) * h * n * n * n * n * n) / double(160)
        + (double(6561) * h * n * n * n * n) / double(160)
        - (double(19683) * h * n * n * n) / double(80)
        + (double(531441) * h * n * n) / double(560)
        - (double(4782969) * h * n) / double(2240) + double(2) * T * n
        + (double(4782969) * h) / double(2240) - double(9) * T - double(1);
    return 0.0; // Just in case.
  }

  double ptrSaw10Double(double phi, double T, double h = 1.0)
  {
    double n = phi / T;
    if (n >= double(9)) return double(2) * T * n - double(10) * T - double(1);
    if (n < double(1))
      return -(h * n * n * n * n * n * n * n * n * n * n) / double(1814400)
        + double(2) * T * n + double(2) * h - double(10) * T - double(1);
    if (n < double(2))
      return (h * n * n * n * n * n * n * n * n * n * n) / double(201600)
        - (h * n * n * n * n * n * n * n * n * n) / double(18144)
        + (h * n * n * n * n * n * n * n * n) / double(4032)
        - (h * n * n * n * n * n * n * n) / double(1512)
        + (h * n * n * n * n * n * n) / double(864)
        - (h * n * n * n * n * n) / double(720) + (h * n * n * n * n) / double(864)
        - (h * n * n * n) / double(1512) + (h * n * n) / double(4032)
        - (h * n) / double(18144) + double(2) * T * n
        + (double(362881) * h) / double(181440) - double(10) * T - double(1);
    if (n < double(3))
      return -(h * n * n * n * n * n * n * n * n * n * n) / double(50400)
        + (h * n * n * n * n * n * n * n * n * n) / double(2268)
        - (double(17) * h * n * n * n * n * n * n * n * n) / double(4032)
        + (double(5) * h * n * n * n * n * n * n * n) / double(216)
        - (double(71) * h * n * n * n * n * n * n) / double(864)
        + (double(143) * h * n * n * n * n * n) / double(720)
        - (double(287) * h * n * n * n * n) / double(864)
        + (double(575) * h * n * n * n) / double(1512)
        - (double(1151) * h * n * n) / double(4032) + (double(329) * h * n) / double(2592)
        + double(2) * T * n + (double(358273) * h) / double(181440) - double(10) * T
        - double(1);
    if (n < double(4))
      return (h * n * n * n * n * n * n * n * n * n * n) / double(21600)
        - (h * n * n * n * n * n * n * n * n * n) / double(648)
        + (double(13) * h * n * n * n * n * n * n * n * n) / double(576)
        - (double(289) * h * n * n * n * n * n * n * n) / double(1512)
        + (double(901) * h * n * n * n * n * n * n) / double(864)
        - (double(2773) * h * n * n * n * n * n) / double(720)
        + (double(8461) * h * n * n * n * n) / double(864)
        - (double(3667) * h * n * n * n) / double(216)
        + (double(11083) * h * n * n) / double(576)
        - (double(233893) * h * n) / double(18144) + double(2) * T * n
        + (double(1066861) * h) / double(181440) - double(10) * T - double(1);
    if (n < double(5))
      return -(h * n * n * n * n * n * n * n * n * n * n) / double(14400)
        + (h * n * n * n * n * n * n * n * n * n) / double(324)
        - (double(35) * h * n * n * n * n * n * n * n * n) / double(576)
        + (double(1055) * h * n * n * n * n * n * n * n) / double(1512)
        - (double(4475) * h * n * n * n * n * n * n) / double(864)
        + (double(18731) * h * n * n * n * n * n) / double(720)
        - (double(77555) * h * n * n * n * n) / double(864)
        + (double(45485) * h * n * n * n) / double(216)
        - (double(185525) * h * n * n) / double(576)
        + (double(5271131) * h * n) / double(18144) + double(2) * T * n
        - (double(4190647) * h) / double(36288) - double(10) * T - double(1);
    if (n < double(6))
      return (h * n * n * n * n * n * n * n * n * n * n) / double(14400)
        - (double(5) * h * n * n * n * n * n * n * n * n * n) / double(1296)
        + (double(55) * h * n * n * n * n * n * n * n * n) / double(576)
        - (double(2095) * h * n * n * n * n * n * n * n) / double(1512)
        + (double(11275) * h * n * n * n * n * n * n) / double(864)
        - (double(60019) * h * n * n * n * n * n) / double(720)
        + (double(316195) * h * n * n * n * n) / double(864)
        - (double(235765) * h * n * n * n) / double(216)
        + (double(1220725) * h * n * n) / double(576)
        - (double(43947619) * h * n) / double(18144) + double(2) * T * n
        + (double(45028103) * h) / double(36288) - double(10) * T - double(1);
    if (n < double(7))
      return -(h * n * n * n * n * n * n * n * n * n * n) / double(21600)
        + (h * n * n * n * n * n * n * n * n * n) / double(324)
        - (double(53) * h * n * n * n * n * n * n * n * n) / double(576)
        + (double(2441) * h * n * n * n * n * n * n * n) / double(1512)
        - (double(15941) * h * n * n * n * n * n * n) / double(864)
        + (double(103277) * h * n * n * n * n * n) / double(720)
        - (double(663581) * h * n * n * n * n) / double(864)
        + (double(604043) * h * n * n * n) / double(216)
        - (double(3818123) * h * n * n) / double(576)
        + (double(167683997) * h * n) / double(18144) + double(2) * T * n
        - (double(1044649181) * h) / double(181440) - double(10) * T - double(1);
    if (n < double(8))
      return (h * n * n * n * n * n * n * n * n * n * n) / double(50400)
        - (h * n * n * n * n * n * n * n * n * n) / double(648)
        + (double(31) * h * n * n * n * n * n * n * n * n) / double(576)
        - (double(1675) * h * n * n * n * n * n * n * n) / double(1512)
        + (double(12871) * h * n * n * n * n * n * n) / double(864)
        - (double(98407) * h * n * n * n * n * n) / double(720)
        + (double(748207) * h * n * n * n * n) / double(864)
        - (double(807745) * h * n * n * n) / double(216)
        + (double(6064393) * h * n * n) / double(576)
        - (double(316559287) * h * n) / double(18144) + double(2) * T * n
        + (double(2345053807) * h) / double(181440) - double(10) * T - double(1);
    if (n < double(9))
      return -(h * n * n * n * n * n * n * n * n * n * n) / double(201600)
        + (h * n * n * n * n * n * n * n * n * n) / double(2268)
        - (double(71) * h * n * n * n * n * n * n * n * n) / double(4032)
        + (double(629) * h * n * n * n * n * n * n * n) / double(1512)
        - (double(5561) * h * n * n * n * n * n * n) / double(864)
        + (double(49049) * h * n * n * n * n * n) / double(720)
        - (double(431441) * h * n * n * n * n) / double(864)
        + (double(3782969) * h * n * n * n) / double(1512)
        - (double(33046721) * h * n * n) / double(4032)
        + (double(287420489) * h * n) / double(18144) + double(2) * T * n
        - (double(2486784401) * h) / double(181440) - double(10) * T - double(1);
    if (n < double(10))
      return (h * n * n * n * n * n * n * n * n * n * n) / double(1814400)
        - (h * n * n * n * n * n * n * n * n * n) / double(18144)
        + (double(5) * h * n * n * n * n * n * n * n * n) / double(2016)
        - (double(25) * h * n * n * n * n * n * n * n) / double(378)
        + (double(125) * h * n * n * n * n * n * n) / double(108)
        - (double(125) * h * n * n * n * n * n) / double(9)
        + (double(3125) * h * n * n * n * n) / double(27)
        - (double(125000) * h * n * n * n) / double(189)
        + (double(156250) * h * n * n) / double(63)
        - (double(3125000) * h * n) / double(567) + double(2) * T * n
        + (double(3125000) * h) / double(567) - double(10) * T - double(1);
    return 0.0; // Just in case.
  }
};

} // namespace SomeDSP
