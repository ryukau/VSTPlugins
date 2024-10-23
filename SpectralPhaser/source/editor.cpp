// Copyright Takamitsu Endo (ryukau@gmail.com).
// SPDX-License-Identifier: GPL-3.0-only

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 16.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 120.0f;
constexpr float labelWidthHalf = int(labelWidth / 2);
constexpr float lfoKnobWidth = 2 * labelHeight;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;

constexpr int_least32_t defaultWidth
  = int_least32_t(3 * uiMargin + 2 * groupLabelWidth + lfoKnobWidth);
constexpr int_least32_t defaultHeight = int_least32_t(uiMargin + 13 * labelY);

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

void Editor::valueChanged(CControl *pControl)
{
  using ID = Synth::ParameterID::ID;

  PlugEditor::valueChanged(pControl);

  ParamID id = pControl->getTag();

  if (id == ID::reportLatency) {
    controller->getComponentHandler()->restartComponent(kLatencyChanged);
    return;
  }

  if (id == ID::frameSize) {
    if (int(getPlainValue(ID::reportLatency)) == 1) return;
    controller->getComponentHandler()->restartComponent(kLatencyChanged);
    return;
  }
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = int(uiMargin / 2);
  constexpr auto left0 = uiMargin - margin;
  constexpr auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  constexpr auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  constexpr auto mixTop0 = top0;
  constexpr auto mixTop1 = mixTop0 + 1 * labelY;
  constexpr auto mixTop2 = mixTop0 + 2 * labelY;
  constexpr auto mixTop3 = mixTop0 + 3 * labelY;
  constexpr auto mixLeft0 = left0;
  constexpr auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Mix");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);
  addLabel(mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "Dry/Wet");
  addTextKnob(
    mixLeft1, mixTop2, labelWidth, labelHeight, uiTextSize, ID::dryWetMix,
    Scales::defaultScale, false, 5);
  addToggleButton(
    mixLeft0, mixTop3, labelWidth, labelHeight, uiTextSize, "Side Chain",
    ID::sideChainSwitch);
  addToggleButton(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, "Latency", ID::reportLatency);

  constexpr auto lfoTop0 = mixTop3 + 1 * labelY;
  constexpr auto lfoTop1 = lfoTop0 + 1 * labelY;
  constexpr auto lfoTop2 = lfoTop0 + 2 * labelY;
  constexpr auto lfoTop3 = lfoTop0 + 3 * labelY;
  constexpr auto lfoTop4 = lfoTop0 + 4 * labelY;
  constexpr auto lfoTop5 = lfoTop0 + 5 * labelY;
  constexpr auto lfoTop6 = lfoTop0 + 6 * labelY;
  constexpr auto lfoLeft0 = left0;
  constexpr auto lfoLeft1 = lfoLeft0 + labelWidth + 2 * margin;
  addGroupLabel(lfoLeft0, lfoTop0, groupLabelWidth, labelHeight, uiTextSize, "LFO");

  addLabel(lfoLeft0, lfoTop1, labelWidth, labelHeight, uiTextSize, "Waveform");
  std::vector<std::string> lfoWaveformItems{
    "Sine",           "Tri-Saw",        "- Reserved 3 -", "- Reserved 4 -",
    "- Reserved 5 -", "- Reserved 6 -", "- Reserved 7 -", "- Reserved 8 -"};
  addOptionMenu<Style::accent>(
    lfoLeft1, lfoTop1, labelWidth, labelHeight, uiTextSize, ID::lfoWaveform,
    lfoWaveformItems);
  addLabel(lfoLeft0, lfoTop2, labelWidth, labelHeight, uiTextSize, "Wave Mod.");
  addTextKnob(
    lfoLeft1, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoWaveMod,
    Scales::bipolarScale, false, 5);
  addLabel(lfoLeft0, lfoTop3, labelWidth, labelHeight, uiTextSize, "Phase");
  addRotaryTextKnob(
    lfoLeft1, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoInitialPhase,
    Scales::defaultScale, 5);
  addLabel(lfoLeft0, lfoTop4, labelWidth, labelHeight, uiTextSize, "Stereo");
  addRotaryTextKnob(
    lfoLeft1, lfoTop4, labelWidth, labelHeight, uiTextSize, ID::lfoStereoPhaseOffset,
    Scales::defaultScale, 5);
  addLabel(lfoLeft0, lfoTop5, labelWidth, labelHeight, uiTextSize, "Rate");
  addTextKnob(
    lfoLeft1, lfoTop5, labelWidth, labelHeight, uiTextSize, ID::lfoRate, Scales::lfoRate,
    false, 5);
  addLabel(lfoLeft0, lfoTop6, labelWidth, labelHeight, uiTextSize, "Sync.");
  auto lfoTempoUpperKnob = addTextKnob(
    lfoLeft1, lfoTop6, labelWidthHalf, labelHeight, uiTextSize, ID::lfoTempoUpper,
    Scales::lfoTempoSync, false, 0, 1);
  if (lfoTempoUpperKnob) {
    const auto denom = double(Scales::lfoTempoSync.getMax());
    lfoTempoUpperKnob->sensitivity = 0.1 / denom;
    lfoTempoUpperKnob->lowSensitivity = 0.0125 / denom;
  }
  auto lfoTempoLowerKnob = addTextKnob(
    lfoLeft1 + labelWidthHalf, lfoTop6, labelWidthHalf, labelHeight, uiTextSize,
    ID::lfoTempoLower, Scales::lfoTempoSync, false, 0, 1);
  if (lfoTempoLowerKnob) {
    const auto denom = double(Scales::lfoTempoSync.getMax());
    lfoTempoLowerKnob->sensitivity = 0.1 / denom;
    lfoTempoLowerKnob->lowSensitivity = 0.0125 / denom;
  }

  constexpr auto dlyTop0 = top0;
  constexpr auto dlyTop1 = dlyTop0 + 1 * labelY;
  constexpr auto dlyTop2 = dlyTop0 + 2 * labelY;
  constexpr auto dlyTop3 = dlyTop0 + 3 * labelY;
  constexpr auto dlyLeft0 = left4;
  constexpr auto dlyLeft1 = dlyLeft0 + labelWidth + 2 * margin;
  addGroupLabel(dlyLeft0, dlyTop0, groupLabelWidth, labelHeight, uiTextSize, "Delay");
  addLabel(dlyLeft0, dlyTop1, labelWidth, labelHeight, uiTextSize, "Transform");
  std::vector<std::string> transformItems{
    "FFT",
    "FWHT",
    "Haar",
    "- Reserved 4 -",
    "- Reserved 5 -",
    "- Reserved 6 -",
    "- Reserved 7 -",
    "- Reserved 8 -"};
  addOptionMenu<Style::warning>(
    dlyLeft1, dlyTop1, labelWidth, labelHeight, uiTextSize, ID::transform,
    transformItems);
  addLabel(dlyLeft0, dlyTop2, labelWidth, labelHeight, uiTextSize, "Frame Size");
  std::vector<std::string> frameSizeItems;
  for (size_t idx = 2; idx <= Synth::maxFrameSizeLog2; ++idx) {
    frameSizeItems.push_back(std::to_string(size_t(1) << idx));
  }
  addOptionMenu<Style::warning>(
    dlyLeft1, dlyTop2, labelWidth, labelHeight, uiTextSize, ID::frameSize,
    frameSizeItems);
  addLabel(dlyLeft0, dlyTop3, labelWidth, labelHeight, uiTextSize, "Feedback");
  addTextKnob<Style::warning>(
    dlyLeft1, dlyTop3, labelWidth, labelHeight, uiTextSize, ID::feedback,
    Scales::feedback, false, 5);

  constexpr auto maskTop0 = dlyTop3 + labelY;
  constexpr auto maskTop1 = maskTop0 + 1 * labelY;
  constexpr auto maskTop2 = maskTop0 + 2 * labelY;
  constexpr auto maskTop3 = maskTop0 + 3 * labelY;
  constexpr auto maskTop4 = maskTop0 + 4 * labelY;
  constexpr auto maskTop5 = maskTop0 + 5 * labelY;
  constexpr auto maskTop6 = maskTop0 + 6 * labelY;
  constexpr auto maskTop7 = maskTop0 + 7 * labelY;
  constexpr auto maskTop8 = maskTop0 + 8 * labelY;
  constexpr auto maskTop9 = maskTop0 + 9 * labelY;
  constexpr auto maskLeft0 = left4;
  constexpr auto maskLeft1 = maskLeft0 + labelWidth + 2 * margin;
  constexpr auto maskLeft2 = maskLeft1 + labelWidth + 2 * margin;
  constexpr auto maskLeft2Knob = maskLeft2 + 2 * margin;
  addGroupLabel(maskLeft0, maskTop0, groupLabelWidth, labelHeight, uiTextSize, "Mask");

  addLabel(maskLeft2, maskTop1, lfoKnobWidth, labelHeight, uiTextSize, "LFO");

  addLabel(maskLeft0, maskTop1, labelWidth, labelHeight, uiTextSize, "Waveform");
  std::vector<std::string> maskWaveformItems{
    "Cosine", "Square",         "Sawtooth Up",    "Sawtooth Down",
    "Noise",  "- Reserved 6 -", "- Reserved 7 -", "- Reserved 8 -"};
  addOptionMenu<Style::accent>(
    maskLeft1, maskTop1, labelWidth, labelHeight, uiTextSize, ID::maskWaveform,
    maskWaveformItems);
  addLabel(maskLeft0, maskTop2, labelWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob(
    maskLeft1, maskTop2, labelWidth, labelHeight, uiTextSize, ID::maskMix,
    Scales::defaultScale, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop2, labelHeight, labelHeight, ID::lfoToMaskMix);
  addLabel(maskLeft0, maskTop3, labelWidth, labelHeight, uiTextSize, "Phase");
  addRotaryTextKnob(
    maskLeft1, maskTop3, labelWidth, labelHeight, uiTextSize, ID::maskPhase,
    Scales::defaultScale, 5);
  addSmallKnob(maskLeft2Knob, maskTop3, labelHeight, labelHeight, ID::lfoToMaskPhase);
  addLabel(maskLeft0, maskTop4, labelWidth, labelHeight, uiTextSize, "Freq.");
  addTextKnob(
    maskLeft1, maskTop4, labelWidth, labelHeight, uiTextSize, ID::maskFreq,
    Scales::maskFreq, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop4, labelHeight, labelHeight, ID::lfoToMaskFreq);
  addLabel(maskLeft0, maskTop5, labelWidth, labelHeight, uiTextSize, "Threshold");
  addTextKnob(
    maskLeft1, maskTop5, labelWidth, labelHeight, uiTextSize, ID::maskThreshold,
    Scales::defaultScale, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop5, labelHeight, labelHeight, ID::lfoToMaskThreshold);
  addLabel(maskLeft0, maskTop6, labelWidth, labelHeight, uiTextSize, "Rotation");
  addTextKnob(
    maskLeft1, maskTop6, labelWidth, labelHeight, uiTextSize, ID::maskRotation,
    Scales::defaultScale, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop6, labelHeight, labelHeight, ID::lfoToMaskRotation);
  addLabel(maskLeft0, maskTop7, labelWidth, labelHeight, uiTextSize, "Spectral Shift");
  addTextKnob(
    maskLeft1, maskTop7, labelWidth, labelHeight, uiTextSize, ID::spectralShift,
    Scales::bipolarScale, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop7, labelHeight, labelHeight, ID::lfoToSpectralShift);
  addLabel(maskLeft0, maskTop8, labelWidth, labelHeight, uiTextSize, "Texture");
  addTextKnob(
    maskLeft1, maskTop8, labelWidth, labelHeight, uiTextSize, ID::maskChirp,
    Scales::defaultScale, false, 5);
  addSmallKnob(maskLeft2Knob, maskTop8, labelHeight, labelHeight, ID::lfoToMaskChirp);

  // Plugin name.
  constexpr auto splashMargin = uiMargin - margin;
  constexpr auto splashWidth = groupLabelWidth;
  constexpr auto splashHeight = labelHeight;
  constexpr auto splashTop = defaultHeight - splashHeight - uiMargin;
  constexpr auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "SpectralPhaser");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
