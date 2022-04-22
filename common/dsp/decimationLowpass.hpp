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

#include <algorithm>
#include <array>

namespace SomeDSP {

/**
Lowpass filter coefficient specialized for 16x oversampling.
Sos stands for second order sections.

```python
import numpy
from scipy import signal

samplerate = 48000
uprate = samplerate * 16 / 2
sos = signal.butter(16, samplerate / 1.8, output="sos", fs=uprate)
```
*/
template<typename Sample> struct Sos16FoldFirstStage {
  constexpr static std::array<std::array<Sample, 5>, 8> co{{
    {Sample(3.5903469155931847e-12), Sample(7.1806938311863695e-12),
     Sample(3.5903469155931847e-12), Sample(-1.2759657610561284),
     Sample(0.40787244610150275)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.2906502176887378),
     Sample(0.42407495130188644)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.320459244427636),
     Sample(0.456965573191349)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.3662708320207162),
     Sample(0.5075130673741699)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.429387848302023),
     Sample(0.5771549894497601)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.5114943545116066),
     Sample(0.6677494954045713)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.6145439579130596),
     Sample(0.7814521523555764)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.7405167001403739),
     Sample(0.9204476945203488)},
  }};
};

/**
Lowpass filter coefficient specialized for 8x oversampling.
Sos stands for second order sections.

```python
import numpy
from scipy import signal

samplerate = 48000
uprate = samplerate * 8 / 2
sos = signal.butter(10, samplerate / 1.9, output="sos", fs=uprate)
```
*/
template<typename Sample> struct Sos8FoldFirstStage {
  constexpr static std::array<std::array<Sample, 5>, 5> co{{
    {Sample(1.6921576928941614e-05), Sample(3.384315385788323e-05),
     Sample(1.6921576928941614e-05), Sample(-0.7844963643040177),
     Sample(0.1583016535071512)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-0.818202758191658),
     Sample(0.20806883350989597)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-0.8910219912408522),
     Sample(0.31558576014706946)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.0154057703544481),
     Sample(0.49923726393009143)},
    {Sample(1.0), Sample(2.0), Sample(1.0), Sample(-1.2147539782254588),
     Sample(0.7935730560475367)},
  }};
};

template<typename Sample, typename Sos> class DecimationLowpass {
  std::array<Sample, Sos::co.size()> x0{};
  std::array<Sample, Sos::co.size()> x1{};
  std::array<Sample, Sos::co.size()> x2{};
  std::array<Sample, Sos::co.size()> y0{};
  std::array<Sample, Sos::co.size()> y1{};
  std::array<Sample, Sos::co.size()> y2{};

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

  void push(Sample input) noexcept
  {
    x0[0] = input;
    std::copy(y0.begin(), y0.end() - 1, x0.begin() + 1);

    for (size_t i = 0; i < Sos::co.size(); ++i) {
      y0[i]                      //
        = Sos::co[i][0] * x0[i]  //
        + Sos::co[i][1] * x1[i]  //
        + Sos::co[i][2] * x2[i]  //
        - Sos::co[i][3] * y1[i]  //
        - Sos::co[i][4] * y2[i]; //
    }

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;
  }

  inline Sample output() { return y0[Sos::co.size() - 1]; }
};

template<typename T> struct HalfBandCoefficient {
  static constexpr std::array<T, 9> h0_a{
    T(0.0765690656031399), T(0.264282270318935),  T(0.47939467893641907),
    T(0.661681722389424),  T(0.7924031566294969), T(0.8776927911111817),
    T(0.9308500986629166), T(0.9640156636878193), T(0.9862978287283355),
  };
  static constexpr std::array<T, 10> h1_a{
    T(0.019911761024506557), T(0.16170648261075027), T(0.37320978687920564),
    T(0.5766558985008232),   T(0.7334355636406803),  T(0.8399227128761151),
    T(0.9074601780285125),   T(0.9492937701934973),  T(0.9760539731706528),
    T(0.9955323321150525),
  };
};

template<typename Sample> class FirstOrderAllpass {
private:
  Sample x1 = 0;
  Sample y1 = 0;

public:
  void reset()
  {
    x1 = 0;
    y1 = 0;
  }

  Sample process(Sample x0, Sample a)
  {
    y1 = a * (x0 - y1) + x1;
    x1 = x0;
    return y1;
  }
};

