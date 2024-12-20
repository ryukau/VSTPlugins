// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float checkboxWidth = 60.0f;
constexpr float splashHeight = 30.0f;
constexpr float indicatorHeight = 3 * labelY - 2 * margin;
constexpr float indicatorWidth = 3 * knobX - margin;
constexpr float infoTextSize = 12.0f;
constexpr float infoTextCellWidth = 100.0f;

constexpr float limiterLabelWidth = knobX + 3 * margin;

constexpr uint32_t defaultWidth = uint32_t(6 * knobX + 30);
constexpr uint32_t defaultHeight
  = uint32_t(2 * knobY + 6 * margin + 3 * labelY + splashHeight + 30);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
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
    case ID::add:
    case ID::mul:
    case ID::moreAdd:
    case ID::moreMul:
    case ID::hardclip:
      break;
    default:
      return;
  }

  curveView->inputGain = getPlainValue(ID::inputGain);
  curveView->shaper.add = getPlainValue(ID::add) * getPlainValue(ID::moreAdd);
  curveView->shaper.mul = getPlainValue(ID::mul) * getPlainValue(ID::moreMul);
  curveView->shaper.hardclip = getPlainValue(ID::hardclip) > 0;
  curveView->invalid();
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();

  switch (tag) {
    case Synth::ParameterID::ID::type:
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

  const auto top0 = 15.0f;
  const auto top1 = top0 + knobY + 2 * margin;
  const auto top2 = top1 + knobY + 2 * margin;
  const auto left0 = 15.0f;

  // Indicator. `curveView` must be instanciated after `infoTextView`.
  const auto marginInfo = std::floor(0.25f * knobX);
  const auto leftInfo0 = left0;
  const auto leftInfo1 = leftInfo0 + marginInfo;
  const auto topInfo0 = top2 + 2 * margin;
  const auto topInfo1 = topInfo0 + indicatorHeight + margin;
  if (infoTextView) infoTextView->forget();
  infoTextView = addTextTableView(
    leftInfo1, topInfo1, 2 * infoTextCellWidth, labelHeight, infoTextSize, "",
    infoTextCellWidth);
  infoTextView->remember();

  addTextView(
    leftInfo1, topInfo1 + labelHeight, 2 * infoTextCellWidth, labelHeight, infoTextSize,
    "Blue line is 0.");

  if (curveView) curveView->forget();
  curveView = addCurveView(
    leftInfo0, topInfo0, indicatorWidth, indicatorHeight, ID::guiInputGain, infoTextView);
  curveView->remember();

  // Shaper.
  addKnob(
    left0 + 0 * knobX, top0, knobX, margin, uiTextSize, "Cutoff", ID::lowpassCutoff);
  addKnob(left0 + 1 * knobX, top0, knobX, margin, uiTextSize, "Input", ID::inputGain);
  addKnob(left0 + 2 * knobX, top0, knobX, margin, uiTextSize, "Add", ID::add);
  addKnob(left0 + 3 * knobX, top0, knobX, margin, uiTextSize, "Mul", ID::mul);
  addKnob(left0 + 4 * knobX, top0, knobX, margin, uiTextSize, "Pre-Clip", ID::clipGain);
  addKnob(left0 + 5 * knobX, top0, knobX, margin, uiTextSize, "Output", ID::outputGain);

  addKnob<Style::warning>(
    left0 + 2 * knobX, top1, knobX, margin, uiTextSize, "More Add", ID::moreAdd);
  addKnob<Style::warning>(
    left0 + 3 * knobX, top1, knobX, margin, uiTextSize, "More Mul", ID::moreMul);

  const auto top1Checkbox = top1 + floor(2.5 * margin);
  const auto leftCheckbox = left0 + 4 * knobX + 2 * margin;
  addCheckbox(
    leftCheckbox, top1Checkbox, 1.5f * knobX, labelHeight, uiTextSize, "Hardclip",
    ID::hardclip);

  addCheckbox(
    leftCheckbox, top1Checkbox + labelY, 1.5f * knobX, labelHeight, uiTextSize, "Lowpass",
    ID::lowpass);

  const auto antiAliasingWidth = 2.0f * knobX - 3.0f * margin;
  const auto antiAliasingLeft = left0 + margin;
  addLabel(
    antiAliasingLeft, top1Checkbox, antiAliasingWidth, labelHeight, uiTextSize,
    "Anti-aliasing");
  std::vector<std::string> typeItems{
    "None", "16x OverSampling", "PolyBLEP 4", "PolyBLEP 8"};
  addOptionMenu(
    antiAliasingLeft, top1Checkbox + labelHeight + margin, antiAliasingWidth, labelHeight,
    uiTextSize, ID::type, typeItems);

  // Limiter.
  const auto leftLimiter0 = left0 + std::floor(3.25f * knobX);
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
  const auto splashTop = defaultHeight - splashHeight - 15.0f;
  const auto splashWidth = 2 * knobX - 2 * margin;
  const auto splashLeft = defaultWidth + 2 * margin - std::floor(2.5f * knobX) - 15.0f;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, 15.0f, 15.0f, defaultWidth - 30.0f,
    defaultHeight - 30.0f, pluginNameTextSize, "ModuloShaper");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
