// (c) 2021-2023 Takamitsu Endo
//
// This file is part of DoubleLoopCymbal.
//
// DoubleLoopCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DoubleLoopCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DoubleLoopCymbal.  If not, see <https://www.gnu.org/licenses/>.

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
  addCheckbox(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, "Release", ID::release);

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
  addLabel(
    tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Transpose [st.]");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::transposeSemitone,
    Scales::semitone, false, 5);
  addLabel(
    tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize,
    "Pitch Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::semitone, false, 5);
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
  constexpr auto impactTop8 = impactTop0 + 8 * labelY;
  constexpr auto impactTop9 = impactTop0 + 9 * labelY;
  constexpr auto impactTop10 = impactTop0 + 10 * labelY;
  constexpr auto impactTop11 = impactTop0 + 11 * labelY;
  constexpr auto impactTop12 = impactTop0 + 12 * labelY;
  constexpr auto impactTop13 = impactTop0 + 13 * labelY;
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
    impactLeft0, impactTop2, labelWidth, labelHeight, uiTextSize, "Noise Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop2, labelWidth, labelHeight, uiTextSize, ID::noiseGain,
    Scales::gain, true, 5);
  addLabel(
    impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize, ID::noiseDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);

  addLabel(
    impactLeft0, impactTop8, labelWidth, labelHeight, uiTextSize,
    "High Shelf Cutoff [Hz]");
  addTextKnob(
    impactLeft1, impactTop8, labelWidth, labelHeight, uiTextSize,
    ID::highShelfFrequencyHz, Scales::cutoffFrequencyHz, false, 5);
  addLabel(
    impactLeft0, impactTop9, labelWidth, labelHeight, uiTextSize, "High Shelf Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop9, labelWidth, labelHeight, uiTextSize, ID::highShelfGain,
    Scales::shelvingGain, true, 5);
  addLabel(
    impactLeft0, impactTop10, labelWidth, labelHeight, uiTextSize,
    "Low Shelf Cutoff [Hz]");
  addTextKnob(
    impactLeft1, impactTop10, labelWidth, labelHeight, uiTextSize,
    ID::lowShelfFrequencyHz, Scales::cutoffFrequencyHz, false, 5);
  addLabel(
    impactLeft0, impactTop11, labelWidth, labelHeight, uiTextSize, "Low Shelf Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop11, labelWidth, labelHeight, uiTextSize, ID::lowShelfGain,
    Scales::shelvingGain, true, 5);
  addLabel(impactLeft0, impactTop12, labelWidth, labelHeight, uiTextSize, "Mix Spike");
  addTextKnob(
    impactLeft1, impactTop12, labelWidth, labelHeight, uiTextSize, ID::allpassMixSpike,
    Scales::defaultScale, false, 5);
  addLabel(
    impactLeft0, impactTop13, labelWidth, labelHeight, uiTextSize, "Mix Alt. Sign");
  addTextKnob(
    impactLeft1, impactTop13, labelWidth, labelHeight, uiTextSize, ID::allpassMixAltSign,
    Scales::defaultScale, false, 5);

  // Secondary.
  constexpr auto secondTop0 = top0 + 0 * labelY;
  constexpr auto secondTop1 = secondTop0 + 1 * labelY;
  constexpr auto secondTop2 = secondTop0 + 2 * labelY;
  constexpr auto secondTop3 = secondTop0 + 3 * labelY;
  constexpr auto secondTop4 = secondTop0 + 4 * labelY;
  constexpr auto secondTop5 = secondTop0 + 5 * labelY;
  constexpr auto secondTop6 = secondTop0 + 6 * labelY;
  constexpr auto secondTop7 = secondTop0 + 7 * labelY;
  constexpr auto secondTop8 = secondTop0 + 8 * labelY;
  constexpr auto secondTop9 = secondTop0 + 9 * labelY;
  constexpr auto secondTop10 = secondTop0 + 10 * labelY;
  constexpr auto secondTop11 = secondTop0 + 11 * labelY;
  constexpr auto secondLeft0 = left8;
  constexpr auto secondLeft1 = secondLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    secondLeft0, secondTop0, groupLabelWidth, labelHeight, uiTextSize, "Allpass Loop");

  addLabel(secondLeft0, secondTop1, labelWidth, labelHeight, uiTextSize, "Shape");
  addTextKnob(
    secondLeft1, secondTop1, labelWidth, labelHeight, uiTextSize, ID::delayTimeShape,
    Scales::defaultScale, false, 5);
  addLabel(
    secondLeft0, secondTop2, labelWidth, labelHeight, uiTextSize, "Delay Base [s]");
  addTextKnob(
    secondLeft1, secondTop2, labelWidth, labelHeight, uiTextSize, ID::delayTimeBaseSecond,
    Scales::delayTimeSecond, false, 5);
  addLabel(
    secondLeft0, secondTop3, labelWidth, labelHeight, uiTextSize, "Delay Random [s]");
  addTextKnob(
    secondLeft1, secondTop3, labelWidth, labelHeight, uiTextSize,
    ID::delayTimeRandomSecond, Scales::delayTimeSecond, false, 5);
  addLabel(
    secondLeft0, secondTop4, labelWidth, labelHeight, uiTextSize, "Modulation [sample]");
  addTextKnob(
    secondLeft1, secondTop4, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);

  addLabel(secondLeft0, secondTop6, labelWidth, labelHeight, uiTextSize, "nNotch");
  addTextKnob(
    secondLeft1, secondTop6, labelWidth, labelHeight, uiTextSize, ID::nAdaptiveNotch,
    Scales::nAdaptiveNotch, false, 0, 0);
  addLabel(secondLeft0, secondTop7, labelWidth, labelHeight, uiTextSize, "Notch Mix");
  addTextKnob(
    secondLeft1, secondTop7, labelWidth, labelHeight, uiTextSize, ID::adaptiveNotchMix,
    Scales::defaultScale, false, 5);
  addLabel(
    secondLeft0, secondTop8, labelWidth, labelHeight, uiTextSize, "Notch Narrowness");
  addTextKnob(
    secondLeft1, secondTop8, labelWidth, labelHeight, uiTextSize,
    ID::adaptiveNotchNarrowness, Scales::adaptiveNotchNarrowness, false, 5);

  addLabel(secondLeft0, secondTop10, labelWidth, labelHeight, uiTextSize, "Feed 1");
  addTextKnob(
    secondLeft1, secondTop10, labelWidth, labelHeight, uiTextSize, ID::allpassFeed1,
    Scales::bipolarScale, false, 5);
  addLabel(secondLeft0, secondTop11, labelWidth, labelHeight, uiTextSize, "Feed 2");
  addTextKnob(
    secondLeft1, secondTop11, labelWidth, labelHeight, uiTextSize, ID::allpassFeed2,
    Scales::bipolarScale, false, 5);

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
    "DoubleLoopCymbal", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
