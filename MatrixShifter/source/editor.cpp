// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 12.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float matrixKnobSize = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float splashHeight = labelHeight;

constexpr float halfKnobX = int(knobX / 2);

constexpr float shiftMatrixWidth = nParallel * matrixKnobSize;
constexpr float shiftMatrixHeight = nSerial * matrixKnobSize;
constexpr float shiftBarBoxWidth = shiftMatrixWidth;
constexpr float shiftBarBoxHeight = 2 * knobY;

constexpr float innerWidth = shiftMatrixWidth + 8 * margin + 4 * knobX;
constexpr float innerHeight
  = labelY + shiftMatrixHeight + 2 * shiftBarBoxHeight + 4 * margin;

constexpr uint32_t defaultWidth = uint32_t(2 * uiMargin + innerWidth);
constexpr uint32_t defaultHeight = uint32_t(2 * uiMargin + innerHeight);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  const auto top1 = top0 + labelY;
  const auto top2 = top1 + shiftMatrixHeight + 2 * margin;

  // Shifter matrix.
  addGroupLabel(left0, top0, shiftMatrixWidth, labelHeight, midTextSize, "Pitch Shift");

  std::vector<ParamID> shiftId;
  shiftId.reserve(nShifter);
  for (uint32_t idx = 0; idx < nShifter; ++idx) shiftId.push_back(ID::shiftSemi0 + idx);
  addMatrixKnob(
    left0, top1, shiftMatrixWidth, shiftMatrixHeight, nSerial, nParallel, shiftId);

  // Barbox.
  const auto barboxLeft0 = left0;
  const auto barboxTop0 = top2;
  const auto barboxTop1 = barboxTop0 + shiftBarBoxHeight + 2 * margin;
  addBarBox(
    barboxLeft0, barboxTop0, shiftBarBoxWidth, shiftBarBoxHeight, ID::shiftDelay0,
    nSerial, Scales::shiftDelay, "Delay");
  addBarBox(
    barboxLeft0, barboxTop1, shiftBarBoxWidth, shiftBarBoxHeight, ID::shiftGain0, nSerial,
    Scales::shiftGain, "Gain");

  // Other controls.
  const auto miscLeft0 = left0 + shiftMatrixWidth + 8 * margin;
  const auto miscLeft1 = miscLeft0 + knobX;
  const auto miscLeft2 = miscLeft1 + knobX;
  const auto miscLeft3 = miscLeft2 + knobX;
  const auto miscLeft4 = miscLeft3 + knobX;

  // Modulation.
  const auto modulationTop0 = top0;
  const auto modulationTop1 = modulationTop0 + labelY;
  const auto modulationTop2 = modulationTop1 + labelY;
  const auto modulationTop3 = modulationTop2 + labelY;

  const auto modulationLeft1 = miscLeft1 + halfKnobX;

  addGroupLabel(
    miscLeft0, modulationTop0, 4 * knobX, labelHeight, uiTextSize, "Modulation");

  addKnob(
    miscLeft0 + 3 * margin, modulationTop1, knobWidth, margin, uiTextSize, "Feedback",
    ID::shiftFeedbackGain);

  addLabel(
    modulationLeft1, modulationTop1, 2 * knobX, labelHeight, uiTextSize, "Feed Cutoff",
    CHoriTxtAlign::kLeftText);
  addTextKnob(
    miscLeft3, modulationTop1, knobX, labelHeight, uiTextSize, ID::shiftFeedbackCutoff,
    Scales::shiftFeedbackCutoff, false, 2, 0);

  addLabel(
    modulationLeft1, modulationTop2, 2 * knobX, labelHeight, uiTextSize,
    "Shift Multiplier", CHoriTxtAlign::kLeftText);
  addTextKnob(
    miscLeft3, modulationTop2, knobX, labelHeight, uiTextSize, ID::shiftSemiMultiplier,
    Scales::shiftSemiMultiplier, false, 5, 0);

  addLabel(
    modulationLeft1, modulationTop3, 2 * knobX, labelHeight, uiTextSize, "Flip Sign",
    CHoriTxtAlign::kLeftText);
  addCheckbox(
    miscLeft3 + halfKnobX - margin, modulationTop3, knobX, labelHeight, uiTextSize, "",
    ID::invertEachSection);

  // LFO.
  const auto lfoTop0 = modulationTop0 + 4 * labelY;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + knobY;
  const auto lfoTop3 = lfoTop2 + knobY;

  addGroupLabel(miscLeft0, lfoTop0, 4 * knobX, labelHeight, uiTextSize, "LFO");

  addToggleButton(
    miscLeft0, lfoTop1, 2 * knobX, labelHeight, uiTextSize, "Tempo Sync",
    ID::lfoTempoSync);
  addTextKnob(
    miscLeft0 + margin, lfoTop1 + labelHeight + margin, knobX - 2 * margin, labelHeight,
    uiTextSize, ID::lfoSyncUpper, Scales::lfoSync, false, 0, 1);
  addTextKnob(
    miscLeft0 + margin, lfoTop1 + 2 * labelHeight + margin, knobX - 2 * margin,
    labelHeight, uiTextSize, ID::lfoSyncLower, Scales::lfoSync, false, 0, 1);
  addLabel(
    miscLeft1, lfoTop1 + int(1.5f * labelHeight) + margin, knobX, labelHeight, uiTextSize,
    "Beat", CHoriTxtAlign::kLeftText);

  addKnob(miscLeft3, lfoTop1, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate);
  addKnob(
    miscLeft2, lfoTop2, knobWidth, margin, uiTextSize, "L/R Offset",
    ID::lfoLrPhaseOffset);
  addKnob(miscLeft3, lfoTop2, knobWidth, margin, uiTextSize, "Shape", ID::lfoSkew);
  addKnob(miscLeft1, lfoTop3, knobWidth, margin, uiTextSize, "To Delay", ID::lfoToDelay);
  addKnob(
    miscLeft2, lfoTop3, knobWidth, margin, uiTextSize, "To Shifter", ID::lfoToPitchShift);
  addKnob(
    miscLeft3, lfoTop3, knobWidth, margin, uiTextSize, "To Filter",
    ID::lfoToFeedbackCutoff);

  // Mix.
  const auto mixTop0 = lfoTop0 + labelY + 3 * knobY + 2 * margin;
  const auto mixTop1 = mixTop0 + labelY;

  addGroupLabel(miscLeft0, mixTop0, 4 * knobX, labelHeight, uiTextSize, "Mix");

  addKnob(miscLeft0, mixTop1, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(miscLeft1, mixTop1, knobWidth, margin, uiTextSize, "Mix", ID::shiftMix);

  addLabel(miscLeft3, mixTop1, knobX, labelHeight, uiTextSize, "Smooth [s]");
  addTextKnob(
    miscLeft3, mixTop1 + labelY, knobX, labelHeight, uiTextSize, ID::smoothness,
    Scales::smoothness, false, 3, 0);

  // Plugin name.
  const auto splashTop = uiMargin + innerHeight - splashHeight;
  const auto splashLeft = miscLeft2;
  addSplashScreen(
    splashLeft, splashTop, 2 * knobX, splashHeight, uiMargin, uiMargin, innerWidth,
    innerHeight, pluginNameTextSize, "MatrixShifter");

  return true;
} // namespace Vst

} // namespace Vst
} // namespace Steinberg
