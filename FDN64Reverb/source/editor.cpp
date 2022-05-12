// (c) 2021-2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 75.0f;
constexpr float labelX = labelWidth + margin;
constexpr float splashWidth = 2 * labelX;
constexpr float splashHeight = 30.0f;

constexpr float barboxWidth = 320.0f;
constexpr float barboxHeight = 160.0f;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 2 * barboxWidth + 4 * margin);
constexpr int_least32_t defaultHeight = int_least32_t(
  2 * uiMargin + 2 * barboxHeight + 4 * margin + 4 * labelY + splashHeight);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

void Editor::refreshSeed(ParamID id)
{
  using ID = Synth::ParameterID::ID;
  if (seedTextKnob == nullptr) return;
  if (id != ID::refreshMatrix) return;
  bool refreshMatrix = getPlainValue(ID::refreshMatrix) > 0;
  if (!refreshMatrix) return;

  std::random_device dev;
  pcg64 rng{dev()};
  std::uniform_real_distribution<double> dist{0.0, 1.0};
  double value = dist(rng);
  controller->setParamNormalized(ID::seed, value);
  controller->performEdit(ID::seed, value);
  seedTextKnob->setValueNormalized(value);
  seedTextKnob->invalid();
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);

  refreshSeed(id);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto top1 = top0 + barboxHeight + 2 * margin;
  const auto top2 = top1 + barboxHeight + 2 * margin;
  const auto top3 = top2 + barboxHeight + 2 * margin;
  const auto left0 = uiMargin;
  const auto left1 = left0 + barboxWidth + 4 * margin;

  addBarBox(
    left0, top0, barboxWidth, barboxHeight, ID::delayTime0, nDelay, Scales::delayTime,
    "DelayTime");
  addBarBox(
    left0, top1, barboxWidth, barboxHeight, ID::timeLfoAmount0, nDelay, Scales::delayTime,
    "Time LFO Amount");
  addBarBox(
    left1, top0, barboxWidth, barboxHeight, ID::lowpassCutoffHz0, nDelay,
    Scales::lowpassCutoffHz, "Lowpass Cutoff [Hz]");
  addBarBox(
    left1, top1, barboxWidth, barboxHeight, ID::highpassCutoffHz0, nDelay,
    Scales::highpassCutoffHz, "Highpass Cutoff [Hz]");

  const auto ctrlLeft1 = left0;
  const auto ctrlLeft2 = ctrlLeft1 + labelX;
  const auto ctrlLeft3 = ctrlLeft2 + labelX;
  const auto ctrlLeft4 = ctrlLeft3 + labelX;
  const auto ctrlLeft5 = ctrlLeft4 + labelX + 4 * margin;
  const auto ctrlLeft6 = ctrlLeft5 + labelX;
  const auto ctrlLeft7 = ctrlLeft6 + labelX;
  const auto ctrlLeft8 = ctrlLeft7 + labelX;
  const auto ctrlTop1 = top2;
  const auto ctrlTop2 = ctrlTop1 + labelY;
  const auto ctrlTop3 = ctrlTop2 + labelY;
  const auto ctrlTop4 = ctrlTop3 + labelY;
  const auto ctrlTop5 = ctrlTop4 + labelY;

  addGroupLabel(
    ctrlLeft1, ctrlTop1, 4 * labelX - margin, labelHeight, uiTextSize, "Delay");
  addLabel(
    ctrlLeft1, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Time Multi.", kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::timeMultiplier,
    Scales::defaultScale, false, 5);
  addLabel(
    ctrlLeft1, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Feedback", kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::feedback,
    Scales::feedback, false, 5);
  addLabel(
    ctrlLeft1, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Interp. Rate",
    kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::delayTimeInterpRate,
    Scales::delayTimeInterpRate, false, 5);
  addLabel(
    ctrlLeft1, ctrlTop5, labelWidth, labelHeight, uiTextSize, "Gate [dB]", kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop5, labelWidth, labelHeight, uiTextSize, ID::gateThreshold,
    Scales::gateThreshold, true, 5);

  addLabel(
    ctrlLeft3, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Matrix", kCenterText);
  std::vector<std::string> matrixTypes{
    "Ortho.",       "S. Ortho.",   "Circ. Ortho.", "Circ. 4",      "Circ. 8",
    "Circ. 16",     "Circ. 32",    "Upper Tri. +", "Upper Tri. -", "Lower Tri. +",
    "Lower Tri. -", "Schroeder +", "Schroeder -",  "Absorbent +",  "Absorbent -",
    "Hadamard",     "Conference",
  };
  addOptionMenu<Style::warning>(
    ctrlLeft4, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::matrixType,
    matrixTypes);

  addLabel(ctrlLeft3, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Seed", kCenterText);
  seedTextKnob = addTextKnob<Style::warning>(
    ctrlLeft4, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed);
  seedTextKnob->sensitivity = 2048.0f / float(1 << 24);
  seedTextKnob->lowSensitivity = 1.0f / float(1 << 24);

  addKickButton<Style::warning>(
    ctrlLeft3 + std::floor(0.25f * labelX), ctrlTop4 + margin,
    std::floor(1.75f * labelX) - 2 * margin, labelY + std::floor(labelHeight / 2),
    uiTextSize, "Change Matrix", ID::refreshMatrix);

  addGroupLabel(ctrlLeft5, ctrlTop1, 2 * labelX - margin, labelHeight, uiTextSize, "Mix");
  addLabel(
    ctrlLeft5, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Dry [dB]", kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::dry, Scales::dry, true,
    5);
  addLabel(
    ctrlLeft5, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Wet [dB]", kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::wet, Scales::wet, true,
    5);
  addLabel(
    ctrlLeft5, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Stereo Cross",
    kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::stereoCross,
    Scales::defaultScale, false, 5);

  addGroupLabel(
    ctrlLeft7, ctrlTop1, 2 * labelX - margin, labelHeight, uiTextSize, "Rotation");
  addLabel(
    ctrlLeft7, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Speed [Hz]", kCenterText);
  addTextKnob(
    ctrlLeft8, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::splitRotationHz,
    Scales::splitRotationHz, false, 5);
  addLabel(
    ctrlLeft7, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Offset", kCenterText);
  addTextKnob(
    ctrlLeft8, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::splitPhaseOffset,
    Scales::defaultScale, false, 5);
  addLabel(ctrlLeft7, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Skew", kCenterText);
  addTextKnob(
    ctrlLeft8, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::splitSkew,
    Scales::splitSkew, false, 5);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = ctrlTop5 + margin;
  const auto splashLeft = ctrlLeft6 + std::floor(0.25f * labelX);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "FDN64Reverb");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
