// (c) 2022 Takamitsu Endo
//
// This file is part of BasicLimiterAutoMake.
//
// BasicLimiterAutoMake is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiterAutoMake is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiterAutoMake.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float splashHeight = 30.0f;

constexpr float limiterLabelWidth = 100.0f;
constexpr float checkboxWidth = 2.0f * limiterLabelWidth;

constexpr uint32_t defaultWidth = uint32_t(2 * uiMargin + 2 * limiterLabelWidth);
constexpr uint32_t defaultHeight = uint32_t(2 * uiMargin + 10 * labelY + splashHeight);

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
  if (infoTextView) infoTextView->forget();
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

template<typename T> inline T ampToDecibel(T x) { return T(20) * std::log10(x); }

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();

  switch (id) {
    case Synth::ParameterID::ID::limiterAttack:
    case Synth::ParameterID::ID::truePeak:
      controller->getComponentHandler()->restartComponent(kLatencyChanged);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);

  // Refresh infoTextView.
  using ID = Synth::ParameterID::ID;

  if (id == ID::limiterAttack) {
    controller->performEdit(ID::overshoot, 0.0);
    return;
  }

  if (infoTextView == nullptr) return;
  if (id != ID::overshoot) return;

  auto &&rdlm = TextTableView::rowDelimiter;
  auto &&cdlm = TextTableView::colDelimiter;
  auto overshoot = ampToDecibel(getPlainValue(ID::overshoot));
  if (overshoot > 0 && overshoot < 1e-5f) overshoot = 1e-5f;
  std::ostringstream os;
  os.precision(5);
  os << std::fixed << "Overshoot [dB]" << cdlm << overshoot << rdlm;
  infoTextView->setText(os.str());
  infoTextView->setDirty();
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Limiter.
  const auto leftLimiter0 = left0;
  const auto leftLimiter1 = leftLimiter0 + limiterLabelWidth;
  const auto topLimiter01 = top0;
  const auto topLimiter02 = top0 + 1 * labelY;
  const auto topLimiter03 = top0 + 2 * labelY;
  const auto topLimiter04 = top0 + 3 * labelY;
  const auto topLimiter05 = top0 + 4 * labelY;
  const auto topLimiter06 = top0 + 5 * labelY;
  const auto topLimiter07 = top0 + 6 * labelY;
  const auto topLimiter08 = top0 + 7 * labelY;
  const auto topLimiter09 = top0 + 8 * labelY;
  const auto topLimiter10 = top0 + 9 * labelY;

  addLabel(
    leftLimiter0, topLimiter01, limiterLabelWidth, labelHeight, uiTextSize,
    "Threshold [dB]", kLeftText);
  auto thresholdKnob = addTextKnob(
    leftLimiter1, topLimiter01, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterThreshold, Scales::limiterThreshold, true, 5);
  if (thresholdKnob) {
    thresholdKnob->sensitivity = 0.002f / 6.0f;
    thresholdKnob->lowSensitivity = 0.002f / 120.0f;
  }
  addCheckbox(
    leftLimiter0, topLimiter02, checkboxWidth, labelHeight, uiTextSize, "Auto Make Up",
    ID::autoMakeupToggle);
  auto makeupTargetKnob = addTextKnob(
    leftLimiter1, topLimiter02, limiterLabelWidth, labelHeight, uiTextSize,
    ID::autoMakeupTargetGain, Scales::limiterThreshold, true, 5);
  if (makeupTargetKnob) {
    makeupTargetKnob->sensitivity = 0.002f / 6.0f;
    makeupTargetKnob->lowSensitivity = 0.002f / 120.0f;
  }

  addLabel(
    leftLimiter0, topLimiter03, limiterLabelWidth, labelHeight, uiTextSize, "Gate [dB]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter03, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterGate, Scales::limiterGate, true, 5);
  addLabel(
    leftLimiter0, topLimiter04, limiterLabelWidth, labelHeight, uiTextSize, "Attack [s]",
    kLeftText);
  addTextKnob<Uhhyou::Style::warning>(
    leftLimiter1, topLimiter04, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterAttack, Scales::limiterAttack, false, 5);
  addLabel(
    leftLimiter0, topLimiter05, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter05, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, Scales::limiterRelease, false, 5);
  addLabel(
    leftLimiter0, topLimiter06, limiterLabelWidth, labelHeight, uiTextSize, "Sustain [s]",
    kLeftText);
  addTextKnob<Uhhyou::Style::warning>(
    leftLimiter1, topLimiter06, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterSustain, Scales::limiterSustain, false, 5);
  addLabel(
    leftLimiter0, topLimiter07, limiterLabelWidth, labelHeight, uiTextSize, "Stereo Link",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter07, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterStereoLink, Scales::defaultScale, false, 5);

  addCheckbox(
    leftLimiter0, topLimiter08, checkboxWidth, labelHeight, uiTextSize, "Sidechain",
    ID::sidechain);
  std::vector<std::string> channelTypeItems{"L-R", "M-S"};
  addOptionMenu(
    leftLimiter1, topLimiter08, limiterLabelWidth, labelHeight, uiTextSize,
    ID::channelType, channelTypeItems);

  addCheckbox(
    leftLimiter0, topLimiter09, checkboxWidth, labelHeight, uiTextSize, "True Peak",
    ID::truePeak);
  addKickButton(
    leftLimiter1, topLimiter09, limiterLabelWidth, labelHeight, uiTextSize,
    "Reset Overshoot", ID::overshoot);

  if (infoTextView) infoTextView->forget();
  infoTextView = addTextTableView(
    leftLimiter0, topLimiter10, 2 * limiterLabelWidth, labelHeight, uiTextSize,
    "Overshoot [dB]", limiterLabelWidth);
  infoTextView->remember();

  // Plugin name.
  const auto splashMargin = margin;
  const auto splashTop = defaultHeight - splashHeight - uiMargin + margin;
  const auto splashLeft = leftLimiter0;
  addSplashScreen(
    splashLeft, splashTop, checkboxWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "BasicLimiterAutoMake");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
