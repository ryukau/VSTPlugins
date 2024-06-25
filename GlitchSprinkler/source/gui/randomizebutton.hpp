// (c) 2023 Takamitsu Endo
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

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "../../../common/gui/plugeditor.hpp"
#include "../../../common/gui/style.hpp"
#include "../parameter.hpp"

#include <algorithm>
#include <random>
#include <string>

namespace VSTGUI {

class RandomizeButton : public CControl {
public:
  std::string label;

  RandomizeButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor)
    : CControl(size, listener, tag)
    , label(label)
    , fontId(fontId)
    , pal(palette)
    , editor(editor)
  {
    if (editor) editor->remember();
  }

  ~RandomizeButton()
  {
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    const double borderW = isMouseEntered ? 2 * borderWidth : borderWidth;
    const double halfBorderWidth = int(borderW / 2.0);
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !isPressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(
      CRect(
        halfBorderWidth, halfBorderWidth, getWidth() - halfBorderWidth,
        getHeight() - halfBorderWidth),
      kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    if (value == 1.0f) {
      value = 0.0f;
    }
    isPressed = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;

    if (!event.buttonState.isLeft()) return;
    isPressed = true;
    value = 1.0f;

    if (editor) {
      using Rng = std::mt19937_64;

      std::random_device source;
      std::random_device::result_type
        random_data[(Rng::state_size - 1) / sizeof(source()) + 1];
      std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
      std::seed_seq seeds(std::begin(random_data), std::end(random_data));
      Rng rng(seeds);

      std::uniform_real_distribution<Steinberg::Vst::ParamValue> uniform{0.0, 1.0};
      setParam(ID::seed, uniform(rng));
    }

    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      invalid();
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0;
      invalid();
    }
    isMouseEntered = false;
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(RandomizeButton, CControl);

private:
  // Before calling check if `editor` is not nullptr.
  inline void setParam(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value)
  {
    editor->valueChanged(id, value);
    editor->updateUI(id, value);
  }

  Steinberg::Vst::PlugEditor *editor = nullptr;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
