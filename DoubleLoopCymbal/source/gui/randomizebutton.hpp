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
    using Scales = Steinberg::Synth::Scales;
    using Value = Steinberg::Vst::ParamValue;

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

      std::uniform_real_distribution<Value> uniform{Value(0), Value(1)};
      setParam(ID::seed, uniform(rng));

      std::uniform_real_distribution<Value> highpassDist{Value(20), Value(80)};
      const auto &hpScale = Scales::halfClosedDensityHz;
      setParam(ID::impactHighpassHz, hpScale.invmapDB(highpassDist(rng)));
      setParam(ID::halfClosedHighpassHz, hpScale.invmapDB(highpassDist(rng)));
      setParam(ID::closingHighpassHz, hpScale.invmapDB(highpassDist(rng)));

      std::uniform_real_distribution<Value> halfClosedDecaySecondDist{
        Value(-20), Value(20)};
      setParam(
        ID::halfClosedDecaySecond,
        Scales::noiseDecaySeconds.invmapDB(halfClosedDecaySecondDist(rng)));
      std::uniform_real_distribution<Value> halfClosedSustainLevelDist{
        Value(-40), Value(0)};
      setParam(
        ID::halfClosedSustainLevel,
        Scales::noiseGain.invmapDB(halfClosedSustainLevelDist(rng)));
      std::uniform_real_distribution<Value> halfClosedPulseSecondDist{
        Value(-80), Value(0)};
      setParam(
        ID::halfClosedPulseSecond,
        Scales::noiseDecaySeconds.invmapDB(halfClosedPulseSecondDist(rng)));
      std::uniform_real_distribution<Value> halfClosedDensityHzDist{Value(0), Value(32)};
      setParam(
        ID::halfClosedDensityHz,
        Scales::halfClosedDensityHz.invmapDB(halfClosedDensityHzDist(rng)));

      std::uniform_real_distribution<Value> allpassMixSpikeDist{Value(0.333), Value(1)};
      setParam(ID::allpassMixSpike, allpassMixSpikeDist(rng));
      setParam(ID::allpassMixAltSign, uniform(rng));

      std::uniform_real_distribution<Value> highShelfFrequencyHzDist{
        Scales::cutoffFrequencyHz.invmap(4000), Scales::cutoffFrequencyHz.invmap(20000)};
      setParam(ID::highShelfFrequencyHz, highShelfFrequencyHzDist(rng));
      std::uniform_real_distribution<Value> highShelfGainDist{
        Scales::shelvingGain.invmapDB(-1), Scales::shelvingGain.invmapDB(0)};
      setParam(ID::highShelfGain, highShelfGainDist(rng));
      std::uniform_real_distribution<Value> lowShelfFrequencyHzDist{Value(20), Value(86)};
      setParam(
        ID::lowShelfFrequencyHz,
        Scales::cutoffFrequencyHz.invmapDB(lowShelfFrequencyHzDist(rng)));
      std::uniform_real_distribution<Value> lowShelfGainDist{Value(-6), Value(0)};
      setParam(ID::lowShelfGain, Scales::shelvingGain.invmapDB(lowShelfGainDist(rng)));
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
