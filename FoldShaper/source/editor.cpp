// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
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
  refreshCurveView(Synth::ParameterID::ID::guiInputGain);
  return curveView;
}

void Editor::refreshCurveView(ParamID id)
{
  using ID = Synth::ParameterID::ID;

  if (curveView == nullptr) return;
  switch (id) {
    case ID::guiInputGain:
    case ID::inputGain:
    case ID::mul:
    case ID::moreMul:
    case ID::hardclip:
      break;
    default:
      return;
  }

  curveView->inputGain = getPlainValue(ID::inputGain);
  curveView->shaper.multiply = getPlainValue(ID::mul) * getPlainValue(ID::moreMul);
  curveView->shaper.hardclip = getPlainValue(ID::hardclip) > 0;
  curveView->invalid();
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();

  switch (tag) {
    case Synth::ParameterID::ID::limiter:
    case Synth::ParameterID::ID::limiterAttack:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
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

  const auto uiMargin = 3 * margin;
  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Shaper.
  addKnob(left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Input", ID::inputGain);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Mul", ID::mul);
  addKnob<Style::warning>(
    left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "More Mul", ID::moreMul);
  addKnob(left0 + 3 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  const auto checkboxTop = top0 + 2 * margin;
  const auto checkboxLeft = left0 + 4 * knobX + 2 * margin;
  addCheckbox(
    checkboxLeft, checkboxTop, checkboxWidth, labelHeight, uiTextSize, "OverSample",
    ID::oversample);
  addCheckbox(
    checkboxLeft, checkboxTop + labelY - margin, checkboxWidth, labelHeight, uiTextSize,
    "Hardclip", ID::hardclip);

  // Indicator. `curveView` must be instanciated after `infoTextView`.
  const auto leftInfo0 = left0;
  const auto leftInfo1 = left0 + int(0.25 * knobX);
  const auto topInfo0 = top0 + knobY + 4 * margin;
  const auto topInfo1 = topInfo0 + indicatorHeight + 2 * margin;
  if (infoTextView) infoTextView->forget();
  infoTextView = addTextTableView(
    leftInfo1, topInfo1, 2 * infoTextCellWidth, labelHeight, infoTextSize, "",
    infoTextCellWidth);
  infoTextView->remember();

  if (curveView) curveView->forget();
  curveView = addCurveView(
    leftInfo0, topInfo0, indicatorWidth, indicatorHeight, ID::guiInputGain, infoTextView);
  curveView->remember();

  // Limiter.
  const auto leftLimiter0 = left0 + indicatorWidth + 2 * margin;
  const auto leftLimiter1 = leftLimiter0 + limiterLabelWidth;
  const auto topLimiter0 = topInfo0;
  const auto topLimiter1 = topLimiter0 + 1 * labelY;
  const auto topLimiter2 = topLimiter0 + 2 * labelY;
  addToggleButton(
    leftLimiter0, topLimiter0, 2 * limiterLabelWidth, labelHeight, midTextSize, "Limiter",
    ID::limiter);
  addLabel(
    leftLimiter0, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize, "Threshold",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterThreshold, Scales::limiterThreshold, false, 5);
  addLabel(
    leftLimiter0, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, Scales::limiterRelease, false, 5);

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashWidth = 2 * knobX;
  const auto splashLeft = defaultWidth - splashWidth - int(0.25 * knobX) - uiMargin;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, uiMargin, uiMargin,
    defaultWidth - 2 * uiMargin, defaultHeight - 2 * uiMargin, pluginNameTextSize,
    "FoldShaper");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
