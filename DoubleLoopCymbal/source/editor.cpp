// (c) 2024 Takamitsu Endo
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

constexpr float barBoxWidth = groupLabelWidth;
constexpr float barBoxHeight = 5 * labelY - 2 * margin;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
constexpr float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 3 * groupLabelWidth);
constexpr int_least32_t defaultHeight = int_least32_t(uiMargin + 19 * labelY);

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

  constexpr auto top0 = int(uiMargin / 2);
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

  constexpr auto mixLeft1_per_3 = mixLeft0 + int(groupLabelWidth * 1 / 3);
  constexpr auto mixLeft2_per_3 = mixLeft0 + int(groupLabelWidth * 2 / 3);
  addCheckbox(
    mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);
  addCheckbox(
    mixLeft1_per_3, mixTop2, labelWidth, labelHeight, uiTextSize, "Release", ID::release);
  addCheckbox(
    mixLeft2_per_3, mixTop2, labelWidth, labelHeight, uiTextSize, "Fixed Noise",
    ID::resetSeedAtNoteOn);

  addLabel(mixLeft0, mixTop3, labelWidth, labelHeight, uiTextSize, "Spread");
  addTextKnob(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, ID::spreaderSpread,
    Scales::defaultScale, false, 5);
  addLabel(mixLeft0, mixTop4, labelWidth, labelHeight, uiTextSize, "Split [Hz]");
  addTextKnob(
    mixLeft1, mixTop4, labelWidth, labelHeight, uiTextSize, ID::spreaderSplitHz,
    Scales::cutoffFrequencyHz, false, 5);

  addToggleButton(
    mixLeft0, mixTop5, labelWidth, labelHeight, uiTextSize, "External Input [dB]",
    ID::useExternalInput);
  addTextKnob(
    mixLeft1, mixTop5, labelWidth, labelHeight, uiTextSize, ID::externalInputGain,
    Scales::gain, true, 5);

  // Tuning.
  constexpr auto tuningTop0 = mixTop5 + labelY;
  constexpr auto tuningTop1 = tuningTop0 + 1 * labelY;
  constexpr auto tuningTop2 = tuningTop0 + 2 * labelY;
  constexpr auto tuningTop3 = tuningTop0 + 3 * labelY;
  constexpr auto tuningTop4 = tuningTop0 + 4 * labelY;
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Note -> Pitch");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::notePitchAmount,
    Scales::bipolarScale, false, 5);
  addLabel(
    tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Transpose [st.]");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::transposeSemitone,
    Scales::semitone, false, 5);
  addLabel(
    tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize,
    "Pitch Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::semitone, false, 5);
  addLabel(
    tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

  // Velocity sensitivity.
  constexpr auto velocityTop0 = tuningTop4 + labelY;
  constexpr auto velocityTop1 = velocityTop0 + 1 * labelY;
  constexpr auto velocityTop2 = velocityTop0 + 2 * labelY;
  constexpr auto velocityTop3 = velocityTop0 + 3 * labelY;
  constexpr auto velocityTop4 = velocityTop0 + 4 * labelY;
  constexpr auto velocityTop5 = velocityTop0 + 5 * labelY;
  constexpr auto velocityTop6 = velocityTop0 + 6 * labelY;
  constexpr auto velocityTop7 = velocityTop0 + 7 * labelY;
  constexpr auto velocityTop8 = velocityTop0 + 8 * labelY;
  constexpr auto velocityLeft0 = left0;
  constexpr auto velocityLeft1 = velocityLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    velocityLeft0, velocityTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Velocity Sensitivity");

  addLabel(
    velocityLeft0, velocityTop1, labelWidth, labelHeight, uiTextSize, "> Output [dB]");
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

  addToggleButton(
    velocityLeft0, velocityTop5, groupLabelWidth, labelHeight, uiTextSize,
    "Use Note-off Velocity", ID::useNoteOffVelocityForClosing);
  addLabel(
    velocityLeft0, velocityTop6, labelWidth, labelHeight, uiTextSize,
    "> Closing Gain [dB]]");
  addTextKnob(
    velocityLeft1, velocityTop6, labelWidth, labelHeight, uiTextSize,
    ID::noteOffVelocityToClosingGain, Scales::velocityRangeDecibel, false, 5);
  addLabel(
    velocityLeft0, velocityTop7, labelWidth, labelHeight, uiTextSize,
    "> Closing Duration");
  addTextKnob(
    velocityLeft1, velocityTop7, labelWidth, labelHeight, uiTextSize,
    ID::noteOffVelocityToClosingDuration, Scales::bipolarScale, false, 5);

  // Noise & Envelope.
  constexpr auto envTop0 = top0 + 0 * labelY;
  constexpr auto envTop1 = envTop0 + 1 * labelY;
  constexpr auto envTop2 = envTop0 + 2 * labelY;
  constexpr auto envTop3 = envTop0 + 3 * labelY;
  constexpr auto envTop4 = envTop0 + 4 * labelY;
  constexpr auto envTop5 = envTop0 + 5 * labelY;
  constexpr auto envTop6 = envTop0 + 6 * labelY;
  constexpr auto envTop7 = envTop0 + 7 * labelY;
  constexpr auto envTop8 = envTop0 + 8 * labelY;
  constexpr auto envTop9 = envTop0 + 9 * labelY;
  constexpr auto envTop10 = envTop0 + 10 * labelY;
  constexpr auto envTop11 = envTop0 + 11 * labelY;
  constexpr auto envTop12 = envTop0 + 12 * labelY;
  constexpr auto envTop13 = envTop0 + 13 * labelY;
  constexpr auto envTop14 = envTop0 + 14 * labelY;
  constexpr auto envTop15 = envTop0 + 15 * labelY;
  constexpr auto envLeft0 = left4;
  constexpr auto envLeft1 = envLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    envLeft0, envTop0, groupLabelWidth, labelHeight, uiTextSize, "Impact Noise");

  addLabel(envLeft0, envTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    envLeft1, envTop1, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed, false,
    0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }
  addLabel(envLeft0, envTop2, labelWidth, labelHeight, uiTextSize, "Texture Mix");
  addTextKnob(
    envLeft1, envTop2, labelWidth, labelHeight, uiTextSize, ID::noiseTextureMix,
    Scales::defaultScale, false, 5);
  addLabel(envLeft0, envTop3, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    envLeft1, envTop3, labelWidth, labelHeight, uiTextSize, ID::noiseGain,
    Scales::halfClosedGain, true, 5);
  addLabel(envLeft0, envTop4, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    envLeft1, envTop4, labelWidth, labelHeight, uiTextSize, ID::noiseDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);

  addGroupLabel(
    envLeft0, envTop5, groupLabelWidth, labelHeight, uiTextSize, "Half Closed Noise");
  addLabel(envLeft0, envTop6, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    envLeft1, envTop6, labelWidth, labelHeight, uiTextSize, ID::halfClosedGain,
    Scales::halfClosedGain, true, 5);
  addLabel(envLeft0, envTop7, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    envLeft1, envTop7, labelWidth, labelHeight, uiTextSize, ID::halfCloseDecaySecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(envLeft0, envTop8, labelWidth, labelHeight, uiTextSize, "Sustain [dB]");
  addTextKnob(
    envLeft1, envTop8, labelWidth, labelHeight, uiTextSize, ID::halfCloseSustainLevel,
    Scales::halfClosedGain, true, 5);
  addLabel(envLeft0, envTop9, labelWidth, labelHeight, uiTextSize, "Pulse Duration [s]");
  addTextKnob(
    envLeft1, envTop9, labelWidth, labelHeight, uiTextSize, ID::halfClosedPulseSecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(envLeft0, envTop10, labelWidth, labelHeight, uiTextSize, "Density [Hz]");
  addTextKnob(
    envLeft1, envTop10, labelWidth, labelHeight, uiTextSize, ID::halfClosedDensityHz,
    Scales::halfClosedDensityHz, false, 5);
  addLabel(envLeft0, envTop11, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    envLeft1, envTop11, labelWidth, labelHeight, uiTextSize, ID::halfClosedHighpassHz,
    Scales::halfClosedDensityHz, false, 5);

  addGroupLabel(
    envLeft0, envTop12, groupLabelWidth, labelHeight, uiTextSize, "Closing Noise");
  addLabel(envLeft0, envTop13, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    envLeft1, envTop13, labelWidth, labelHeight, uiTextSize, ID::closingGain,
    Scales::halfClosedGain, true, 5);
  addLabel(envLeft0, envTop14, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    envLeft1, envTop14, labelWidth, labelHeight, uiTextSize, ID::closingAttackSecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(envLeft0, envTop15, labelWidth, labelHeight, uiTextSize, "Release Ratio");
  addTextKnob(
    envLeft1, envTop15, labelWidth, labelHeight, uiTextSize, ID::closingReleaseRatio,
    Scales::closingReleaseRatio, false, 5);

  // Randomize.
  constexpr auto randomTop0 = envTop15 + labelY;
  constexpr auto randomTop1 = randomTop0 + 1 * labelY;
  constexpr auto randomTop2 = randomTop0 + 2 * labelY;
  constexpr auto randomTop3 = randomTop0 + 3 * labelY;
  constexpr auto randomTop4 = randomTop0 + 4 * labelY;
  constexpr auto randomLeft0 = left4;
  constexpr auto randomLeft1 = randomLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    randomLeft0, randomTop0, groupLabelWidth, labelHeight, uiTextSize, "Randomize");

  addLabel(
    randomLeft0, randomTop1, labelWidth, labelHeight, uiTextSize,
    "> Half Closed Highpass");
  addTextKnob(
    randomLeft1, randomTop1, labelWidth, labelHeight, uiTextSize,
    ID::randomHalfClosedHighpass, Scales::defaultScale, false, 5);
  addLabel(
    randomLeft0, randomTop2, labelWidth, labelHeight, uiTextSize, "> Filter Cutoff");
  addTextKnob(
    randomLeft1, randomTop2, labelWidth, labelHeight, uiTextSize, ID::randomAllpassFilter,
    Scales::defaultScale, false, 5);

  // Allpass loop.
  constexpr auto loopTop0 = top0 + 0 * labelY;
  constexpr auto loopTop1 = loopTop0 + 1 * labelY;
  constexpr auto loopTop2 = loopTop0 + 2 * labelY;
  constexpr auto loopTop3 = loopTop0 + 3 * labelY;
  constexpr auto loopTop4 = loopTop0 + 4 * labelY;
  constexpr auto loopTop5 = loopTop0 + 5 * labelY;
  constexpr auto loopTop6 = loopTop0 + 6 * labelY;
  constexpr auto loopTop7 = loopTop0 + 7 * labelY;
  constexpr auto loopTop8 = loopTop0 + 8 * labelY;
  constexpr auto loopTop9 = loopTop0 + 9 * labelY;
  constexpr auto loopTop10 = loopTop0 + 10 * labelY;
  constexpr auto loopTop11 = loopTop0 + 11 * labelY;
  constexpr auto loopTop12 = loopTop0 + 12 * labelY;
  constexpr auto loopTop13 = loopTop0 + 13 * labelY;
  constexpr auto loopTop14 = loopTop0 + 14 * labelY;
  constexpr auto loopTop15 = loopTop0 + 15 * labelY;
  constexpr auto loopTop16 = loopTop0 + 16 * labelY;
  constexpr auto loopTop17 = loopTop0 + 17 * labelY;
  constexpr auto loopLeft0 = left8;
  constexpr auto loopLeft1 = loopLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    loopLeft0, loopTop0, groupLabelWidth, labelHeight, uiTextSize, "Allpass Loop");

  addLabel(loopLeft0, loopTop1, labelWidth, labelHeight, uiTextSize, "Character");
  addTextKnob(
    loopLeft1, loopTop1, labelWidth, labelHeight, uiTextSize, ID::delayTimeRandomRatio,
    Scales::defaultScale, false, 5);
  addLabel(loopLeft0, loopTop2, labelWidth, labelHeight, uiTextSize, "Pitch Ratio [st.]");
  addTextKnob(
    loopLeft1, loopTop2, labelWidth, labelHeight, uiTextSize, ID::delayTimeLoopRatio,
    Scales::semitone, false, 5);
  addLabel(
    loopLeft0, loopTop3, labelWidth, labelHeight, uiTextSize, "Modulation [sample]");
  addTextKnob(
    loopLeft1, loopTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);

  addLabel(loopLeft0, loopTop5, labelWidth, labelHeight, uiTextSize, "Delay Count 1");
  addTextKnob(
    loopLeft1, loopTop5, labelWidth, labelHeight, uiTextSize, ID::allpassDelayCount1,
    Scales::allpassDelayCount, false, 0, 1);
  addLabel(loopLeft0, loopTop6, labelWidth, labelHeight, uiTextSize, "Delay Count 2");
  addTextKnob(
    loopLeft1, loopTop6, labelWidth, labelHeight, uiTextSize, ID::allpassDelayCount2,
    Scales::allpassDelayCount, false, 0, 1);
  addLabel(loopLeft0, loopTop7, labelWidth, labelHeight, uiTextSize, "Feed 1");
  addTextKnob(
    loopLeft1, loopTop7, labelWidth, labelHeight, uiTextSize, ID::allpassFeed1,
    Scales::bipolarScale, false, 5);
  addLabel(loopLeft0, loopTop8, labelWidth, labelHeight, uiTextSize, "Feed 2");
  addTextKnob(
    loopLeft1, loopTop8, labelWidth, labelHeight, uiTextSize, ID::allpassFeed2,
    Scales::bipolarScale, false, 5);

  addLabel(loopLeft0, loopTop10, labelWidth, labelHeight, uiTextSize, "Mix Spike");
  addTextKnob(
    loopLeft1, loopTop10, labelWidth, labelHeight, uiTextSize, ID::allpassMixSpike,
    Scales::defaultScale, false, 5);
  addLabel(loopLeft0, loopTop11, labelWidth, labelHeight, uiTextSize, "Mix Alt. Sign");
  addTextKnob(
    loopLeft1, loopTop11, labelWidth, labelHeight, uiTextSize, ID::allpassMixAltSign,
    Scales::defaultScale, false, 5);

  // Filter.
  constexpr auto filterTop0 = loopTop12 + 0 * labelY;
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

  // Plugin name & randomize button.
  constexpr float splashHeight = int(labelHeight + 2 * margin);
  constexpr auto splashMargin = int(uiMargin / 2);
  constexpr auto splashTop = filterTop4 + labelY + int(labelHeight);
  constexpr auto splashLeft0 = left8;
  constexpr auto splashLeft1 = splashLeft0 + labelWidth + 2 * margin;
  auto panicButton = new RandomizeButton(
    CRect(splashLeft0, splashTop, splashLeft0 + labelWidth, splashTop + splashHeight),
    this, 0, "Random", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  addSplashScreen(
    splashLeft1, splashTop, labelWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "DoubleLoopCymbal", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
