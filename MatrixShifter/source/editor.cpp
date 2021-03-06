// (c) 2021 Takamitsu Endo
//
// This file is part of MatrixShifter.
//
// MatrixShifter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixShifter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixShifter.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr float shiftMatrixWidth = nParallel * matrixKnobSize;
constexpr float shiftMatrixHeight = nSerial * matrixKnobSize;
constexpr float shiftBarBoxWidth = 3 * knobX;

constexpr float innerWidth = shiftMatrixWidth + shiftBarBoxWidth + 8 * margin + 5 * knobX;
constexpr float innerHeight = 2 * labelY + shiftMatrixHeight;

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

  // Status bar.
  auto statusBar = addTextView(
    left0, uiMargin + labelY + shiftMatrixHeight + 2 * margin, shiftMatrixWidth,
    labelHeight, uiTextSize, "Hello world.");

  // Shifter matrix.
  addGroupLabel(left0, top0, shiftMatrixWidth, labelHeight, midTextSize, "Pitch Shift");

  std::vector<ParamID> shiftId;
  shiftId.reserve(nShifter);
  for (uint32_t idx = 0; idx < nShifter; ++idx) shiftId.push_back(ID::shiftSemi0 + idx);
  addMatrixKnob(
    left0, top1, shiftMatrixWidth, shiftMatrixHeight, nSerial, nParallel, shiftId,
    statusBar);

  // Barbox.
  constexpr float shiftBarBoxHeight = innerHeight / 2 - 2 * margin;
  const auto barboxLeft0 = left0 + shiftMatrixWidth + 4 * margin;
  const auto barboxTop0 = top0;
  const auto barboxTop1 = barboxTop0 + shiftBarBoxHeight + 2 * margin;
  addBarBox(
    barboxLeft0, barboxTop0, shiftBarBoxWidth, shiftBarBoxHeight, ID::shiftDelay0,
    nSerial, Scales::shiftDelay, "Delay");
  addBarBox(
    barboxLeft0, barboxTop1, shiftBarBoxWidth, shiftBarBoxHeight, ID::shiftGain0, nSerial,
    Scales::shiftGain, "Gain");

  // Other controls.
  const auto miscLeft0 = left0 + shiftMatrixWidth + shiftBarBoxWidth + 8 * margin;
  const auto miscLeft1 = miscLeft0 + knobX;
  const auto miscLeft2 = miscLeft1 + knobX;
  const auto miscLeft3 = miscLeft2 + knobX;
  const auto miscLeft4 = miscLeft3 + knobX;

  const auto miscTop0 = top0;
  const auto miscTop1 = miscTop0 + knobY;
  const auto miscTop2 = miscTop1 + knobY;
  const auto miscTop3 = miscTop2 + labelY;

  addKnob(miscLeft0, miscTop0, knobWidth, margin, uiTextSize, "LFO Freq", ID::lfoHz);
  addKnob(miscLeft1, miscTop0, knobWidth, margin, uiTextSize, "LFO Amt.", ID::lfoAmount);
  addKnob(miscLeft2, miscTop0, knobWidth, margin, uiTextSize, "LFO Skew", ID::lfoSkew);
  addKnob(
    miscLeft3, miscTop0, knobWidth, margin, uiTextSize, "LFO Shift", ID::lfoShiftOffset);
  addKnob(
    miscLeft4, miscTop0, knobWidth, margin, uiTextSize, "LFO fb.Cut",
    ID::lfoToFeedbackCutoff);

  addKnob(miscLeft0, miscTop1, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(miscLeft1, miscTop1, knobWidth, margin, uiTextSize, "Mix", ID::shiftMix);
  addKnob(miscLeft2, miscTop1, knobWidth, margin, uiTextSize, "Phase", ID::shiftPhase);
  addKnob(
    miscLeft3, miscTop1, knobWidth, margin, uiTextSize, "Feedback",
    ID::shiftFeedbackGain);

  addLabel(miscLeft0, miscTop2, knobX, labelHeight, uiTextSize, "Shift*");
  addTextKnob(
    miscLeft1, miscTop2, knobX, labelHeight, uiTextSize, ID::shiftSemiMultiplier,
    Scales::shiftSemiMultiplier, false, 5, 0);

  addCheckbox(
    miscLeft2, miscTop2, 2 * knobX, labelHeight, uiTextSize, "InvertEachSection",
    ID::invertEachSection);

  addLabel(miscLeft0, miscTop3, knobX, labelHeight, uiTextSize, "Smooth [s]");
  addTextKnob(
    miscLeft1, miscTop3, knobX, labelHeight, uiTextSize, ID::smoothness,
    Scales::smoothness, false, 3, 0);

  addLabel(miscLeft2, miscTop3, knobX, labelHeight, uiTextSize, "fb.Cut");
  addTextKnob(
    miscLeft3, miscTop3, knobX, labelHeight, uiTextSize, ID::shiftFeedbackCutoff,
    Scales::shiftFeedbackCutoff, false, 2, 0);

  // Plugin name.
  const auto splashTop = uiMargin + innerHeight - splashHeight;
  const auto splashLeft = miscLeft0;
  addSplashScreen(
    splashLeft, splashTop, 2 * knobX, splashHeight, uiMargin, uiMargin, innerWidth,
    innerHeight, pluginNameTextSize, "MatrixShifter");

  return true;
} // namespace Vst

} // namespace Vst
} // namespace Steinberg
