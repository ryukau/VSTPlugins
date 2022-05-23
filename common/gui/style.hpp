// (c) 2020-2021 Takamitsu Endo
// (c) 2020 Konstantin Voinov
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

#include "vstgui4/vstgui/lib/ccolor.h"
#include "vstgui4/vstgui/lib/cfont.h"
#include "vstgui4/vstgui/lib/cstring.h"

namespace Uhhyou {

// Using common because default is a keyword in C++.
enum class Style { common, accent, warning };

class Palette {
public:
  Palette() { load(); }
  void load();

  const VSTGUI::UTF8String &fontName() { return _fontName; }
  const int &fontFace() { return _fontFace; }
  const VSTGUI::CColor &foreground() { return _foreground; }
  const VSTGUI::CColor &foregroundButtonOn() { return _foregroundButtonOn; }
  const VSTGUI::CColor &foregroundInactive() { return _foregroundInactive; }
  const VSTGUI::CColor &background() { return _background; }
  const VSTGUI::CColor &boxBackground() { return _boxBackground; }
  const VSTGUI::CColor &border() { return _border; }
  const VSTGUI::CColor &borderCheckbox() { return _borderCheckbox; }
  const VSTGUI::CColor &borderLabel() { return _borderLabel; }
  const VSTGUI::CColor &unfocused() { return _unfocused; }
  const VSTGUI::CColor &highlightMain() { return _highlightMain; }
  const VSTGUI::CColor &highlightAccent() { return _highlightAccent; }
  const VSTGUI::CColor &highlightButton() { return _highlightButton; }
  const VSTGUI::CColor &highlightWarning() { return _highlightWarning; }
  const VSTGUI::CColor &overlay() { return _overlay; }
  const VSTGUI::CColor &overlayHighlight() { return _overlayHighlight; }
  const VSTGUI::CColor &overlayFaint() { return _overlayFaint; }

private:
  VSTGUI::UTF8String _fontName{"Tinos"};
  int _fontFace{VSTGUI::CTxtFace::kBoldFace | VSTGUI::CTxtFace::kItalicFace};
  VSTGUI::CColor _foreground{0x00, 0x00, 0x00};
  VSTGUI::CColor _foregroundButtonOn{0x00, 0x00, 0x00};
  VSTGUI::CColor _foregroundInactive{0x00, 0x00, 0x00};
  VSTGUI::CColor _background{0xff, 0xff, 0xff};
  VSTGUI::CColor _boxBackground{0xff, 0xff, 0xff};
  VSTGUI::CColor _border{0x00, 0x00, 0x00};
  VSTGUI::CColor _borderCheckbox{0x00, 0x00, 0x00};
  VSTGUI::CColor _borderLabel{0x00, 0x00, 0x00};
  VSTGUI::CColor _unfocused{0xdd, 0xdd, 0xdd};
  VSTGUI::CColor _highlightMain{0x0b, 0xa4, 0xf1};
  VSTGUI::CColor _highlightAccent{0x13, 0xc1, 0x36};
  VSTGUI::CColor _highlightButton{0xfc, 0xc0, 0x4f};
  VSTGUI::CColor _highlightWarning{0xfc, 0x80, 0x80};
  VSTGUI::CColor _overlay{0x00, 0x00, 0x00, 0x88};
  VSTGUI::CColor _overlayHighlight{0x00, 0xff, 0x00, 0x33};
  VSTGUI::CColor _overlayFaint{0x00, 0x00, 0x00, 0x0b};
};

} // namespace Uhhyou
