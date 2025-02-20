// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelWidth = 80.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float halfLabelWidth = int(labelWidth / 2);

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 6 * labelWidth + 14 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 9 * labelY + 2 * labelWidth + 2 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::syncUI(ParamID id, float normalized)
{
  auto syncer = xyControlMap.find(id);
  if (syncer == xyControlMap.end()) return;
  syncer->second->sync(normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  PlugEditor::valueChanged(pControl);
  syncUI(pControl->getTag(), pControl->getValueNormalized());
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);
  syncUI(id, normalized);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;

  // Misc.
  constexpr auto miscTop0 = top0;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscTop2 = miscTop1 + labelWidth;
  constexpr auto miscTop3 = miscTop2 + labelY;
  constexpr auto miscTop4 = miscTop3 + labelY;
  constexpr auto miscTop5 = miscTop4 + labelY;
  constexpr auto miscTop6 = miscTop5 + labelY;
  constexpr auto miscTop7 = miscTop6 + labelY;
  constexpr auto miscTop8 = miscTop7 + labelY;
  constexpr auto miscTop9 = miscTop8 + labelY;
  constexpr auto miscTop10 = miscTop9 + labelY;
  constexpr auto miscTop11 = miscTop10 + labelY;
  constexpr auto miscLeft0 = left0;
  constexpr auto miscLeft1 = miscLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    miscLeft0, miscTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Gain");
  addKnob(miscLeft0, miscTop1, labelWidth, margin, uiTextSize, "Output", ID::outputGain);
  addKnob(miscLeft1, miscTop1, labelWidth, margin, uiTextSize, "Mix", ID::mix);

  addGroupLabel(
    miscLeft0, miscTop3, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Stereo");
  addLabel(miscLeft0, miscTop4, labelWidth, labelHeight, uiTextSize, "Link [Hz]");
  addTextKnob(
    miscLeft1, miscTop4, labelWidth, labelHeight, uiTextSize, ID::stereoPhaseLinkHz,
    Scales::stereoPhaseLinkHz, false, 5);
  addLabel(miscLeft0, miscTop5, labelWidth, labelHeight, uiTextSize, "Cross");
  addTextKnob(
    miscLeft1, miscTop5, labelWidth, labelHeight, uiTextSize, ID::stereoPhaseCross,
    Scales::defaultScale, false, 5);
  addLabel(miscLeft0, miscTop6, labelWidth, labelHeight, uiTextSize, "Offset");
  addRotaryTextKnob(
    miscLeft1, miscTop6, labelWidth, labelHeight, uiTextSize, ID::stereoPhaseOffset,
    Scales::defaultScale, 5);

  addGroupLabel(
    miscLeft0, miscTop7, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Warp");
  addLabel(miscLeft0, miscTop8, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob<Style::warning>(
    miscLeft1, miscTop8, labelWidth, labelHeight, uiTextSize, ID::phaseWarp,
    Scales::defaultScale, false, 5);

  addGroupLabel(
    miscLeft0, miscTop9, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Misc.");
  addLabel(miscLeft0, miscTop10, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop10, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);
  addLabel(miscLeft0, miscTop11, labelWidth, labelHeight, uiTextSize, "Oversampling");
  std::vector<std::string> oversamplingItems{"1x", "2x", "16x"};
  addOptionMenu(
    miscLeft1, miscTop11, labelWidth, labelHeight, uiTextSize, ID::oversampling,
    oversamplingItems);

  // Input Modulation.
  constexpr auto inTop0 = top0;
  constexpr auto inTop1 = inTop0 + labelY;
  constexpr auto inTop2 = inTop1 + labelY;
  constexpr auto inTop3 = inTop2 + labelY;
  constexpr auto inTop4 = inTop3 + labelY;
  constexpr auto inTop5 = inTop4 + labelY;
  constexpr auto inTop6 = inTop5 + labelY;
  constexpr auto inTop7 = inTop6 + labelY;
  constexpr auto inLeft0 = miscLeft0 + 2 * labelWidth + 6 * margin;
  constexpr auto inLeft1 = inLeft0 + labelWidth + 2 * margin;

  addGroupLabel(inLeft0, inTop0, 2 * labelWidth, labelHeight, uiTextSize, "Main Input");

  addLabel(inLeft0, inTop1, labelWidth, labelHeight, uiTextSize, "Modulation");
  addTextKnob(
    inLeft1, inTop1, labelWidth, labelHeight, uiTextSize, ID::inputPhaseMod,
    Scales::modulation, false, 5);
  addLabel(inLeft0, inTop2, labelWidth, labelHeight, uiTextSize, "Lowpass [Hz]");
  addTextKnob(
    inLeft1, inTop2, labelWidth, labelHeight, uiTextSize, ID::inputLowpassHz,
    Scales::cutoffHz, false, 3);
  addLabel(inLeft0, inTop3, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    inLeft1, inTop3, labelWidth, labelHeight, uiTextSize, ID::inputHighpassHz,
    Scales::cutoffHz, false, 3);
  addLabel(inLeft0, inTop4, labelWidth, labelHeight, uiTextSize, "Gate [dB]");
  addTextKnob(
    inLeft1, inTop4, labelWidth, labelHeight, uiTextSize, ID::inputGateThreshold,
    Scales::gateThreshold, true, 5);
  addToggleButton(
    inLeft0, inTop5, labelWidth, labelHeight, uiTextSize, "Envelope [s]",
    ID::inputEnvelopeEnable);
  addTextKnob(
    inLeft1, inTop5, labelWidth, labelHeight, uiTextSize, ID::inputEnvelopeReleaseSecond,
    Scales::envelopeSecond, false, 5);

  addLabel(
    inLeft0, inTop6, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Asymmetry");
  addAsymXYControls(
    inLeft0, inTop7, labelWidth, labelHeight, margin, uiTextSize,
    ID::inputPreAsymmetryAmount, ID::inputPostAsymmetryAmount, ID::inputPreAsymmetryHarsh,
    ID::inputPostAsymmetryHarsh, Scales::defaultScale);

  // Side chain Modulation.
  constexpr auto sideTop0 = top0;
  constexpr auto sideTop1 = sideTop0 + labelY;
  constexpr auto sideTop2 = sideTop1 + labelY;
  constexpr auto sideTop3 = sideTop2 + labelY;
  constexpr auto sideTop4 = sideTop3 + labelY;
  constexpr auto sideTop5 = sideTop4 + labelY;
  constexpr auto sideTop6 = sideTop5 + labelY;
  constexpr auto sideTop7 = sideTop6 + labelY;
  constexpr auto sideLeft0 = inLeft0 + 2 * labelWidth + 6 * margin;
  constexpr auto sideLeft1 = sideLeft0 + labelWidth + 2 * margin;

  addGroupLabel(
    sideLeft0, sideTop0, 2 * (labelWidth + margin), labelHeight, uiTextSize,
    "Side Chain");

  addLabel(sideLeft0, sideTop1, labelWidth, labelHeight, uiTextSize, "Modulation");
  addTextKnob(
    sideLeft1, sideTop1, labelWidth, labelHeight, uiTextSize, ID::sideChainPhaseMod,
    Scales::modulation, false, 5);
  addLabel(sideLeft0, sideTop2, labelWidth, labelHeight, uiTextSize, "Lowpass [Hz]");
  addTextKnob(
    sideLeft1, sideTop2, labelWidth, labelHeight, uiTextSize, ID::sideChainLowpassHz,
    Scales::cutoffHz, false, 3);
  addLabel(sideLeft0, sideTop3, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    sideLeft1, sideTop3, labelWidth, labelHeight, uiTextSize, ID::sideChainHighpassHz,
    Scales::cutoffHz, false, 3);
  addLabel(sideLeft0, sideTop4, labelWidth, labelHeight, uiTextSize, "Gate [dB]");
  addTextKnob(
    sideLeft1, sideTop4, labelWidth, labelHeight, uiTextSize, ID::sideChainGateThreshold,
    Scales::gateThreshold, true, 5);
  addToggleButton(
    sideLeft0, sideTop5, labelWidth, labelHeight, uiTextSize, "Envelope [s]",
    ID::sideChainEnvelopeEnable);
  addTextKnob(
    sideLeft1, sideTop5, labelWidth, labelHeight, uiTextSize,
    ID::sideChainEnvelopeReleaseSecond, Scales::envelopeSecond, false, 5);

  addLabel(
    sideLeft0, sideTop6, 2 * (labelWidth + margin), labelHeight, uiTextSize, "Asymmetry");
  addAsymXYControls(
    sideLeft0, sideTop7, labelWidth, labelHeight, margin, uiTextSize,
    ID::sideChainPreAsymmetryAmount, ID::sideChainPostAsymmetryAmount,
    ID::sideChainPreAsymmetryHarsh, ID::sideChainPostAsymmetryHarsh,
    Scales::defaultScale);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = int(2 * labelWidth + 2 * margin);
  constexpr auto splashHeight = labelHeight + margin;
  constexpr auto splashTop = defaultHeight - uiMargin - splashHeight;
  constexpr auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "AccumulativeRingMod");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