template<typename Sample, typename Coefficient> class HalfBandIIR {
private:
  std::array<FirstOrderAllpass<Sample>, Coefficient::h0_a.size()> ap0;
  std::array<FirstOrderAllpass<Sample>, Coefficient::h1_a.size()> ap1;

public:
  void reset()
  {
    for (auto &ap : ap0) ap.reset();
    for (auto &ap : ap1) ap.reset();
  }

  // input[0] must be earlier sample.
  Sample process(const std::array<Sample, 2> &input)
  {
    auto s0 = input[0];
    for (size_t i = 0; i < ap0.size(); ++i) s0 = ap0[i].process(s0, Coefficient::h0_a[i]);
    auto s1 = input[1];
    for (size_t i = 0; i < ap1.size(); ++i) s1 = ap1[i].process(s1, Coefficient::h1_a[i]);
    return Sample(0.5) * (s0 + s1);
  }
};

template<typename Sample> struct SocpFractionalDelayFir8x16 {
  constexpr static size_t bufferSize = 8;
  constexpr static size_t intDelay = 3;
  constexpr static size_t upfold = 16;

  constexpr static std::array<std::array<Sample, bufferSize>, 15> coefficient{{
    {Sample(-0.0022352501481522655), Sample(0.008885349968296331),
     Sample(-0.024354054855736534), Sample(0.06590664320017858),
     Sample(0.9861842357058305), Sample(-0.04406225776248801),
     Sample(0.012495968362007196), Sample(-0.0028206344699358288)},
    {Sample(-0.004422008617515909), Sample(0.017678364285843788),
     Sample(-0.049015774506830374), Sample(0.1375857774965184),
     Sample(0.9610474292621478), Sample(-0.08113507095427837),
     Sample(0.023655194021416276), Sample(-0.005393910987301639)},
    {Sample(-0.006481751766702893), Sample(0.026068898795753827),
     Sample(-0.07316648584234423), Sample(0.2138839581807108), Sample(0.9251677633239604),
     Sample(-0.11103366120494834), Sample(0.03320712312705344),
     Sample(-0.007645844613483016)},
    {Sample(-0.008339269088424483), Sample(0.033751270722197295),
     Sample(-0.09595987673214824), Sample(0.2935281473182622), Sample(0.8793138700077404),
     Sample(-0.13372324347559555), Sample(0.04094444248006669),
     Sample(-0.009515341232098299)},
    {Sample(-0.009924629239474651), Sample(0.04043224384484812),
     Sample(-0.11653997412417323), Sample(0.37514840586871834),
     Sample(0.8244305456051493), Sample(-0.14931634935281096),
     Sample(0.046725467897377325), Sample(-0.010955710499634258)},
    {Sample(-0.011175099746906208), Sample(0.04583932008647262),
     Sample(-0.1340601884888018), Sample(0.4573036964605567), Sample(0.7616201950339072),
     Sample(-0.15806693074393935), Sample(0.05047473356703032),
     Sample(-0.01193572616831952)},
    {Sample(-0.012037217537914304), Sample(0.04972938261318181),
     Sample(-0.147703576266092), Sample(0.5385108269409077), Sample(0.6921197674294074),
     Sample(-0.1603607383673463), Sample(0.05218151613345006),
     Sample(-0.012439960945594417)},
    {Sample(-0.012468671179450263), Sample(0.05189689755716546),
     Sample(-0.15670299700822185), Sample(0.6172747706305065), Sample(0.617274770630507),
     Sample(-0.15670299700822216), Sample(0.051896897557165644),
     Sample(-0.012468671179450413)},
    {Sample(-0.012439960945594176), Sample(0.0521815161334496),
     Sample(-0.16036073836734535), Sample(0.6921197674294061), Sample(0.5385108269409089),
     Sample(-0.14770357626609298), Sample(0.04972938261318241),
     Sample(-0.012037217537914521)},
    {Sample(-0.011935726168319402), Sample(0.05047473356703032),
     Sample(-0.15806693074393935), Sample(0.7616201950339071),
     Sample(0.45730369646055646), Sample(-0.13406018848880166),
     Sample(0.04583932008647259), Sample(-0.011175099746906138)},
    {Sample(-0.010955710499634222), Sample(0.04672546789737744),
     Sample(-0.14931634935281105), Sample(0.8244305456051493),
     Sample(0.37514840586871845), Sample(-0.1165399741241734),
     Sample(0.04043224384484828), Sample(-0.009924629239474741)},
    {Sample(-0.009515341232098452), Sample(0.04094444248006681),
     Sample(-0.1337232434755956), Sample(0.8793138700077404), Sample(0.2935281473182622),
     Sample(-0.0959598767321482), Sample(0.03375127072219737),
     Sample(-0.008339269088424524)},
    {Sample(-0.007645844613483245), Sample(0.033207123127054),
     Sample(-0.11103366120494915), Sample(0.9251677633239614),
     Sample(0.21388395818070982), Sample(-0.07316648584234348),
     Sample(0.026068898795753425), Sample(-0.006481751766702774)},
    {Sample(-0.005393910987301498), Sample(0.023655194021415877),
     Sample(-0.08113507095427787), Sample(0.9610474292621471), Sample(0.1375857774965191),
     Sample(-0.049015774506830825), Sample(0.017678364285844052),
     Sample(-0.004422008617515965)},
    {Sample(-0.00282063446993611), Sample(0.012495968362007866),
     Sample(-0.04406225776248905), Sample(0.9861842357058319),
     Sample(0.06590664320017736), Sample(-0.0243540548557357),
     Sample(0.008885349968295896), Sample(-0.002235250148152139)},
  }};
};

