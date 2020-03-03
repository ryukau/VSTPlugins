// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "vstgui/vstgui.h"

#include "arraycontrol.hpp"

#include <algorithm>
#include <random>

namespace VSTGUI {

class BarBox : public ArrayControl {
public:
  Steinberg::IPlugView *editor = nullptr;
  bool drawCenterLine = false;

  BarBox(
    Steinberg::Vst::EditController *controller,
    const CRect &size,
    Steinberg::IPlugView *editor,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue)
    : ArrayControl(controller, size, id, value, defaultValue)
    , editor(editor)
    , sliderWidth((size.right - size.left) / value.size())
  {
    setWantsFocus(true);
    // editor->remember();
  }

  ~BarBox()
  {
    // if (editor) editor->forget();
    if (indexFontID) indexFontID->forget();
    if (nameFontID) nameFontID->forget();
  }

  void draw(CDrawContext *pContext) override;

  CLASS_METHODS(BarBox, CView);

  CMouseEventResult onMouseEntered(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseExited(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
  CMouseEventResult onMouseCancel() override;
  bool onWheel(
    const CPoint &where,
    const CMouseWheelAxis &axis,
    const float &distance,
    const CButtonState &buttons) override;
  int32_t onKeyDown(VstKeyCode &key) override;

  void setIndexFont(CFontRef fontID) { indexFontID = fontID; }
  void setNameFont(CFontRef fontID) { nameFontID = fontID; }
  void setHighlightColor(CColor color) { highlightColor = color; }
  void setValueColor(CColor color) { valueColor = color; }
  void setBorderColor(CColor color) { borderColor = color; }
  void setDefaultBorderWidth(float width) { defaultBorderWidth = width; }
  void setName(std::string name) { this->name = name; }

protected:
  void averageLowpass(size_t start);
  void highpass(size_t start);
  void randomize(size_t start, double mix);
  void sparseRandomize(size_t start);
  void permute(size_t start);
  void invert(size_t start, bool preserveMin);
  void normalize(size_t start, bool preserveMin) noexcept;
  void multiplySkip(size_t start, size_t interval) noexcept;
  void emphasizeLow(size_t start);
  void emphasizeHigh(size_t start);

  void setValueFromPosition(CPoint &position, const CButtonState &buttons);
  void setValueFromLine(CPoint p0, CPoint p1);
  void setMousePosition(CPoint &where);
  inline size_t calcIndex(CPoint position);

  CColor highlightColor{0x00, 0xff, 0x00, 0x33};
  CColor splitterColor{0xff, 0xff, 0xff};
  CColor valueColor{0xdd, 0xdd, 0xdd};
  CColor borderColor{0, 0, 0};
  CColor nameColor{0, 0, 0, 0x88};

  CFontRef indexFontID = nullptr;
  CFontRef nameFontID = nullptr;

  CCoord defaultBorderWidth = 2.0;

  bool isMouseLeftDown = false;
  bool isMouseMiddleDown = false;
  bool isMouseEntered = false;

  CPoint mousePosition{-1.0, -1.0};
  CPoint anchor{-1.0, -1.0};
  CCoord sliderWidth;

  std::string name;
};

} // namespace VSTGUI
