// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

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
  std::vector<Steinberg::Vst::ParamID> targetId;

  RandomizeButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor,
    std::vector<Steinberg::Vst::ParamID> targetId)
    : CControl(size, listener, tag), pal(palette), editor(editor), targetId(targetId)
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
    auto borderW = int(pal.guiScale() * borderWidth * (isMouseEntered ? 2 : 1));
    borderW += 1 ^ (borderW % 2); // Always odd number.
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !isPressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(CRect(0, 0, getWidth(), getHeight()), kDrawFilledAndStroked);
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
      for (const auto &id : targetId) setParam(id, uniform(rng));
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

  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
