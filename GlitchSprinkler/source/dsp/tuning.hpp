// (c) 2024 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

/*
This file can be shorter if the compiler supports C++26. C++26 provides constexpr
everywhere for <cmath>.
*/

#pragma once

#include <array>

namespace SomeDSP {

// `for (let i = 0; i < 12; ++i) console.log(2**(i / 12));`
std::array<double, 12> tuningRatioEt12{
  double(1.00000000000000),   double(1.0594630943592953), double(1.122462048309373),
  double(1.189207115002721),  double(1.2599210498948732), double(1.3348398541700344),
  double(1.4142135623730951), double(1.4983070768766815), double(1.5874010519681994),
  double(1.681792830507429),  double(1.7817974362806785), double(1.887748625363387),
};
std::array<size_t, 7> scaleEt12ChurchC{0, 2, 4, 5, 7, 9, 11}; // Ionic, Major
std::array<size_t, 7> scaleEt12ChurchD{0, 2, 3, 5, 7, 9, 10}; // Dorian
std::array<size_t, 7> scaleEt12ChurchE{0, 1, 3, 5, 7, 8, 10}; // Frygian
std::array<size_t, 7> scaleEt12ChurchF{0, 2, 4, 6, 7, 9, 11}; // Lydian
std::array<size_t, 7> scaleEt12ChurchG{0, 2, 4, 5, 7, 9, 10}; // Mixolydian
std::array<size_t, 7> scaleEt12ChurchA{0, 2, 3, 5, 7, 8, 10}; // Aeolian, Minor
std::array<size_t, 7> scaleEt12ChurchB{0, 1, 3, 5, 6, 8, 10}; // Locrian
std::array<size_t, 3> scaleEt12Sus2{0, 2, 7};
std::array<size_t, 3> scaleEt12Sus4{0, 5, 7};
std::array<size_t, 6> scaleEt12Chord2_4_7_9_11{0, 2, 4, 7, 9, 11};
std::array<size_t, 4> scaleEt12Maj7{0, 4, 7, 11};
std::array<size_t, 4> scaleEt12Min7{0, 3, 7, 10};
std::array<size_t, 8> scaleEt12MajExtended{0, 4, 7, 11, 14, 18, 21, 25};
std::array<size_t, 8> scaleEt12MinExtended{0, 3, 7, 10, 14, 17, 21, 24};
std::array<size_t, 6> scaleEt12WholeTone2{0, 2, 4, 6, 8, 10};
std::array<size_t, 4> scaleEt12WholeTone3{0, 3, 6, 9};
std::array<size_t, 3> scaleEt12WholeTone4{0, 4, 8};
std::array<size_t, 6> scaleEt12Blues{0, 3, 5, 6, 7, 10};

/**
```javascript
var cents = [0, 1, 240, 241, 400, 480, 481, 720, 721, 900, 960, 961];
console.log(cents.map(v => 2**(v / 1200)));
```
*/
std::array<double, 12> tuningRatioEt5{
  double(1),                  //  0:    0 (exact)
  double(1.0005777895065548), //  1:    1
  double(1.148698354997035),  //  2:  240 (exact)
  double(1.149362060852749),  //  3:  241
  double(1.2599210498948732), //  4:  400
  double(1.3195079107728942), //  5:  480 (exact)
  double(1.320270308597555),  //  6:  481
  double(1.515716566510398),  //  7:  720 (exact)
  double(1.5165923316374392), //  8:  721
  double(1.681792830507429),  //  9:  900
  double(1.7411011265922482), // 10:  960 (exact)
  double(1.742107116553044),  // 11:  961
};
std::array<size_t, 5> scaleEt5{0, 2, 5, 7, 10};

// cents = [0, 120, 240, 300, 360, 480, 600, 720, 840, 960, 1020, 1080];
std::array<double, 12> tuningRatioEt10Minor{
  double(1.0000000000000),    double(1.0717734625362931), double(1.148698354997035),
  double(1.189207115002721),  double(1.2311444133449163), double(1.3195079107728942),
  double(1.4142135623730951), double(1.515716566510398),  double(1.624504792712471),
  double(1.7411011265922482), double(1.8025009252216604), double(1.8660659830736148),
};

std::array<double, 12> tuningRatioJust5Major{
  double(1) / double(1),   double(16) / double(15), double(9) / double(8),
  double(6) / double(5),   double(5) / double(4),   double(4) / double(3),
  double(45) / double(32), double(3) / double(2),   double(8) / double(5),
  double(5) / double(3),   double(16) / double(9),  double(15) / double(8),
};

std::array<double, 12> tuningRatioJust5Minor{
  double(1) / double(1),   double(16) / double(15), double(10) / double(9),
  double(6) / double(5),   double(5) / double(4),   double(4) / double(3),
  double(64) / double(45), double(3) / double(2),   double(8) / double(5),
  double(5) / double(3),   double(9) / double(5),   double(15) / double(8),
};

std::array<double, 13> tuningRatioJust7{
  double(1) / double(1), double(8) / double(7),  double(7) / double(6),
  double(6) / double(5), double(5) / double(4),  double(4) / double(3),
  double(7) / double(5), double(10) / double(7), double(3) / double(2),
  double(8) / double(5), double(5) / double(3),  double(12) / double(7),
  double(7) / double(4),
};

} // namespace SomeDSP
