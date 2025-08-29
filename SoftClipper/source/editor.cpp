// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "gui/curveview.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
}

Editor::~Editor()
{
  if (curveView) curveView->forget();
  if (infoTextView) infoTextView->forget();
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

CurveView<decltype(Synth::Scales::guiInputGainScale)> *Editor::addCurveView(
  CCoord left,
  CCoord top,
  CCoord width,
  CCoord height,
  ParamID id,
  TextTableView *textView)
{
  auto curveView = new CurveView(
    CRect(left, top, left + width, top + height), palette,
    Synth::Scales::guiInputGainScale, textView);
  curveView->setValueNormalized(controller->getParamNormalized(id));
  curveView->setDefaultValue(param->getDefaultNormalized(id));
  frame->addView(curveView);
  addToControlMap(id, curveView);
  refreshCurveView(Synth::ParameterID::ID::clip);
  return curveView;
}

void Editor::refreshCurveView(ParamID id)
{
  using ID = Synth::ParameterID::ID;

  if (curveView == nullptr) return;
  switch (id) {
    case ID::guiInputGain:
    case ID::orderFraction:
    case ID::orderInteger:
    case ID::clip:
    case ID::ratio:
    case ID::slope:
    case ID::inputGain:
      break;
    default:
      return;
  }
  curveView->inputGain = getPlainValue(ID::inputGain);
  curveView->clipper.set(
    getPlainValue(ID::clip),
    getPlainValue(ID::orderFraction) + getPlainValue(ID::orderInteger),
    getPlainValue(ID::ratio), getPlainValue(ID::slope));
  curveView->invalid();
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);

  refreshCurveView(id);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);

  refreshCurveView(id);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  addKnob(left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Input", ID::inputGain);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Clip", ID::clip);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Ratio", ID::ratio);
  addKnob(left0 + 3 * knobX, top0, knobX, margin, uiTextSize, "Slope", ID::slope);
  addKnob(left0 + 4 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto top1 = top0 + knobY + labelHeight;

  // Indicator. `curveView` must be instanciated after `infoTextView`.
  if (infoTextView) infoTextView->forget();
  infoTextView = addTextTableView(
    left0 + 2 * margin + indicatorHeight, top1, infoTextCellWidth * 3, indicatorHeight,
    infoTextSize, "", infoTextCellWidth);
  infoTextView->remember();

  if (curveView) curveView->forget();
  curveView = addCurveView(
    left0, top1, indicatorHeight, indicatorHeight, ID::guiInputGain, infoTextView);
  curveView->remember();

  // Clipper.
  const auto left1 = left0 + 5 * knobX + 4 * margin;
  const auto left2 = left1 + knobX + 6 * margin;

  addLabel(
    left1, top0, int(1.5 * knobX), labelHeight, uiTextSize, "Order Integer", kLeftText);
  addTextKnob(
    left2, top0, knobX, labelHeight, uiTextSize, ID::orderInteger, Scales::orderInteger);

  addLabel(
    left1, top0 + labelY, int(1.5 * knobX), labelHeight, uiTextSize, "Order Fraction",
    kLeftText);
  addTextKnob(
    left2, top0 + labelY, knobX, labelHeight, uiTextSize, ID::orderFraction,
    Scales::defaultScale, false, 4);

  addCheckbox(
    left1, top0 + 2 * labelY, int(1.5 * knobX), labelHeight, uiTextSize, "OverSample",
    ID::oversample);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashWidth = 2 * knobX - 2 * margin;
  const auto splashLeft = defaultWidth - splashWidth - uiMargin;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, uiMargin, uiMargin,
    defaultWidth - 2 * uiMargin, defaultHeight - 2 * uiMargin, pluginNameTextSize,
    "SoftClipper");

  return true;
}

} // namespace Vst
} // namespace Steinberg
