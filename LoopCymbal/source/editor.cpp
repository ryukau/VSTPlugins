// (c) 2021-2023 Takamitsu Endo
//
// This file is part of LoopCymbal.
//
// LoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "gui/randomizebutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 16.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 80.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 2 * knobWidth;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;
constexpr float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
constexpr float splashHeight = int(2 * labelHeight + 2 * margin);

constexpr float barBoxWidth = groupLabelWidth;
constexpr float barBoxHeight = 5 * labelY - 2 * margin;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
constexpr float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 3 * groupLabelWidth);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 20 * labelY - 2 * margin);

constexpr const char *wireDidntCollidedText = "Wire didn't collide.";
constexpr const char *membraneDidntCollidedText = "Membrane didn't collide.";

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
  using ID = Synth::ParameterID::ID;

  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  using ID = Synth::ParameterID::ID;

  PlugEditor::updateUI(id, normalized);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;
  constexpr auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  constexpr auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  // Mix.
  constexpr auto mixTop0 = top0;
  constexpr auto mixTop1 = mixTop0 + 1 * labelY;
  constexpr auto mixTop2 = mixTop0 + 2 * labelY;
  constexpr auto mixTop3 = mixTop0 + 3 * labelY;
  constexpr auto mixTop4 = mixTop0 + 4 * labelY;
  constexpr auto mixTop5 = mixTop0 + 5 * labelY;
  constexpr auto mixTop6 = mixTop0 + 6 * labelY;
  constexpr auto mixTop7 = mixTop0 + 7 * labelY;
  constexpr auto mixTop8 = mixTop0 + 8 * labelY;
  constexpr auto mixTop9 = mixTop0 + 9 * labelY;
  constexpr auto mixTop10 = mixTop0 + 10 * labelY;
  constexpr auto mixTop11 = mixTop0 + 11 * labelY;
  constexpr auto mixLeft0 = left0;
  constexpr auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Mix & Options");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);
  addCheckbox(
    mixLeft0, mixTop3, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);
  addCheckbox(
    mixLeft0, mixTop4, labelWidth, labelHeight, uiTextSize, "Reset Seed at Note-on",
    ID::resetSeedAtNoteOn);

  addLabel(mixLeft0, mixTop6, labelWidth, labelHeight, uiTextSize, "Stereo Balance");
  addTextKnob(
    mixLeft1, mixTop6, labelWidth, labelHeight, uiTextSize, ID::stereoBalance,
    Scales::bipolarScale, false, 5);
  addLabel(mixLeft0, mixTop7, labelWidth, labelHeight, uiTextSize, "Stereo Merge");
  addTextKnob(
    mixLeft1, mixTop7, labelWidth, labelHeight, uiTextSize, ID::stereoMerge,
    Scales::defaultScale, false, 5);

  addToggleButton(
    mixLeft0, mixTop8, groupLabelWidth, labelHeight, uiTextSize, "External Input",
    ID::useExternalInput);
  addLabel(mixLeft0, mixTop9, labelWidth, labelHeight, uiTextSize, "External Gain [dB]");
  addTextKnob(
    mixLeft1, mixTop9, labelWidth, labelHeight, uiTextSize, ID::externalInputGain,
    Scales::gain, true, 5);

  // Tuning.
  constexpr auto tuningTop0 = top0 + 12 * labelY;
  constexpr auto tuningTop1 = tuningTop0 + 1 * labelY;
  constexpr auto tuningTop2 = tuningTop0 + 2 * labelY;
  constexpr auto tuningTop3 = tuningTop0 + 3 * labelY;
  constexpr auto tuningTop4 = tuningTop0 + 4 * labelY;
  constexpr auto tuningTop5 = tuningTop0 + 5 * labelY;
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Note -> Pitch");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::notePitchAmount,
    Scales::bipolarScale, false, 5);
  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::tuningCent,
    Scales::cent, false, 5);
  addLabel(
    tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize,
    "Pitch Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);
  addLabel(
    tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

  // Cymbal.
  constexpr auto impactTop0 = top0 + 0 * labelY;
  constexpr auto impactTop1 = impactTop0 + 1 * labelY;
  constexpr auto impactTop2 = impactTop0 + 2 * labelY;
  constexpr auto impactTop3 = impactTop0 + 3 * labelY;
  constexpr auto impactTop4 = impactTop0 + 4 * labelY;
  constexpr auto impactTop5 = impactTop0 + 5 * labelY;
  constexpr auto impactTop6 = impactTop0 + 6 * labelY;
  constexpr auto impactTop7 = impactTop0 + 7 * labelY;
  constexpr auto impactLeft0 = left4;
  constexpr auto impactLeft1 = impactLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    impactLeft0, impactTop0, groupLabelWidth, labelHeight, uiTextSize, "Cymbal");

  addLabel(impactLeft0, impactTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    impactLeft1, impactTop1, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed,
    false, 0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }
  addLabel(
    impactLeft0, impactTop2, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob(
    impactLeft1, impactTop2, labelWidth, labelHeight, uiTextSize, ID::noiseDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(
    impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Delay Base [s]");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeBaseSecond,
    Scales::delayTimeSecond, false, 5);
  addLabel(
    impactLeft0, impactTop4, labelWidth, labelHeight, uiTextSize, "Delay Random [s]");
  addTextKnob(
    impactLeft1, impactTop4, labelWidth, labelHeight, uiTextSize,
    ID::delayTimeRandomSecond, Scales::delayTimeSecond, false, 5);
  addLabel(
    impactLeft0, impactTop5, labelWidth, labelHeight, uiTextSize, "Modulation [sample]");
  addTextKnob(
    impactLeft1, impactTop5, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);
  addLabel(impactLeft0, impactTop6, labelWidth, labelHeight, uiTextSize, "Feed");
  addTextKnob(
    impactLeft1, impactTop6, labelWidth, labelHeight, uiTextSize, ID::allpassFeed,
    Scales::bipolarScale, false, 5);
  addLabel(impactLeft0, impactTop7, labelWidth, labelHeight, uiTextSize, "Distance");
  addTextKnob(
    impactLeft1, impactTop7, labelWidth, labelHeight, uiTextSize, ID::hihatDistance,
    Scales::hihatDistance, false, 5);

  // Randomize button.
  const auto randomButtonTop = top0 + 18 * labelY;
  const auto randomButtonLeft = left0 + labelWidth + 2 * margin;
  auto panicButton = new RandomizeButton(
    CRect(
      randomButtonLeft, randomButtonTop, randomButtonLeft + labelWidth,
      randomButtonTop + splashHeight),
    this, 0, "Random", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = top0 + 18 * labelY;
  constexpr auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, labelWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "LoopCymbal", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
