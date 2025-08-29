// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

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

  const float top0 = uiMargin;
  const float left0 = uiMargin;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0;
  addGroupLabel(gainLeft, gainTop, 2 * knobX, labelHeight, midTextSize, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Boost",
    ID::gainBoost);
  addKnob(
    gainLeft + 1 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Gain", ID::gain);

  // Pitch.
  const auto pitchTop0 = gainTop + labelY + knobY;
  const auto pitchLeft0 = left0;
  addGroupLabel(pitchLeft0, pitchTop0, 2 * knobX, labelHeight, midTextSize, "Pitch");
  addCheckbox(
    pitchLeft0, pitchTop0 + labelY - 2 * margin, int(1.75 * knobX), labelHeight,
    uiTextSize, "Add Aliasing", ID::aliasing);

  const auto pitchLabelWidth = knobX - 2 * margin;
  const auto pitchLeft1 = pitchLeft0 + margin;
  const auto pitchLeft2 = pitchLeft0 + knobX;

  const auto pitchTop1 = pitchTop0 + 2 * labelY - 3 * margin;
  addLabel(pitchLeft1, pitchTop1, pitchLabelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    pitchLeft2, pitchTop1, knobX, labelHeight, uiTextSize, ID::masterOctave,
    Scales::masterOctave);

  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(pitchLeft1, pitchTop2, pitchLabelWidth, labelHeight, uiTextSize, "Multiply");
  addTextKnob(
    pitchLeft2, pitchTop2, knobX, labelHeight, uiTextSize, ID::pitchMultiply,
    Scales::pitchMultiply, false, 3);

  const auto pitchTop3 = pitchTop2 + labelY;
  addLabel(pitchLeft1, pitchTop3, pitchLabelWidth, labelHeight, uiTextSize, "Modulo");
  addTextKnob(
    pitchLeft2, pitchTop3, knobX, labelHeight, uiTextSize, ID::pitchModulo,
    Scales::pitchModulo, false, 3);

  const auto pitchTop4 = pitchTop3 + labelY;
  addLabel(pitchLeft1, pitchTop4, pitchLabelWidth, labelHeight, uiTextSize, "Expand");
  addTextKnob(
    pitchLeft2, pitchTop4, knobX, labelHeight, uiTextSize, ID::overtoneExpand,
    Scales::overtoneExpand, false, 3);

  const auto pitchTop5 = pitchTop4 + labelY;
  addLabel(pitchLeft1, pitchTop5, pitchLabelWidth, labelHeight, uiTextSize, "Shift");
  addTextKnob(
    pitchLeft2, pitchTop5, knobX, labelHeight, uiTextSize, ID::overtoneShift,
    Scales::overtoneShift, false, 3);

  // Random.
  const auto randomTop0 = pitchTop0 + labelHeight + 6 * labelY - margin;
  const auto randomLeft0 = left0;
  const auto randomLeft1 = randomLeft0 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 2 * knobX, labelHeight, midTextSize, "Random");
  addCheckbox(
    randomLeft0, randomTop0 + labelY - 2 * margin, int(1.5 * knobX), labelHeight,
    uiTextSize, "Retrigger", ID::randomRetrigger);

  const auto randomTop1 = randomTop0 + 2 * labelY - 3 * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2 * margin, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    randomLeft1 - 2 * margin, randomTop1, knobX, labelHeight, uiTextSize, ID::seed,
    Scales::seed);

  const auto randomTop2 = randomTop1 + labelY;
  addKnob(
    randomLeft0, randomTop2, knobWidth, margin, uiTextSize, "To Gain", ID::randomGain);
  addKnob(
    randomLeft1, randomTop2, knobWidth, margin, uiTextSize, "To Pitch",
    ID::randomFrequency);

  const auto randomTop3 = randomTop2 + knobY;
  addKnob(
    randomLeft0, randomTop3, knobWidth, margin, uiTextSize, "To Attack",
    ID::randomAttack);
  addKnob(
    randomLeft1, randomTop3, knobWidth, margin, uiTextSize, "To Decay", ID::randomDecay);

  const auto randomTop4 = randomTop3 + knobY;
  addKnob(
    randomLeft0, randomTop4, knobWidth, margin, uiTextSize, "To Sat.",
    ID::randomSaturation);
  addKnob(
    randomLeft1, randomTop4, knobWidth, margin, uiTextSize, "To Phase", ID::randomPhase);

  // Misc.
  const auto miscTop = randomTop4 + knobY;
  const auto miscLeft = left0;
  addGroupLabel(miscLeft, miscTop, 2 * knobX, labelHeight, midTextSize, "Misc.");

  addKnob(
    miscLeft, miscTop + labelY, knobWidth, margin, uiTextSize, "Smooth", ID::smoothness);

  const auto miscLeft0 = miscLeft + knobX - int((checkboxWidth - knobWidth) / 2);
  const auto miscTop0 = miscTop + labelY;
  std::vector<std::string> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(
    miscLeft0, miscTop0, checkboxWidth, labelHeight, uiTextSize, ID::nVoice,
    nVoiceOptions);
  addCheckbox(
    miscLeft0, miscTop0 + labelY, int(1.25 * checkboxWidth), labelHeight, uiTextSize,
    "Unison", ID::unison);

  // Modifier.
  const auto modTop = top0 + 4 * (barboxY + margin);
  const auto modLeft = left0 + 2 * knobX + 4 * margin + labelY;
  addGroupLabel(modLeft, modTop, 4 * knobX, labelHeight, midTextSize, "Modifier");

  const auto modTop0 = modTop + labelY;
  addKnob(
    modLeft, modTop0, knobWidth, margin, uiTextSize, "Attack*", ID::attackMultiplier);
  addKnob(
    modLeft + knobX, modTop0, knobWidth, margin, uiTextSize, "Decay*",
    ID::decayMultiplier);
  addKnob(
    modLeft + 2 * knobX, modTop0, knobWidth, margin, uiTextSize, "Gain^", ID::gainPower);
  addKnob(
    modLeft + 3 * knobX, modTop0, knobWidth, margin, uiTextSize, "Sat. Mix",
    ID::saturationMix);

  const auto modTop1 = modTop0 + knobY;
  addCheckbox(
    int(modLeft + 0.4 * knobX), modTop1, int(1.25 * checkboxWidth), labelHeight,
    uiTextSize, "Declick", ID::declick);

  // Phaser.
  const auto phaserTop = modTop;
  const auto phaserLeft = modLeft + 4 * knobX + 4 * margin;
  addGroupLabel(
    phaserLeft, phaserTop, 7 * knobX + labelY, labelHeight, midTextSize, "Phaser");

  const auto phaserTop0 = phaserTop + labelY;
  addKnob(phaserLeft, phaserTop0, knobWidth, margin, uiTextSize, "Mix", ID::phaserMix);
  addKnob(
    phaserLeft + knobX, phaserTop0, knobWidth, margin, uiTextSize, "Freq",
    ID::phaserFrequency);
  addKnob<Style::warning>(
    phaserLeft + 2 * knobX, phaserTop0, knobWidth, margin, uiTextSize, "Feedback",
    ID::phaserFeedback); // Warning
  addKnob(
    phaserLeft + 3 * knobX, phaserTop0, knobWidth, margin, uiTextSize, "Range",
    ID::phaserRange);
  addKnob(
    phaserLeft + 4 * knobX, phaserTop0, knobWidth, margin, uiTextSize, "Min",
    ID::phaserMin);
  addKnob(
    phaserLeft + 5 * knobX, phaserTop0, knobWidth, margin, uiTextSize, "Offset",
    ID::phaserOffset);
  addRotaryKnob(
    phaserLeft + 6 * knobX, phaserTop0, knobWidth + labelY, margin, uiTextSize, "Phase",
    ID::phaserPhase);

  const auto phaserTop1 = phaserTop0 + knobY;
  std::vector<std::string> phaserStageItems{
    "Stage 1",  "Stage 2",  "Stage 3",  "Stage 4",  "Stage 5",  "Stage 6",
    "Stage 7",  "Stage 8",  "Stage 9",  "Stage 10", "Stage 11", "Stage 12",
    "Stage 13", "Stage 14", "Stage 15", "Stage 16"};
  addOptionMenu(
    phaserLeft - margin, phaserTop1, knobX, labelHeight, uiTextSize, ID::phaserStage,
    phaserStageItems);

  // Attack.
  const auto attackTop = top0;
  const auto attackLeft = left0 + 2 * knobX + 4 * margin;
  addGroupVerticalLabel(
    attackLeft, attackTop, barboxHeight, labelHeight, midTextSize, "Attack");

  const auto attackLeft0 = attackLeft + labelY;
  addBarBox(
    attackLeft0, attackTop, barboxWidth, barboxHeight, ID::attack0, Synth::nOvertone,
    Scales::envelopeA, "Attack");

  // Decay.
  const auto decayTop = attackTop + barboxY + margin;
  const auto decayLeft = attackLeft;
  addGroupVerticalLabel(
    decayLeft, decayTop, barboxHeight, labelHeight, midTextSize, "Decay");

  const auto decayLeft0 = decayLeft + labelY;
  addBarBox(
    decayLeft0, decayTop, barboxWidth, barboxHeight, ID::decay0, Synth::nOvertone,
    Scales::envelopeD, "Decay");

  // Overtone.
  const auto overtoneTop = decayTop + barboxY + margin;
  const auto overtoneLeft = attackLeft;
  addGroupVerticalLabel(
    overtoneLeft, overtoneTop, barboxHeight, labelHeight, midTextSize, "Gain");

  const auto overtoneLeft0 = overtoneLeft + labelY;
  addBarBox(
    overtoneLeft0, overtoneTop, barboxWidth, barboxHeight, ID::overtone0,
    Synth::nOvertone, Scales::gainDecibel, "Gain");

  // Saturation.
  const auto saturationTop = overtoneTop + barboxY + margin;
  const auto saturationLeft = attackLeft;
  addGroupVerticalLabel(
    saturationLeft, saturationTop, barboxHeight, labelHeight, midTextSize, "Saturation");

  const auto saturationLeft0 = saturationLeft + labelY;
  addBarBox(
    saturationLeft0, saturationTop, barboxWidth, barboxHeight, ID::saturation0,
    Synth::nOvertone, Scales::saturation, "Saturation");

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, int(2.5 * knobX), splashHeight, uiMargin, uiMargin,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "EnvelopedSine");

  return true;
}

} // namespace Vst
} // namespace Steinberg
