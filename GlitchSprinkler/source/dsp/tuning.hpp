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

- ET or Et : Equal temperament.
- MT or Mt : Meantone temperament.
- Just : Just intonation.
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
var cents = [0, 0, 240, 240, 400, 480, 480, 720, 720, 900, 960, 960];
console.log(cents.map(v => 2**(v / 1200)));
```
*/
std::array<double, 12> tuningRatioEt5{
  double(1),                  //  0:    0
  double(1),                  //  1:    0
  double(1.148698354997035),  //  2:  240
  double(1.148698354997035),  //  3:  240
  double(1.2599210498948732), //  4:  400
  double(1.3195079107728942), //  5:  480
  double(1.3195079107728942), //  6:  480
  double(1.515716566510398),  //  7:  720
  double(1.515716566510398),  //  8:  720
  double(1.681792830507429),  //  9:  900
  double(1.7411011265922482), // 10:  960
  double(1.7411011265922482), // 11:  960
};
std::array<size_t, 5> scaleEt5{0, 2, 5, 7, 10};

// cents = [0, 120, 240, 300, 360, 480, 600, 720, 840, 900, 960, 1080];
std::array<double, 12> tuningRatioEt10{
  double(1),                  //  0:    0
  double(1.0717734625362931), //  1:  120
  double(1.148698354997035),  //  2:  240
  double(1.189207115002721),  //  3:  300
  double(1.2311444133449163), //  4:  360
  double(1.3195079107728942), //  5:  480
  double(1.4142135623730951), //  6:  600
  double(1.515716566510398),  //  7:  720
  double(1.624504792712471),  //  8:  840
  double(1.681792830507429),  //  9:  900
  double(1.7411011265922482), // 10:  960
  double(1.8660659830736148), // 11: 1080
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

std::array<double, 12> tuningRatioMtPythagorean{
  double(1) / double(1),     //  0:
  double(256) / double(243), //  1:
  double(9) / double(8),     //  2:
  double(32) / double(27),   //  3:
  double(81) / double(64),   //  4:
  double(4) / double(3),     //  5:
  // double(1024) / double(729), //  6: pairs with 1.
  double(729) / double(512), //  6: pairs with 11.
  double(3) / double(2),     //  7:
  double(128) / double(81),  //  8:
  double(27) / double(16),   //  9:
  double(16) / double(9),    // 10:
  double(243) / double(128), // 11:
};

// Commented numbers indicate pitches in 12 ET semitone.
std::array<double, 12> tuningRatioMtThirdComma{
  double(1.00000000000000), //  0.00000000000000
  double(1.07553713917372), //  1.26068811667588
  double(1.11572158347028), //  1.89572475332965
  double(1.20000000000000), //  3.15641287000552
  double(1.24483465182143), //  3.79144950665930
  double(1.33886590016434), //  5.05213762333518
  // double(1.38888888888889), //  5.68717425998895
  double(1.44000000000000), //  6.31282574001105
  double(1.49380158218572), //  6.94786237666482
  double(1.60663908019721), //  8.20855049334070
  double(1.66666666666667), //  8.84358712999448
  double(1.79256189862287), // 10.10427524667035
  double(1.85953597245047), // 10.73931188332413
};

// Commented numbers indicate pitches in 12 ET semitone.
std::array<double, 12> tuningRatioMtQuarterComma{
  double(1.00000000000000), //  0.00000000000000
  double(1.06998448796228), //  1.17107857668956
  double(1.11803398874989), //  1.93156856932417
  double(1.19627902497698), //  3.10264714601374
  double(1.25000000000000), //  3.86313713864835
  double(1.33748060995284), //  5.03421571533791
  // double(1.39754248593737), //  5.79470570797252
  double(1.43108350559987), //  6.20529429202748
  double(1.49534878122122), //  6.96578428466209
  double(1.60000000000000), //  8.13686286135165
  double(1.67185076244106), //  8.89735285398626
  double(1.78885438199983), // 10.06843143067583
  double(1.86918597652653), // 10.82892142331043
};

} // namespace SomeDSP