template<typename Sample> struct SocpFractionalDelayFir16x8 {
  constexpr static size_t bufferSize = 16;
  constexpr static size_t intDelay = 7;
  constexpr static size_t upfold = 8;

  constexpr static std::array<std::array<Sample, bufferSize>, 7> coefficient{{
    {Sample(-0.0009137369460620322), Sample(0.002507493782052301),
     Sample(-0.0056152875379307165), Sample(0.010878179517888449),
     Sample(-0.019383293443834226), Sample(0.03343051695978674),
     Sample(-0.060000280418228386), Sample(0.1388043588535928),
     Sample(0.9706989777508082), Sample(-0.09971191279047106),
     Sample(0.04501436126022387), Sample(-0.023871162274251973),
     Sample(0.012745036651631602), Sample(-0.006367107398254405),
     Sample(0.0027754881923304623), Sample(-0.0009916321592815076)},
    {Sample(-0.0017057925062780666), Sample(0.004694960889597546),
     Sample(-0.010552315361198463), Sample(0.02054134966360736),
     Sample(-0.03686242369686063), Sample(0.06434512021196942),
     Sample(-0.11839028099352701), Sample(0.2983250634779617), Sample(0.8944024680983272),
     Sample(-0.16542709350314344), Sample(0.0783941479518113),
     Sample(-0.042339536581919766), Sample(0.022825492065605378),
     Sample(-0.011473344104266401), Sample(0.005023500744089398),
     Sample(-0.0018013163557756891)},
    {Sample(-0.0022547647050963422), Sample(0.006225161880998577),
     Sample(-0.014044618452944068), Sample(0.02747810795399806),
     Sample(-0.04968334377178693), Sample(0.08785653467168976),
     Sample(-0.1662225126221644), Sample(0.4668747751928871), Sample(0.7778755173959371),
     Sample(-0.196255426397257), Sample(0.09695013280568895),
     Sample(-0.05324169904989514), Sample(0.02896431865363349),
     Sample(-0.014644353412156986), Sample(0.00643918376174353),
     Sample(-0.002317013905275785)},
    {Sample(-0.002472351647752062), Sample(0.006847903393051679),
     Sample(-0.015510420804278311), Sample(0.030507163698455034),
     Sample(-0.05560286818245174), Sample(0.09971508045478719),
     Sample(-0.19470854168199406), Sample(0.6312240347701845), Sample(0.6312240347701751),
     Sample(-0.19470854168198481), Sample(0.09971508045477946),
     Sample(-0.055602868182445774), Sample(0.03050716369845062),
     Sample(-0.015510420804276245), Sample(0.006847903393050949),
     Sample(-0.00247235164775159)},
    {Sample(-0.0023170139052764993), Sample(0.006439183761743969),
     Sample(-0.014644353412158313), Sample(0.028964318653636138),
     Sample(-0.05324169904989862), Sample(0.09695013280569362),
     Sample(-0.1962554263972624), Sample(0.7778755173959423), Sample(0.4668747751928819),
     Sample(-0.16622251262215895), Sample(0.08785653467168454),
     Sample(-0.04968334377178279), Sample(0.02747810795399469),
     Sample(-0.014044618452942137), Sample(0.006225161880997826),
     Sample(-0.0022547647050953244)},
    {Sample(-0.001801316355776215), Sample(0.005023500744089943),
     Sample(-0.011473344104267284), Sample(0.02282549206560673),
     Sample(-0.04233953658192138), Sample(0.07839414795181288),
     Sample(-0.16542709350314516), Sample(0.8944024680983295),
     Sample(0.29832506347795934), Sample(-0.11839028099352511),
     Sample(0.06434512021196787), Sample(-0.03686242369685906),
     Sample(0.020541349663606007), Sample(-0.010552315361197575),
     Sample(0.004694960889596957), Sample(-0.0017057925062772112)},
    {Sample(-0.0009916321592814278), Sample(0.002775488192330381),
     Sample(-0.006367107398253951), Sample(0.012745036651630771),
     Sample(-0.023871162274250773), Sample(0.04501436126022267),
     Sample(-0.09971191279046962), Sample(0.9706989777508058), Sample(0.1388043588535946),
     Sample(-0.060000280418229995), Sample(0.03343051695978791),
     Sample(-0.019383293443835138), Sample(0.010878179517889294),
     Sample(-0.005615287537931306), Sample(0.0025074937820525715),
     Sample(-0.0009137369460617704)},
  }};
};

