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

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 4 * groupLabelWidth);
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
  constexpr auto left12 = left0 + 3 * groupLabelWidth + 4 * margin + uiMargin;

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

  addLabel(mixLeft0, mixTop6, labelWidth, labelHeight, uiTextSize, "Spread");
  addTextKnob(
    mixLeft1, mixTop6, labelWidth, labelHeight, uiTextSize, ID::spreaderSpread,
    Scales::defaultScale, false, 5);
  addLabel(mixLeft0, mixTop7, labelWidth, labelHeight, uiTextSize, "Split [Hz]");
  addTextKnob(
    mixLeft1, mixTop7, labelWidth, labelHeight, uiTextSize, ID::spreaderSplitHz,
    Scales::cutoffFrequencyHz, false, 5);

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
  constexpr auto impactTop14 = impactTop0 + 14 * labelY;
  constexpr auto impactTop15 = impactTop0 + 15 * labelY;
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
    Scales::halfClosedGain, true, 5);
  addLabel(
    impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize, ID::noiseDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);

  addLabel(
    impactLeft0, impactTop5, labelWidth, labelHeight, uiTextSize,
    "Half Closed Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop5, labelWidth, labelHeight, uiTextSize, ID::halfClosedGain,
    Scales::halfClosedGain, true, 5);
  addLabel(
    impactLeft0, impactTop6, labelWidth, labelHeight, uiTextSize,
    "Half Closed Decay [s]");
  addTextKnob(
    impactLeft1, impactTop6, labelWidth, labelHeight, uiTextSize,
    ID::halfCloseDecaySecond, Scales::noiseDecaySeconds, false, 5);
  addLabel(
    impactLeft0, impactTop7, labelWidth, labelHeight, uiTextSize,
    "Half Closed Sustain [dB]");
  addTextKnob(
    impactLeft1, impactTop7, labelWidth, labelHeight, uiTextSize,
    ID::halfCloseSustainLevel, Scales::halfClosedGain, true, 5);
  addLabel(
    impactLeft0, impactTop8, labelWidth, labelHeight, uiTextSize,
    "Half Closed Pulse Duration [s]");
  addTextKnob(
    impactLeft1, impactTop8, labelWidth, labelHeight, uiTextSize,
    ID::halfClosedPulseSecond, Scales::noiseDecaySeconds, false, 5);
  addLabel(
    impactLeft0, impactTop9, labelWidth, labelHeight, uiTextSize,
    "Half Closed Density [Hz]");
  addTextKnob(
    impactLeft1, impactTop9, labelWidth, labelHeight, uiTextSize, ID::halfClosedDensityHz,
    Scales::halfClosedDensityHz, false, 5);
  addLabel(
    impactLeft0, impactTop10, labelWidth, labelHeight, uiTextSize,
    "Half Closed Highpass [Hz]");
  addTextKnob(
    impactLeft1, impactTop10, labelWidth, labelHeight, uiTextSize,
    ID::halfClosedHighpassHz, Scales::halfClosedDensityHz, false, 5);

  addLabel(
    impactLeft0, impactTop12, labelWidth, labelHeight, uiTextSize, "Close Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop12, labelWidth, labelHeight, uiTextSize, ID::closeGain,
    Scales::halfClosedGain, true, 5);
  addLabel(
    impactLeft0, impactTop13, labelWidth, labelHeight, uiTextSize, "Close Attack [s]");
  addTextKnob(
    impactLeft1, impactTop13, labelWidth, labelHeight, uiTextSize, ID::closeAttackSeconds,
    Scales::noiseDecaySeconds, false, 5);

  addLabel(
    impactLeft0, impactTop14, labelWidth, labelHeight, uiTextSize, "Loss Gain [dB]");
  addTextKnob(
    impactLeft1, impactTop14, labelWidth, labelHeight, uiTextSize, ID::lossGain,
    Scales::halfClosedGain, true, 5);

  // TODO: Delays.
  constexpr auto filterTop0 = top0 + 0 * labelY;
  constexpr auto filterTop1 = filterTop0 + 1 * labelY;
  constexpr auto filterTop2 = filterTop0 + 2 * labelY;
  constexpr auto filterTop3 = filterTop0 + 3 * labelY;
  constexpr auto filterTop4 = filterTop0 + 4 * labelY;
  constexpr auto filterLeft0 = left8;
  constexpr auto filterLeft1 = filterLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    filterLeft0, filterTop0, groupLabelWidth, labelHeight, uiTextSize, "Filter");

  addLabel(
    filterLeft0, filterTop1, labelWidth, labelHeight, uiTextSize,
    "High Shelf Cutoff [Hz]");
  addTextKnob(
    filterLeft1, filterTop1, labelWidth, labelHeight, uiTextSize,
    ID::highShelfFrequencyHz, Scales::cutoffFrequencyHz, false, 5);
  addLabel(
    filterLeft0, filterTop2, labelWidth, labelHeight, uiTextSize, "High Shelf Gain [dB]");
  auto highShelfGainTextKnob = addTextKnob(
    filterLeft1, filterTop2, labelWidth, labelHeight, uiTextSize, ID::highShelfGain,
    Scales::shelvingGain, true, 5);
  if (highShelfGainTextKnob) {
    highShelfGainTextKnob->lowSensitivity = 1.0 / 30000.0;
  }
  addLabel(
    filterLeft0, filterTop3, labelWidth, labelHeight, uiTextSize,
    "Low Shelf Cutoff [Hz]");
  addTextKnob(
    filterLeft1, filterTop3, labelWidth, labelHeight, uiTextSize, ID::lowShelfFrequencyHz,
    Scales::cutoffFrequencyHz, false, 5);
  addLabel(
    filterLeft0, filterTop4, labelWidth, labelHeight, uiTextSize, "Low Shelf Gain [dB]");
  auto lowShelfGainTextKnob = addTextKnob(
    filterLeft1, filterTop4, labelWidth, labelHeight, uiTextSize, ID::lowShelfGain,
    Scales::shelvingGain, true, 5);
  if (highShelfGainTextKnob) {
    lowShelfGainTextKnob->lowSensitivity = 1.0 / 30000.0;
  }

  // TODO: Delays.
  constexpr auto velocityTop0 = filterTop4 + labelY;
  constexpr auto velocityTop1 = velocityTop0 + 1 * labelY;
  constexpr auto velocityTop2 = velocityTop0 + 2 * labelY;
  constexpr auto velocityTop3 = velocityTop0 + 3 * labelY;
  constexpr auto velocityTop4 = velocityTop0 + 4 * labelY;
  constexpr auto velocityLeft0 = left8;
  constexpr auto velocityLeft1 = velocityLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    velocityLeft0, velocityTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Velocity Sensitivity");

  addLabel(
    velocityLeft0, velocityTop1, labelWidth, labelHeight, uiTextSize,
    "> Output Gain [dB]");
  addTextKnob(
    velocityLeft1, velocityTop1, labelWidth, labelHeight, uiTextSize,
    ID::velocityToOutputGain, Scales::velocityRangeDecibel, false, 5);
  addLabel(
    velocityLeft0, velocityTop2, labelWidth, labelHeight, uiTextSize,
    "> Half Closed Density");
  addTextKnob(
    velocityLeft1, velocityTop2, labelWidth, labelHeight, uiTextSize,
    ID::velocityToHalfClosedDensity, Scales::bipolarScale, false, 5);
  addLabel(
    velocityLeft0, velocityTop3, labelWidth, labelHeight, uiTextSize,
    "> Half Closed Highpass");
  addTextKnob(
    velocityLeft1, velocityTop3, labelWidth, labelHeight, uiTextSize,
    ID::velocityToHalfClosedHighpass, Scales::bipolarScale, false, 5);
  addLabel(
    velocityLeft0, velocityTop4, labelWidth, labelHeight, uiTextSize,
    "> Modulation [sample]");
  addTextKnob(
    velocityLeft1, velocityTop4, labelWidth, labelHeight, uiTextSize,
    ID::velocityToDelayTimeMod, Scales::delayTimeModAmount, false, 5);

  // TODO: Delays.
  constexpr auto thirdTop0 = top0 + 0 * labelY;
  constexpr auto thirdTop1 = thirdTop0 + 1 * labelY;
  constexpr auto thirdTop2 = thirdTop0 + 2 * labelY;
  constexpr auto thirdTop3 = thirdTop0 + 3 * labelY;
  constexpr auto thirdTop4 = thirdTop0 + 4 * labelY;
  constexpr auto thirdTop5 = thirdTop0 + 5 * labelY;
  constexpr auto thirdTop6 = thirdTop0 + 6 * labelY;
  constexpr auto thirdTop7 = thirdTop0 + 7 * labelY;
  constexpr auto thirdTop8 = thirdTop0 + 8 * labelY;
  constexpr auto thirdTop9 = thirdTop0 + 9 * labelY;
  constexpr auto thirdTop10 = thirdTop0 + 10 * labelY;
  constexpr auto thirdTop11 = thirdTop0 + 11 * labelY;
  constexpr auto thirdTop12 = thirdTop0 + 12 * labelY;
  constexpr auto thirdTop13 = thirdTop0 + 13 * labelY;
  constexpr auto thirdTop14 = thirdTop0 + 14 * labelY;
  constexpr auto thirdTop15 = thirdTop0 + 15 * labelY;
  constexpr auto thirdTop16 = thirdTop0 + 16 * labelY;
  constexpr auto thirdTop17 = thirdTop0 + 17 * labelY;
  constexpr auto thirdLeft0 = left12;
  constexpr auto thirdLeft1 = thirdLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    thirdLeft0, thirdTop0, groupLabelWidth, labelHeight, uiTextSize, "Allpass Loop");

  addLabel(thirdLeft0, thirdTop1, labelWidth, labelHeight, uiTextSize, "Shape");
  addTextKnob(
    thirdLeft1, thirdTop1, labelWidth, labelHeight, uiTextSize, ID::delayTimeShape,
    Scales::defaultScale, false, 5);
  addLabel(thirdLeft0, thirdTop2, labelWidth, labelHeight, uiTextSize, "Delay Base [s]");
  addTextKnob(
    thirdLeft1, thirdTop2, labelWidth, labelHeight, uiTextSize, ID::delayTimeBaseSecond,
    Scales::delayTimeSecond, false, 5);
  addLabel(
    thirdLeft0, thirdTop3, labelWidth, labelHeight, uiTextSize, "Delay Random [s]");
  addTextKnob(
    thirdLeft1, thirdTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeRandomSecond,
    Scales::delayTimeSecond, false, 5);
  addLabel(
    thirdLeft0, thirdTop4, labelWidth, labelHeight, uiTextSize, "Pitch Ratio [st.]");
  addTextKnob(
    thirdLeft1, thirdTop4, labelWidth, labelHeight, uiTextSize, ID::delayTimeRatio,
    Scales::semitone, false, 5);
  addLabel(
    thirdLeft0, thirdTop5, labelWidth, labelHeight, uiTextSize, "Modulation [sample]");
  addTextKnob(
    thirdLeft1, thirdTop5, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);

  addLabel(thirdLeft0, thirdTop8, labelWidth, labelHeight, uiTextSize, "Delay Count 1");
  addTextKnob(
    thirdLeft1, thirdTop8, labelWidth, labelHeight, uiTextSize, ID::allpassDelayCount1,
    Scales::allpassDelayCount, false, 0, 1);
  addLabel(thirdLeft0, thirdTop9, labelWidth, labelHeight, uiTextSize, "Delay Count 2");
  addTextKnob(
    thirdLeft1, thirdTop9, labelWidth, labelHeight, uiTextSize, ID::allpassDelayCount2,
    Scales::allpassDelayCount, false, 0, 1);
  addLabel(thirdLeft0, thirdTop10, labelWidth, labelHeight, uiTextSize, "Feed 1");
  addTextKnob(
    thirdLeft1, thirdTop10, labelWidth, labelHeight, uiTextSize, ID::allpassFeed1,
    Scales::bipolarScale, false, 5);
  addLabel(thirdLeft0, thirdTop11, labelWidth, labelHeight, uiTextSize, "Feed 2");
  addTextKnob(
    thirdLeft1, thirdTop11, labelWidth, labelHeight, uiTextSize, ID::allpassFeed2,
    Scales::bipolarScale, false, 5);

  addLabel(thirdLeft0, thirdTop13, labelWidth, labelHeight, uiTextSize, "Mix Spike");
  addTextKnob(
    thirdLeft1, thirdTop13, labelWidth, labelHeight, uiTextSize, ID::allpassMixSpike,
    Scales::defaultScale, false, 5);
  addLabel(thirdLeft0, thirdTop14, labelWidth, labelHeight, uiTextSize, "Mix Alt. Sign");
  addTextKnob(
    thirdLeft1, thirdTop14, labelWidth, labelHeight, uiTextSize, ID::allpassMixAltSign,
    Scales::defaultScale, false, 5);

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
