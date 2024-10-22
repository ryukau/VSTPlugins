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
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 75.0f;
constexpr float labelX = labelWidth + margin;
constexpr float knobWidth = 50.0f;
constexpr float splashWidth = 2 * labelWidth - 4 * margin;
constexpr float splashHeight = 30.0f;

constexpr float barboxWidth = 6 * labelX + 3 * margin;
constexpr float barboxHeight = 6 * labelY - 2 * margin;

constexpr int_least32_t defaultWidth = int_least32_t(2 * uiMargin + barboxWidth);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 11 * labelY + barboxHeight + 2 * margin);

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

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto twoInDecibel = 6.020599913279624;       // == 20.0 * std::log10(2.0);
  constexpr auto oneFifthInDecibel = 3.5218251811136247; //== 20.0 * std::log10(1.5);

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  const auto ctrlLeft1 = left0;
  const auto ctrlLeft2 = ctrlLeft1 + labelX;
  const auto ctrlLeft3 = ctrlLeft2 + labelX;
  const auto ctrlLeft4 = ctrlLeft3 + labelX;
  const auto ctrlLeft5 = ctrlLeft4 + labelX + 4 * margin;
  const auto ctrlLeft6 = ctrlLeft5 + labelX;
  const auto ctrlTop1 = top0;
  const auto ctrlTop2 = ctrlTop1 + labelY;
  const auto ctrlTop3 = ctrlTop2 + labelY;
  const auto ctrlTop4 = ctrlTop3 + labelY;
  const auto ctrlTop5 = ctrlTop4 + labelY;
  const auto ctrlTop6 = ctrlTop5 + labelY;
  const auto ctrlTop7 = ctrlTop6 + labelY;

  addGroupLabel(
    ctrlLeft1, ctrlTop1, 4 * labelX - margin, labelHeight, uiTextSize, "Delay");
  addLabel(
    ctrlLeft1, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Pitch", kCenterText);
  auto textKnobPitch = addTextKnob(
    ctrlLeft2, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::pitch, Scales::pitch,
    false, 5);
  if (textKnobPitch) {
    textKnobPitch->sensitivity = twoInDecibel / 48.0 / 120.0;
    textKnobPitch->lowSensitivity = 0.0001 * twoInDecibel / 120.0;
    textKnobPitch->wheelSensitivity = oneFifthInDecibel / 4.0 / 120.0;
  }
  addLabel(
    ctrlLeft1, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Offset", kCenterText);
  auto textKnobUnisonPitchOffset = addTextKnob(
    ctrlLeft2, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::unisonPitchOffset,
    Scales::pitch, false, 5);
  if (textKnobUnisonPitchOffset) {
    textKnobPitch->lowSensitivity = 0.0001 * twoInDecibel / 120.0;
  }
  addLabel(
    ctrlLeft1, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Pitch Cross", kCenterText);
  addTextKnob(
    ctrlLeft2, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::pitchCross,
    Scales::defaultScale, false, 5);
  addCheckbox(
    ctrlLeft1, ctrlTop5, labelWidth, labelHeight, uiTextSize, "Mirror",
    ID::mirrorUnisonPitch);
  addCheckbox(
    ctrlLeft1, ctrlTop6, labelWidth, labelHeight, uiTextSize, "S1 Reverse",
    ID::shifterMainReverse);
  addCheckbox(
    ctrlLeft1, ctrlTop7, labelWidth, labelHeight, uiTextSize, "S2 Reverse",
    ID::shifterUnisonReverse);
  addKnob(
    ctrlLeft2, ctrlTop5, labelWidth, margin, uiTextSize, "L-R Lean", ID::stereoLean);

  addLabel(
    ctrlLeft3, ctrlTop2, labelWidth, labelHeight, uiTextSize, "Delay Time [s]",
    kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop2, labelWidth, labelHeight, uiTextSize, ID::delayTime,
    Scales::delayTime, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop3, labelWidth, labelHeight, uiTextSize, "Feedback", kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop3, labelWidth, labelHeight, uiTextSize, ID::feedback,
    Scales::feedback, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Stereo Cross",
    kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::stereoCross,
    Scales::defaultScale, false, 5);
  addLabel(
    ctrlLeft3, ctrlTop5, labelWidth, labelHeight, uiTextSize, "Channel Type",
    kCenterText);
  std::vector<std::string> channelTypeItems{"L-R", "M-S"};
  addOptionMenu<Style::warning>(
    ctrlLeft4, ctrlTop5, labelWidth, labelHeight, uiTextSize, ID::channelType,
    channelTypeItems);
  addLabel(
    ctrlLeft3, ctrlTop6, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]",
    kCenterText);
  addTextKnob(
    ctrlLeft4, ctrlTop6, labelWidth, labelHeight, uiTextSize, ID::highpassCutoffHz,
    Scales::highpassCutoffHz, false, 5);

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
    ctrlLeft5, ctrlTop4, labelWidth, labelHeight, uiTextSize, "Unison Mix", kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop4, labelWidth, labelHeight, uiTextSize, ID::unisonMix,
    Scales::defaultScale, false, 5);
  addLabel(
    ctrlLeft5, ctrlTop5, labelWidth, labelHeight, uiTextSize, "Smoothing [s]",
    kCenterText);
  addTextKnob(
    ctrlLeft6, ctrlTop5, labelWidth, labelHeight, uiTextSize, ID::smoothingTime,
    Scales::smoothingTime, false, 5);

  const auto lfoTop0 = ctrlTop7 + labelY + 2 * margin;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + labelY;
  const auto lfoTop3 = lfoTop2 + labelY;
  const auto lfoTop4 = lfoTop3 + labelY;

  const auto lfoLeft0 = left0;
  const auto lfoLeft1 = lfoLeft0 + labelX;
  const auto lfoLeft2 = lfoLeft1 + labelX;
  const auto lfoLeft3 = lfoLeft2 + labelX;
  const auto lfoLeft4 = lfoLeft3 + labelX + 4 * margin;
  const auto lfoLeft5 = lfoLeft4 + labelX;

  addGroupLabel(
    lfoLeft0, lfoTop0, double(defaultWidth) - 2 * uiMargin, labelHeight, uiTextSize,
    "LFO");

  addCheckbox(
    lfoLeft0 + knobWidth / 2, lfoTop2, knobWidth, labelHeight, uiTextSize, "Sync.",
    ID::lfoTempoSync);
  addTextKnob(
    lfoLeft1, lfoTop1 + 4 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoUpper,
    Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft1, lfoTop3 - 4 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoLower,
    Scales::lfoTempoLower, false, 0, 1);

  addLabel(lfoLeft2, lfoTop1, labelWidth, labelHeight, uiTextSize, "Rate", kCenterText);
  addTextKnob(
    lfoLeft3, lfoTop1, labelWidth, labelHeight, uiTextSize, ID::lfoRate, Scales::lfoRate,
    false, 5);
  addLabel(
    lfoLeft2, lfoTop2, labelWidth, labelHeight, uiTextSize, "Stereo Offset", kCenterText);
  addTextKnob(
    lfoLeft3, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoStereoOffset,
    Scales::stereoLean, false, 5);
  addLabel(
    lfoLeft2, lfoTop3, labelWidth, labelHeight, uiTextSize, "Unison Offset", kCenterText);
  addRotaryTextKnob(
    lfoLeft3, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoUnisonOffset,
    Scales::defaultScale, 5);

  addLabel(lfoLeft4, lfoTop1, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  std::vector<std::string> lfoInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu(
    lfoLeft5, lfoTop1, labelWidth, labelHeight, uiTextSize, ID::lfoInterpolation,
    lfoInterpolationItems);
  addLabel(
    lfoLeft4, lfoTop2, labelWidth, labelHeight, uiTextSize, "To Pitch", kCenterText);
  addTextKnob(
    lfoLeft5, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoToPitch, Scales::pitch,
    false, 5);
  addLabel(
    lfoLeft4, lfoTop3, labelWidth, labelHeight, uiTextSize, "To Unison", kCenterText);
  addTextKnob(
    lfoLeft5, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoToUnison,
    Scales::pitch, false, 5);

  auto barboxLfoWavetable = addBarBox(
    lfoLeft0, lfoTop4, barboxWidth, barboxHeight, ID::lfoWavetable0, nLfoWavetable,
    Scales::lfoWavetable, "LFO Wave");
  if (barboxLfoWavetable) {
    barboxLfoWavetable->sliderZero = 0.5f;
  }

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = ctrlTop7;
  const auto splashLeft = ctrlLeft5 + 2 * margin;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "PitchShiftDelay");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