template<typename Sample, typename FractionalDelayFIR> class FirUpSampler {
  std::array<Sample, FractionalDelayFIR::bufferSize> buf{};

public:
  std::array<Sample, FractionalDelayFIR::upfold> output;

  void reset() { buf.fill(Sample(0)); }

  void process(Sample input)
  {
    std::rotate(buf.rbegin(), buf.rbegin() + 1, buf.rend());
    buf[0] = input;

    std::fill(output.begin(), output.end() - 1, Sample(0));
    output.back() = buf[FractionalDelayFIR::intDelay];
    for (size_t i = 0; i < FractionalDelayFIR::coefficient.size(); ++i) {
      auto &&phase = FractionalDelayFIR::coefficient[i];
      for (size_t n = 0; n < phase.size(); ++n) output[i] += buf[n] * phase[n];
    }
  }
};

template<typename Sample, size_t upSample> class CubicUpSampler {
  std::array<Sample, 4> buf{};

public:
  std::array<Sample, upSample> output;

  void reset() { buf.fill(Sample(0)); }

  // Range of t is in [0, 1]. Interpolates between y1 and y2.
  inline Sample cubicInterp(const std::array<Sample, 4> &y, Sample t)
  {
    auto t2 = t * t;
    auto c0 = y[1] - y[2];
    auto c1 = (y[2] - y[0]) * 0.5f;
    auto c2 = c0 + c1;
    auto c3 = c0 + c2 + (y[3] - y[1]) * 0.5f;
    return c3 * t * t2 - (c2 + c3) * t2 + c1 * t + y[1];
  }

  void process(Sample input)
  {
    std::rotate(buf.begin(), buf.begin() + 1, buf.end());
    buf.back() = input;

    std::fill(output.begin() + 1, output.end(), Sample(0));
    output[0] = buf[1];
    for (size_t i = 1; i < output.size(); ++i) {
      output[i] = cubicInterp(buf, Sample(i) / Sample(upSample));
    }
  }
};

template<typename Sample, size_t upSample> class LinearUpSampler {
  Sample buf = 0;

public:
  std::array<Sample, upSample> output;

  void reset() { buf = 0; }

  void process(Sample input)
  {
    Sample diff = input - buf;
    for (size_t i = 0; i < output.size(); ++i) {
      output[i] = buf + diff * Sample(i) / Sample(upSample);
    }
    buf = input;
  }
};

} // namespace SomeDSP
