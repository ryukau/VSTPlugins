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

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto top1 = top0 + labelY;
  const auto top2 = top1 + labelY;
  const auto top3 = top2 + labelY;
  const auto top4 = top3 + labelY;
  const auto top5 = top4 + labelY;
  const auto top6 = top5 + labelY;
  const auto top7 = top6 + labelY;
  const auto top8 = top7 + labelY;
  const auto top9 = top8 + labelY;
  const auto top10 = top9 + labelY;
  const auto top11 = top10 + labelY;
  const auto top12 = top11 + labelY;
  const auto top13 = top12 + labelY;
  const auto top14 = top13 + labelY;
  const auto left0 = uiMargin;
  const auto left2 = left0 + labelWidth + 2 * margin;
  const auto left4 = left2 + labelWidth + 4 * margin;
  const auto left6 = left4 + labelWidth + 2 * margin;
  const auto left8 = left6 + labelWidth + 4 * margin;
  const auto left10 = left8 + labelWidth + 2 * margin;

  // Gain.
  addGroupLabel(left0, top0, groupLabelWidth, labelHeight, uiTextSize, "Gain");

  addLabel(left0, top1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    left2, top1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::outputGain,
    true, 5);
  addCheckbox(
    left0, top2, labelWidth, labelHeight, uiTextSize, "Normalize", ID::gainNormalization);
  addCheckbox(
    left2, top2, labelWidth, labelHeight, uiTextSize, "2x Sampling", ID::overSampling);

  // Impact.
  addGroupLabel(left0, top3, groupLabelWidth, labelHeight, uiTextSize, "Impact");

  addLabel(left0, top4, labelWidth, labelHeight, uiTextSize, "Amplitude [dB]");
  addTextKnob(
    left2, top4, labelWidth, labelHeight, uiTextSize, ID::pulseAmplitude,
    Scales::pulseAmplitude, true, 5);
  addLabel(left0, top5, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    left2, top5, labelWidth, labelHeight, uiTextSize, ID::pulseDecaySeconds,
    Scales::pulseDecaySeconds, false, 5);
  addLabel(left0, top6, labelWidth, labelHeight, uiTextSize, "Position");
  addTextKnob(
    left2, top6, labelWidth, labelHeight, uiTextSize, ID::impactCenterRimPosition,
    Scales::defaultScale, false, 5);

  // FDN Misc.
  addGroupLabel(left4, top0, groupLabelWidth, labelHeight, uiTextSize, "Delay");

  addLabel(left4, top1, labelWidth, labelHeight, uiTextSize, "Cross Feed");
  addTextKnob(
    left6, top1, labelWidth, labelHeight, uiTextSize, ID::fdnMatrixIdentityAmount,
    Scales::fdnMatrixIdentityAmount, false, 5);
  addLabel(left4, top2, labelWidth, labelHeight, uiTextSize, "Feedback");
  addTextKnob<Style::warning>(
    left6, top2, labelWidth, labelHeight, uiTextSize, ID::fdnFeedback,
    Scales::fdnFeedback, false, 5);
  addLabel(left4, top3, labelWidth, labelHeight, uiTextSize, "Shape");
  addTextKnob(
    left6, top3, labelWidth, labelHeight, uiTextSize, ID::fdnShape, Scales::defaultScale,
    false, 5);

  // FDN Modulation.
  addGroupLabel(left4, top4, groupLabelWidth, labelHeight, uiTextSize, "Modulation");

  addLabel(left4, top5, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob<Style::warning>(
    left6, top5, labelWidth, labelHeight, uiTextSize, ID::fdnModulationAmount,
    Scales::defaultScale, false, 5);
  addLabel(left4, top6, labelWidth, labelHeight, uiTextSize, "Interp. Rate");
  addTextKnob<Style::warning>(
    left6, top6, labelWidth, labelHeight, uiTextSize, ID::fdnInterpRate,
    Scales::fdnInterpRate, false, 5);
  addLabel(left4, top7, labelWidth, labelHeight, uiTextSize, "Max Ratio");
  addTextKnob(
    left6, top7, labelWidth, labelHeight, uiTextSize, ID::fdnMaxModulationRatio,
    Scales::fdnMaxModulationRatio, false, 5);

  addToggleButton(
    left4, top8, groupLabelWidth, labelHeight, uiTextSize, "Envelope", ID::enableModEnv);
  addLabel(left4, top9, labelWidth, labelHeight, uiTextSize, "Sustain [s]");
  addTextKnob(
    left6, top9, labelWidth, labelHeight, uiTextSize, ID::modEnvSustainSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(left4, top10, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    left6, top10, labelWidth, labelHeight, uiTextSize, ID::modEnvReleaseSeconds,
    Scales::envelopeSeconds, false, 5);

  // FDN Randomization.
  addGroupLabel(left4, top11, groupLabelWidth, labelHeight, uiTextSize, "Random");
  addLabel(left4, top12, labelWidth, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    left6, top12, labelWidth, labelHeight, uiTextSize, ID::fdnSeed, Scales::seed);
  addLabel(left4, top13, labelWidth, labelHeight, uiTextSize, "Matrix Rnd.");
  addTextKnob(
    left6, top13, labelWidth, labelHeight, uiTextSize, ID::fdnRandomMatrix,
    Scales::defaultScale, false, 5);
  addLabel(left4, top14, labelWidth, labelHeight, uiTextSize, "Overtone Rnd.");
  addTextKnob(
    left6, top14, labelWidth, labelHeight, uiTextSize, ID::fdnRandomOvertone,
    Scales::defaultScale, false, 5);

  // FDN Filter.
  addGroupLabel(left8, top0, groupLabelWidth, labelHeight, uiTextSize, "Filter");
  addLabel(left8, top1, labelWidth, labelHeight, uiTextSize, "LP Cut [Hz]");
  addTextKnob(
    left10, top1, labelWidth, labelHeight, uiTextSize, ID::fdnLowpassCutoffHz,
    Scales::filterCutoffHz, false, 5);
  addBarBox(
    left8, top2, barboxWidth, barboxHeight, ID::fdnLowpassQ0, fdnSize, Scales::filterQ,
    "LP Q");

  addLabel(left8, top7, labelWidth, labelHeight, uiTextSize, "HP Cut [Hz]");
  addTextKnob(
    left10, top7, labelWidth, labelHeight, uiTextSize, ID::fdnHighpassCutoffHz,
    Scales::filterCutoffHz, false, 5);
  addBarBox(
    left8, top8, barboxWidth, barboxHeight, ID::fdnHighpassQ0, fdnSize, Scales::filterQ,
    "HP Q");

  // Tuning.
  addGroupLabel(left0, top7, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(left0, top8, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    left2, top8, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(left0, top9, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    left2, top9, labelWidth, labelHeight, uiTextSize, ID::tuningCent, Scales::cent, false,
    5);
  addLabel(left0, top10, labelWidth, labelHeight, uiTextSize, "Equal Temp.");
  addTextKnob(
    left2, top10, labelWidth, labelHeight, uiTextSize, ID::tuningET,
    Scales::equalTemperament, false, 0, 1);
  addLabel(left0, top11, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    left2, top11, labelWidth, labelHeight, uiTextSize, ID::tuningA4Hz, Scales::a4Hz,
    false, 0, a4HzOffset);
  addLabel(left0, top12, labelWidth, labelHeight, uiTextSize, "P.Bend Range [st.]");
  addTextKnob(
    left2, top12, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);
  addLabel(left0, top13, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    left2, top13, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

  const auto slideAtWidth = int(groupLabelWidth / 3);
  const auto slideAtLeft1 = left0 + 1 * slideAtWidth;
  const auto slideAtLeft2 = left0 + 2 * slideAtWidth;
  addLabel(left0, top14, slideAtWidth, labelHeight, uiTextSize, "Slide at");
  addCheckbox(
    slideAtLeft1, top14, slideAtWidth, labelHeight, uiTextSize, "Note-on",
    ID::slideAtNoteOn);
  addCheckbox(
    slideAtLeft2, top14, slideAtWidth, labelHeight, uiTextSize, "Note-off",
    ID::slideAtNoteOff);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = top13 + int(labelHeight / 4) + 2 * margin;
  const auto splashLeft = left8 + int(labelWidth / 4);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "MembraneSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
