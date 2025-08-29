// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
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

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Misc.
  const auto miscTop0 = top0;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscTop2 = miscTop1 + labelWidth;
  const auto miscTop3 = miscTop2 + labelY;
  const auto miscTop4 = miscTop3 + labelY;
  const auto miscTop5 = miscTop4 + labelY;
  const auto miscTop6 = miscTop5 + labelY;
  const auto miscTop7 = miscTop6 + labelY;
  const auto miscTop8 = miscTop7 + labelY;
  const auto miscTop9 = miscTop8 + labelY;
  const auto miscTop10 = miscTop9 + labelY;
  const auto miscTop11 = miscTop10 + labelY;
  const auto miscLeft0 = left0;
  const auto miscLeft1 = miscLeft0 + labelWidth + 2 * margin;

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
  const auto inTop0 = top0;
  const auto inTop1 = inTop0 + labelY;
  const auto inTop2 = inTop1 + labelY;
  const auto inTop3 = inTop2 + labelY;
  const auto inTop4 = inTop3 + labelY;
  const auto inTop5 = inTop4 + labelY;
  const auto inTop6 = inTop5 + labelY;
  const auto inTop7 = inTop6 + labelY;
  const auto inLeft0 = miscLeft0 + 2 * labelWidth + 6 * margin;
  const auto inLeft1 = inLeft0 + labelWidth + 2 * margin;

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
  const auto sideTop0 = top0;
  const auto sideTop1 = sideTop0 + labelY;
  const auto sideTop2 = sideTop1 + labelY;
  const auto sideTop3 = sideTop2 + labelY;
  const auto sideTop4 = sideTop3 + labelY;
  const auto sideTop5 = sideTop4 + labelY;
  const auto sideTop6 = sideTop5 + labelY;
  const auto sideTop7 = sideTop6 + labelY;
  const auto sideLeft0 = inLeft0 + 2 * labelWidth + 6 * margin;
  const auto sideLeft1 = sideLeft0 + labelWidth + 2 * margin;

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
  const auto splashMargin = uiMargin;
  const auto splashWidth = int(2 * labelWidth + 2 * margin);
  const auto splashHeight = labelHeight + margin;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = left0;
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
