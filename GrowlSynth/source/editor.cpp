// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

// 480 + 20
constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 2 * knobWidth;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;
constexpr float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
constexpr float splashHeight = int(labelHeight * 3 / 2);

constexpr float barboxWidth = groupLabelWidth;
constexpr float barboxHeight = 5 * labelY - 2 * margin;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
constexpr float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

constexpr int_least32_t defaultWidth
  = int_least32_t(4 * uiMargin + 4 * groupLabelWidth + 12 * margin);
constexpr int_least32_t defaultHeight = int_least32_t(2 * uiMargin + tabViewHeight);

enum tabIndex { tabBatter, tabSnare };

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

  constexpr auto top0 = uiMargin;
  constexpr auto top1 = top0 + 1 * labelY;
  constexpr auto top2 = top0 + 2 * labelY;
  constexpr auto top3 = top0 + 3 * labelY;
  constexpr auto top4 = top0 + 4 * labelY;
  constexpr auto top5 = top0 + 5 * labelY;
  constexpr auto top6 = top0 + 6 * labelY;
  constexpr auto top7 = top0 + 7 * labelY;
  constexpr auto top8 = top0 + 8 * labelY;
  constexpr auto top9 = top0 + 9 * labelY;
  constexpr auto top10 = top0 + 10 * labelY;
  constexpr auto top11 = top0 + 11 * labelY;
  constexpr auto top12 = top0 + 12 * labelY;
  constexpr auto top13 = top0 + 13 * labelY;
  constexpr auto top14 = top0 + 14 * labelY;
  constexpr auto top15 = top0 + 15 * labelY;
  constexpr auto top16 = top0 + 16 * labelY;
  constexpr auto top17 = top0 + 17 * labelY;
  constexpr auto top18 = top0 + 18 * labelY;
  constexpr auto top19 = top0 + 19 * labelY;
  constexpr auto top20 = top0 + 20 * labelY;
  constexpr auto top21 = top0 + 21 * labelY;
  constexpr auto top22 = top0 + 22 * labelY;
  constexpr auto top23 = top0 + 23 * labelY;
  constexpr auto left0 = uiMargin;
  constexpr auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  constexpr auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  // Mix.
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
  addToggleButton(
    mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]",
    ID::safetyHighpassEnable);
  addTextKnob(
    mixLeft1, mixTop2, labelWidth, labelHeight, uiTextSize, ID::safetyHighpassHz,
    Scales::safetyHighpassHz, false, 5);
  addCheckbox(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);

  // Tuning.
  constexpr auto tuningTop0 = top0 + 4 * labelY;
  constexpr auto tuningTop1 = tuningTop0 + 1 * labelY;
  constexpr auto tuningTop2 = tuningTop0 + 2 * labelY;
  constexpr auto tuningTop3 = tuningTop0 + 3 * labelY;
  constexpr auto tuningTop4 = tuningTop0 + 4 * labelY;
  constexpr auto tuningTop5 = tuningTop0 + 5 * labelY;
  constexpr auto tuningTop6 = tuningTop0 + 6 * labelY;
  constexpr auto tuningTop7 = tuningTop0 + 7 * labelY;
  constexpr auto tuningTop8 = tuningTop0 + 8 * labelY;
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::tuningCent,
    Scales::cent, false, 5);
  addLabel(tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize, "Equal Temp.");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::tuningET,
    Scales::equalTemperament, false, 0, 1);
  addLabel(tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::tuningA4Hz,
    Scales::a4Hz, false, 0, a4HzOffset);
  addLabel(
    tuningLeft0, tuningTop6, labelWidth, labelHeight, uiTextSize, "P.Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop6, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);
  addLabel(
    tuningLeft0, tuningTop7, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    tuningLeft1, tuningTop7, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);
  constexpr auto slideAtWidth = int(groupLabelWidth / 3);
  constexpr auto slideAtLeft1 = tuningLeft0 + 1 * slideAtWidth;
  constexpr auto slideAtLeft2 = tuningLeft0 + 2 * slideAtWidth;
  addLabel(tuningLeft0, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Slide at");
  addCheckbox(
    slideAtLeft1, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Note-on",
    ID::slideAtNoteOn);
  addCheckbox(
    slideAtLeft2, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Note-off",
    ID::slideAtNoteOff);

  // Envelope.
  constexpr auto envTop0 = top0 + 0 * labelY;
  constexpr auto envTop1 = envTop0 + 1 * labelY;
  constexpr auto envTop2 = envTop0 + 2 * labelY;
  constexpr auto envTop3 = envTop0 + 3 * labelY;
  constexpr auto envTop4 = envTop0 + 4 * labelY;
  constexpr auto envTop5 = envTop0 + 5 * labelY;
  constexpr auto envLeft0 = left4;
  constexpr auto envLeft1 = envLeft0 + labelWidth + 2 * margin;
  addGroupLabel(envLeft0, envTop0, groupLabelWidth, labelHeight, uiTextSize, "Envelope");

  addLabel(envLeft0, envTop1, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    envLeft1, envTop1, labelWidth, labelHeight, uiTextSize, ID::envelopeAttackSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    envLeft1, envTop2, labelWidth, labelHeight, uiTextSize, ID::envelopeDecaySeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    envLeft1, envTop3, labelWidth, labelHeight, uiTextSize, ID::envelopeReleaseSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop4, labelWidth, labelHeight, uiTextSize, "Transition [s]");
  addTextKnob(
    envLeft1, envTop4, labelWidth, labelHeight, uiTextSize, ID::envelopeTransitionSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop5, labelWidth, labelHeight, uiTextSize, "AM");
  addTextKnob(
    envLeft1, envTop5, labelWidth, labelHeight, uiTextSize, ID::envelopeAM,
    Scales::envelopeAM, false, 5);

  // Breath
  constexpr auto breathTop0 = top0 + 6 * labelY;
  constexpr auto breathTop1 = breathTop0 + 1 * labelY;
  constexpr auto breathTop2 = breathTop0 + 2 * labelY;
  constexpr auto breathTop3 = breathTop0 + 3 * labelY;
  constexpr auto breathTop4 = breathTop0 + 4 * labelY;
  constexpr auto breathLeft0 = left4;
  constexpr auto breathLeft1 = breathLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    breathLeft0, breathTop0, groupLabelWidth, labelHeight, uiTextSize, "Breath");

  addLabel(breathLeft0, breathTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    breathLeft1, breathTop1, labelWidth, labelHeight, uiTextSize, ID::breathGain,
    Scales::gain, true, 5);
  addLabel(breathLeft0, breathTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    breathLeft1, breathTop2, labelWidth, labelHeight, uiTextSize, ID::breathDecaySeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(
    breathLeft0, breathTop3, labelWidth, labelHeight, uiTextSize, "Smoothing [Hz]");
  addTextKnob(
    breathLeft1, breathTop3, labelWidth, labelHeight, uiTextSize,
    ID::breathAMLowpassCutoffHz, Scales::cutoffHz, false, 5);
  addLabel(breathLeft0, breathTop4, labelWidth, labelHeight, uiTextSize, "Formant");
  addTextKnob(
    breathLeft1, breathTop4, labelWidth, labelHeight, uiTextSize, ID::breathFormantOctave,
    Scales::formantOctave, false, 5);

  // Pulse.
  constexpr auto pulseTop0 = top0 + 11 * labelY;
  constexpr auto pulseTop1 = pulseTop0 + 1 * labelY;
  constexpr auto pulseTop2 = pulseTop0 + 2 * labelY;
  constexpr auto pulseTop3 = pulseTop0 + 3 * labelY;
  constexpr auto pulseTop4 = pulseTop0 + 4 * labelY;
  constexpr auto pulseTop5 = pulseTop0 + 5 * labelY;
  constexpr auto pulseTop6 = pulseTop0 + 6 * labelY;
  constexpr auto pulseLeft0 = left4;
  constexpr auto pulseLeft1 = pulseLeft0 + labelWidth + 2 * margin;
  addGroupLabel(pulseLeft0, pulseTop0, groupLabelWidth, labelHeight, uiTextSize, "Pulse");

  addLabel(pulseLeft0, pulseTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    pulseLeft1, pulseTop1, labelWidth, labelHeight, uiTextSize, ID::pulseGain,
    Scales::gain, true, 5);
  addLabel(pulseLeft0, pulseTop2, labelWidth, labelHeight, uiTextSize, "Pitch [octave]");
  addTextKnob(
    pulseLeft1, pulseTop2, labelWidth, labelHeight, uiTextSize, ID::pulsePitchOctave,
    Scales::formantOctave, false, 5);
  addLabel(pulseLeft0, pulseTop3, labelWidth, labelHeight, uiTextSize, "Bend [octave]");
  addTextKnob(
    pulseLeft1, pulseTop3, labelWidth, labelHeight, uiTextSize, ID::pulseBendOctave,
    Scales::pulseBendOctave, false, 5);
  addLabel(pulseLeft0, pulseTop4, labelWidth, labelHeight, uiTextSize, "Modulation");
  addTextKnob(
    pulseLeft1, pulseTop4, labelWidth, labelHeight, uiTextSize,
    ID::frequencyModulationMix, Scales::defaultScale, false, 5);
  addLabel(pulseLeft0, pulseTop5, labelWidth, labelHeight, uiTextSize, "Formant");
  addTextKnob(
    pulseLeft1, pulseTop5, labelWidth, labelHeight, uiTextSize, ID::pulseFormantOctave,
    Scales::formantOctave, false, 5);
  addLabel(pulseLeft0, pulseTop6, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    pulseLeft1, pulseTop6, labelWidth, labelHeight, uiTextSize, ID::formantSeed,
    Scales::seed, false, 0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }

  // Comb.
  constexpr auto combTop0 = top0;
  constexpr auto combTop1 = combTop0 + 1 * labelY;
  constexpr auto combTop2 = combTop0 + 2 * labelY;
  constexpr auto combTop3 = combTop0 + 3 * labelY;
  constexpr auto combTop4 = combTop0 + 4 * labelY;
  constexpr auto combTop5 = combTop0 + 5 * labelY;
  constexpr auto combTop6 = combTop0 + 6 * labelY;
  constexpr auto combTop7 = combTop0 + 7 * labelY;
  constexpr auto combTop8 = combTop0 + 8 * labelY;
  constexpr auto combTop9 = combTop0 + 9 * labelY;
  constexpr auto combTop10 = combTop0 + 10 * labelY;
  constexpr auto combTop11 = combTop0 + 11 * labelY;
  constexpr auto combTop12 = combTop0 + 12 * labelY;
  constexpr auto combTop13 = combTop0 + 13 * labelY;
  constexpr auto combTop14 = combTop0 + 14 * labelY;
  constexpr auto combTop15 = combTop0 + 15 * labelY;
  constexpr auto combLeft0 = left8;
  constexpr auto combLeft1 = combLeft0 + labelWidth + 2 * margin;
  addGroupLabel(combLeft0, combTop0, groupLabelWidth, labelHeight, uiTextSize, "Comb");

  addLabel(combLeft0, combTop1, labelWidth, labelHeight, uiTextSize, "Follow Note");
  addTextKnob(
    combLeft1, combTop1, labelWidth, labelHeight, uiTextSize, ID::combFollowNote,
    Scales::defaultScale, false, 5);

  addLabel(combLeft0, combTop3, labelWidth, labelHeight, uiTextSize, "Time Spread [s]");
  addTextKnob(
    combLeft1, combTop3, labelWidth, labelHeight, uiTextSize, ID::maxTimeSpreadSeconds,
    Scales::maxTimeSpreadSeconds, false, 5);
  addLabel(combLeft0, combTop4, labelWidth, labelHeight, uiTextSize, "Random [octave]");
  addTextKnob(
    combLeft1, combTop4, labelWidth, labelHeight, uiTextSize,
    ID::combDelayFrequencyRandom, Scales::randomOctave, false, 5);
  /// TODO: add time spread modulation.
  addLabel(combLeft0, combTop5, labelWidth, labelHeight, uiTextSize, "Frequency [Hz]");
  addTextKnob(
    combLeft1, combTop5, labelWidth, labelHeight, uiTextSize, ID::combDelayFrequencyHz,
    Scales::frequencyHz, false, 5);
  addLabel(combLeft0, combTop6, labelWidth, labelHeight, uiTextSize, "Feedback Gain");
  addTextKnob(
    combLeft1, combTop6, labelWidth, labelHeight, uiTextSize, ID::combFeedbackGain,
    Scales::feedbackGain, false, 5);
  addLabel(combLeft0, combTop7, labelWidth, labelHeight, uiTextSize, "Feedback Mod.");
  addTextKnob(
    combLeft1, combTop7, labelWidth, labelHeight, uiTextSize,
    ID::combFeedbackFollowEnvelope, Scales::defaultScale, false, 5);
  addLabel(combLeft0, combTop8, labelWidth, labelHeight, uiTextSize, "LP Cut [Hz]");
  addTextKnob(
    combLeft1, combTop8, labelWidth, labelHeight, uiTextSize, ID::combLowpassHz,
    Scales::cutoffHz, false, 5);
  addLabel(combLeft0, combTop9, labelWidth, labelHeight, uiTextSize, "HP Cut [Hz]");
  addTextKnob(
    combLeft1, combTop9, labelWidth, labelHeight, uiTextSize, ID::combHighpassHz,
    Scales::cutoffHz, false, 5);
  addLabel(combLeft0, combTop10, labelWidth, labelHeight, uiTextSize, "AP Cut");
  addTextKnob(
    combLeft1, combTop10, labelWidth, labelHeight, uiTextSize, ID::combAllpassCut,
    Scales::frequencyRatio, false, 5);
  addLabel(combLeft0, combTop11, labelWidth, labelHeight, uiTextSize, "AP Q");
  addTextKnob(
    combLeft1, combTop11, labelWidth, labelHeight, uiTextSize, ID::combAllpassQ,
    Scales::filterQ, false, 5);
  addLabel(combLeft0, combTop12, labelWidth, labelHeight, uiTextSize, "AP Mod.");
  addTextKnob(
    combLeft1, combTop12, labelWidth, labelHeight, uiTextSize, ID::combAllpassMod,
    Scales::cutoffMod, false, 5);
  addLabel(combLeft0, combTop13, labelWidth, labelHeight, uiTextSize, "Loss Threshold");
  addTextKnob(
    combLeft1, combTop13, labelWidth, labelHeight, uiTextSize,
    ID::combEnergyLossThreshold, Scales::energyLossThreshold, false, 5);
  addLabel(combLeft0, combTop14, labelWidth, labelHeight, uiTextSize, "Delay Mod.");
  addTextKnob(
    combLeft1, combTop14, labelWidth, labelHeight, uiTextSize, ID::combDelayTimeMod,
    Scales::cutoffMod, false, 5);
  addLabel(combLeft0, combTop15, labelWidth, labelHeight, uiTextSize, "Delay Slew Rate.");
  addTextKnob(
    combLeft1, combTop15, labelWidth, labelHeight, uiTextSize, ID::combDelayTimeSlewRate,
    Scales::slewRate, false, 5);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = top0 + 13 * labelY + int(labelHeight / 4) + 2 * margin;
  constexpr auto splashLeft = left0 + int(labelWidth / 4);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "GrowlSynth", true);

  return true;
}

} // namespace Vst
} // namespace Steinberg
