// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 22.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = 40.0f;
  knobX = 60.0f; // With margin.
  knobY = knobHeight + labelY;
  barboxWidth = 12.0f * knobX;
  barboxHeight = 2.0f * knobY;
  barboxY = barboxHeight + 2.0f * margin;
  checkboxWidth = 60.0f;
  splashHeight = 40.0f;
  defaultWidth = int32(barboxWidth + 2 * knobX + labelY + 4 * margin + 40);
  defaultHeight = int32(40 + 2 * labelY + knobY + 4 * barboxY + 2 * margin);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const float top0 = 20.0f;
  const float left0 = 20.0f;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0;
  addGroupLabel(gainLeft, gainTop, 2.0 * knobX, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Boost", ID::gainBoost);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain);

  // Pitch.
  const auto pitchTop0 = gainTop + labelY + knobY;
  const auto pitchLeft0 = left0;
  addGroupLabel(pitchLeft0, pitchTop0, 2.0f * knobX, "Pitch");
  addCheckbox(
    pitchLeft0, pitchTop0 + labelY - 2.0f * margin, knobX, "Add Aliasing", ID::aliasing);

  const auto pitchLabelWidth = knobX - 2.0f * margin;
  const auto pitchLeft1 = pitchLeft0 + margin;
  const auto pitchLeft2 = pitchLeft0 + knobX;

  const auto pitchTop1 = pitchTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(pitchLeft1, pitchTop1, pitchLabelWidth, "Octave");
  addTextKnob(
    pitchLeft2, pitchTop1, knobX, colorBlue, ID::masterOctave, Scales::masterOctave);

  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(pitchLeft1, pitchTop2, pitchLabelWidth, "Multiply");
  addTextKnob(
    pitchLeft2, pitchTop2, knobX, colorBlue, ID::pitchMultiply, Scales::pitchMultiply,
    false, 3);

  const auto pitchTop3 = pitchTop2 + labelY;
  addLabel(pitchLeft1, pitchTop3, pitchLabelWidth, "Modulo");
  addTextKnob(
    pitchLeft2, pitchTop3, knobX, colorBlue, ID::pitchModulo, Scales::pitchModulo, false,
    3);

  const auto pitchTop4 = pitchTop3 + labelY;
  addLabel(pitchLeft1, pitchTop4, pitchLabelWidth, "Expand");
  addTextKnob(
    pitchLeft2, pitchTop4, knobX, colorBlue, ID::overtoneExpand, Scales::overtoneExpand,
    false, 3);

  const auto pitchTop5 = pitchTop4 + labelY;
  addLabel(pitchLeft1, pitchTop5, pitchLabelWidth, "Shift");
  addTextKnob(
    pitchLeft2, pitchTop5, knobX, colorBlue, ID::overtoneShift, Scales::overtoneShift,
    false, 3);

  // Random.
  const auto randomTop0 = pitchTop0 + labelHeight + 6.0 * labelY - margin;
  const auto randomLeft0 = left0;
  const auto randomLeft1 = randomLeft0 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 2.0f * knobX, "Random");
  addCheckbox(
    randomLeft0, randomTop0 + labelY - 2.0f * margin, knobX, "Retrigger",
    ID::randomRetrigger);

  const auto randomTop1 = randomTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2.0f * margin, "Seed");
  addTextKnob(
    randomLeft1 - 2.0f * margin, randomTop1, knobX, colorBlue, ID::seed, Scales::seed);

  const auto randomTop2 = randomTop1 + labelY;
  addKnob(randomLeft0, randomTop2, knobWidth, colorBlue, "To Gain", ID::randomGain);
  addKnob(randomLeft1, randomTop2, knobWidth, colorBlue, "To Pitch", ID::randomFrequency);

  const auto randomTop3 = randomTop2 + knobY;
  addKnob(randomLeft0, randomTop3, knobWidth, colorBlue, "To Attack", ID::randomAttack);
  addKnob(randomLeft1, randomTop3, knobWidth, colorBlue, "To Decay", ID::randomDecay);

  const auto randomTop4 = randomTop3 + knobY;
  addKnob(randomLeft0, randomTop4, knobWidth, colorBlue, "To Sat.", ID::randomSaturation);
  addKnob(randomLeft1, randomTop4, knobWidth, colorBlue, "To Phase", ID::randomPhase);

  // Misc.
  const auto miscTop = randomTop4 + knobY;
  const auto miscLeft = left0;
  addGroupLabel(miscLeft, miscTop, 2.0f * knobX, "Misc.");

  addKnob(miscLeft, miscTop + labelY, knobWidth, colorBlue, "Smooth", ID::smoothness);

  const auto miscLeft0 = miscLeft + knobX - (checkboxWidth - knobWidth) / 2.0f;
  const auto miscTop0 = miscTop + labelY;
  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(miscLeft0, miscTop0, checkboxWidth, ID::nVoice, nVoiceOptions);
  addCheckbox(miscLeft0, miscTop0 + labelY, checkboxWidth, "Unison", ID::unison);

  // Modifier.
  const auto modTop = top0 + 4.0f * (barboxY + margin);
  const auto modLeft = left0 + 2.0f * knobX + 4.0f * margin + labelY;
  addGroupLabel(modLeft, modTop, 4.0f * knobX, "Modifier");

  const auto modTop0 = modTop + labelY;
  addKnob(modLeft, modTop0, knobWidth, colorBlue, "Attack*", ID::attackMultiplier);
  addKnob(modLeft + knobX, modTop0, knobWidth, colorBlue, "Decay*", ID::decayMultiplier);
  addKnob(modLeft + 2.0f * knobX, modTop0, knobWidth, colorBlue, "Gain^", ID::gainPower);
  addKnob(
    modLeft + 3.0f * knobX, modTop0, knobWidth, colorBlue, "Sat. Mix", ID::saturationMix);

  const auto modTop1 = modTop0 + knobY;
  addCheckbox(modLeft + 0.4f * knobX, modTop1, checkboxWidth, "Declick", ID::declick);

  // Phaser.
  const auto phaserTop = modTop;
  const auto phaserLeft = modLeft + 4.0f * knobX + 4.0f * margin;
  addGroupLabel(phaserLeft, phaserTop, 7.0f * knobX + labelY, "Phaser");

  const auto phaserTop0 = phaserTop + labelY;
  addKnob(phaserLeft, phaserTop0, knobWidth, colorBlue, "Mix", ID::phaserMix);
  addKnob(
    phaserLeft + knobX, phaserTop0, knobWidth, colorBlue, "Freq", ID::phaserFrequency);
  addKnob(
    phaserLeft + 2.0f * knobX, phaserTop0, knobWidth, colorRed, "Feedback",
    ID::phaserFeedback);
  addKnob(
    phaserLeft + 3.0f * knobX, phaserTop0, knobWidth, colorBlue, "Range",
    ID::phaserRange);
  addKnob(
    phaserLeft + 4.0f * knobX, phaserTop0, knobWidth, colorBlue, "Min", ID::phaserMin);
  addKnob(
    phaserLeft + 5.0f * knobX, phaserTop0, knobWidth, colorBlue, "Offset",
    ID::phaserOffset);
  addRotaryKnob(
    phaserLeft + 6.0f * knobX, phaserTop0, knobWidth + labelY, colorBlue, "Phase",
    ID::phaserPhase);

  const auto phaserTop1 = phaserTop0 + knobY;
  std::vector<UTF8String> phaserStageItems{
    "Stage 1",  "Stage 2",  "Stage 3",  "Stage 4",  "Stage 5",  "Stage 6",
    "Stage 7",  "Stage 8",  "Stage 9",  "Stage 10", "Stage 11", "Stage 12",
    "Stage 13", "Stage 14", "Stage 15", "Stage 16"};
  addOptionMenu(
    phaserLeft - margin, phaserTop1, knobX, ID::phaserStage, phaserStageItems);

  // Attack.
  const auto attackTop = top0;
  const auto attackLeft = left0 + 2.0f * knobX + 4.0f * margin;
  // addGroupVerticalLabel(attackLeft, attackTop, barboxHeight, "Attack");

  const auto attackLeft0 = attackLeft + labelY;
  addBarBox(
    attackLeft0, attackTop, barboxWidth, barboxHeight, ID::attack0, Synth::nOvertone,
    "Attack");

  // Decay.
  const auto decayTop = attackTop + barboxY + margin;
  const auto decayLeft = attackLeft;
  // addGroupVerticalLabel(decayLeft, decayTop, barboxHeight, "Decay");

  const auto decayLeft0 = decayLeft + labelY;
  addBarBox(
    decayLeft0, decayTop, barboxWidth, barboxHeight, ID::decay0, Synth::nOvertone,
    "Decay");

  // Overtone.
  const auto overtoneTop = decayTop + barboxY + margin;
  const auto overtoneLeft = attackLeft;
  // addGroupVerticalLabel(overtoneLeft, overtoneTop, barboxHeight, "Gain");

  const auto overtoneLeft0 = overtoneLeft + labelY;
  addBarBox(
    overtoneLeft0, overtoneTop, barboxWidth, barboxHeight, ID::overtone0,
    Synth::nOvertone, "Gain");

  // Saturation.
  const auto saturationTop = overtoneTop + barboxY + margin;
  const auto saturationLeft = attackLeft;
  // addGroupVerticalLabel(saturationLeft, saturationTop, barboxHeight, "Saturation");

  const auto saturationLeft0 = saturationLeft + labelY;
  addBarBox(
    saturationLeft0, saturationTop, barboxWidth, barboxHeight, ID::saturation0,
    Synth::nOvertone, "Saturation");

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 20.0f;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.5f * knobX, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, "EnvelopedSine");

  return true;
}

} // namespace Vst
} // namespace Steinberg
