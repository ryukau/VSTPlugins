// (c) 2021-2022 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

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
constexpr uint32_t defaultHeight = uint32_t(2 * uiMargin + 8 * labelY + splashHeight);

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
  const auto topLimiter1 = top0;
  const auto topLimiter2 = top0 + 1 * labelY;
  const auto topLimiter3 = top0 + 2 * labelY;
  const auto topLimiter4 = top0 + 3 * labelY;
  const auto topLimiter5 = top0 + 4 * labelY;
  const auto topLimiter6 = top0 + 5 * labelY;
  const auto topLimiter7 = top0 + 6 * labelY;
  const auto topLimiter8 = top0 + 7 * labelY;

  addLabel(
    leftLimiter0, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    "Threshold [dB]", kLeftText);
  auto thresholdKnob = addTextKnob(
    leftLimiter1, topLimiter1, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterThreshold, Scales::limiterThreshold, true, 5);
  if (thresholdKnob) {
    thresholdKnob->sensitivity = 0.002f / 6.0f;
    thresholdKnob->lowSensitivity = 0.002f / 120.0f;
  }
  addLabel(
    leftLimiter0, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize, "Gate [dB]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter2, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterGate, Scales::limiterGate, true, 5);
  addLabel(
    leftLimiter0, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize, "Attack [s]",
    kLeftText);
  auto textKnobAttack = addTextKnob<Uhhyou::Style::warning>(
    leftLimiter1, topLimiter3, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterAttack, Scales::limiterAttack, false, 5);
  if (textKnobAttack) textKnobAttack->liveUpdate = false;
  addLabel(
    leftLimiter0, topLimiter4, limiterLabelWidth, labelHeight, uiTextSize, "Release [s]",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter4, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterRelease, Scales::limiterRelease, false, 5);
  addLabel(
    leftLimiter0, topLimiter5, limiterLabelWidth, labelHeight, uiTextSize, "Sustain [s]",
    kLeftText);
  addTextKnob<Uhhyou::Style::warning>(
    leftLimiter1, topLimiter5, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterSustain, Scales::limiterSustain, false, 5);
  addLabel(
    leftLimiter0, topLimiter6, limiterLabelWidth, labelHeight, uiTextSize, "Stereo Link",
    kLeftText);
  addTextKnob(
    leftLimiter1, topLimiter6, limiterLabelWidth, labelHeight, uiTextSize,
    ID::limiterStereoLink, Scales::defaultScale, false, 5);

  addCheckbox(
    leftLimiter0, topLimiter7, checkboxWidth, labelHeight, uiTextSize, "True Peak",
    ID::truePeak);

  addKickButton(
    leftLimiter1, topLimiter7, limiterLabelWidth, labelHeight, uiTextSize,
    "Reset Overshoot", ID::overshoot);

  if (infoTextView) infoTextView->forget();
  infoTextView = addTextTableView(
    leftLimiter0, topLimiter8, 2 * limiterLabelWidth, labelHeight, uiTextSize,
    "Overshoot [dB]", limiterLabelWidth);
  infoTextView->remember();

  // Plugin name.
  const auto splashMargin = margin;
  const auto splashTop = defaultHeight - splashHeight - uiMargin + margin;
  const auto splashLeft = leftLimiter0;
  addSplashScreen(
    splashLeft, splashTop, checkboxWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "BasicLimiter");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
