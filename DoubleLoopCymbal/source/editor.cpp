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
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 160.0f;
constexpr float labelWidthHalf = 80.0f;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 3 * groupLabelWidth);
constexpr int_least32_t defaultHeight = int_least32_t(uiMargin + 18 * labelY);

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

  // Velocity Map.
  constexpr auto velocityTop0 = tuningTop4 + 1 * labelY;
  constexpr auto velocityTop1 = velocityTop0 + 1 * labelY;
  constexpr auto velocityTop2 = velocityTop0 + 2 * labelY;
  constexpr auto velocityTop3 = velocityTop0 + 3 * labelY;
  constexpr auto velocityTop4 = velocityTop0 + 4 * labelY;
  constexpr auto velocityTop5 = velocityTop0 + 5 * labelY;
  constexpr auto velocityTop6 = velocityTop0 + 6 * labelY;
  constexpr auto velocityLeft0 = left0;
  constexpr auto velocityLeft1 = velocityLeft0 + labelWidthHalf;
  constexpr auto velocityLeft2 = velocityLeft1 + labelWidthHalf + 2 * margin;
  constexpr auto velocityLeft3 = velocityLeft2 + labelWidthHalf;

  addGroupLabel(
    velocityLeft0, velocityTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Velocity Map");

  addLabel(
    velocityLeft0, velocityTop1, labelWidthHalf, labelHeight, uiTextSize,
    "Imp. Gain [dB]");
  addTextKnob(
    velocityLeft1, velocityTop1, labelWidthHalf, labelHeight, uiTextSize,
    ID::velocityToImpactGain, Scales::velocityRangeDecibel, false, 5);
  addLabel(
    velocityLeft0, velocityTop2, labelWidthHalf, labelHeight, uiTextSize,
    "Imp. Highpass");
  addTextKnob(
    velocityLeft1, velocityTop2, labelWidthHalf, labelHeight, uiTextSize,
    ID::velocityToImpactHighpass, Scales::bipolarScale, false, 5);

  addLabel(
    velocityLeft0, velocityTop3, labelWidthHalf, labelHeight, uiTextSize, "HC Density");
  addTextKnob(
    velocityLeft1, velocityTop3, labelWidthHalf, labelHeight, uiTextSize,
    ID::velocityToHalfClosedDensity, Scales::bipolarScale, false, 5);
  addLabel(
    velocityLeft0, velocityTop4, labelWidthHalf, labelHeight, uiTextSize, "HC Highpass");
  addTextKnob(
    velocityLeft1, velocityTop4, labelWidthHalf, labelHeight, uiTextSize,
    ID::velocityToHalfClosedHighpass, Scales::bipolarScale, false, 5);

  addToggleButton(
    velocityLeft2, velocityTop1, labelWidth, labelHeight, uiTextSize, "Note-off Velocity",
    ID::useNoteOffVelocityForClosing);
  addLabel(
    velocityLeft2, velocityTop2, labelWidthHalf, labelHeight, uiTextSize,
    "Cl. Gain [dB]");
  addTextKnob(
    velocityLeft3, velocityTop2, labelWidthHalf, labelHeight, uiTextSize,
    ID::noteOffVelocityToClosingGain, Scales::velocityRangeDecibel, false, 5);
  addLabel(
    velocityLeft2, velocityTop3, labelWidthHalf, labelHeight, uiTextSize, "Cl. Release");
  addTextKnob(
    velocityLeft3, velocityTop3, labelWidthHalf, labelHeight, uiTextSize,
    ID::noteOffVelocityToClosingReleaseRatio, Scales::bipolarScale, false, 5);
  addLabel(
    velocityLeft2, velocityTop4, labelWidthHalf, labelHeight, uiTextSize, "Cl. Highpass");
  addTextKnob(
    velocityLeft3, velocityTop4, labelWidthHalf, labelHeight, uiTextSize,
    ID::noteOffVelocityToClosingHighpass, Scales::bipolarScale, false, 5);

  addLabel(
    velocityLeft2, velocityTop6, labelWidthHalf, labelHeight, uiTextSize,
    "AP Modulation");
  addTextKnob(
    velocityLeft3, velocityTop6, labelWidthHalf, labelHeight, uiTextSize,
    ID::velocityToDelayTimeMod, Scales::delayTimeModAmount, false, 5);

  // Impact Noise.
  constexpr auto imTop0 = top0 + 0 * labelY;
  constexpr auto imTop1 = imTop0 + 1 * labelY;
  constexpr auto imTop2 = imTop0 + 2 * labelY;
  constexpr auto imTop3 = imTop0 + 3 * labelY;
  constexpr auto imTop4 = imTop0 + 4 * labelY;
  constexpr auto imTop5 = imTop0 + 5 * labelY;
  constexpr auto imLeft0 = left4;
  constexpr auto imLeft1 = imLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    imLeft0, imTop0, groupLabelWidth, labelHeight, uiTextSize, "Impact Noise");

  addLabel(imLeft0, imTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    imLeft1, imTop1, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed, false,
    0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }
  addLabel(imLeft0, imTop2, labelWidth, labelHeight, uiTextSize, "Texture Mix");
  addTextKnob(
    imLeft1, imTop2, labelWidth, labelHeight, uiTextSize, ID::impactTextureMix,
    Scales::defaultScale, false, 5);
  addLabel(imLeft0, imTop3, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    imLeft1, imTop3, labelWidth, labelHeight, uiTextSize, ID::impactGain,
    Scales::noiseGain, true, 5);
  addLabel(imLeft0, imTop4, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    imLeft1, imTop4, labelWidth, labelHeight, uiTextSize, ID::impactDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(imLeft0, imTop5, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    imLeft1, imTop5, labelWidth, labelHeight, uiTextSize, ID::impactHighpassHz,
    Scales::halfClosedDensityHz, false, 5);

  // Half Closed Noise.
  constexpr auto hcTop0 = imTop5 + 1 * labelY;
  constexpr auto hcTop1 = hcTop0 + 1 * labelY;
  constexpr auto hcTop2 = hcTop0 + 2 * labelY;
  constexpr auto hcTop3 = hcTop0 + 3 * labelY;
  constexpr auto hcTop4 = hcTop0 + 4 * labelY;
  constexpr auto hcTop5 = hcTop0 + 5 * labelY;
  constexpr auto hcTop6 = hcTop0 + 6 * labelY;
  constexpr auto hcLeft0 = left4;
  constexpr auto hcLeft1 = hcLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    hcLeft0, hcTop0, groupLabelWidth, labelHeight, uiTextSize, "Half Closed Noise");
  addLabel(hcLeft0, hcTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    hcLeft1, hcTop1, labelWidth, labelHeight, uiTextSize, ID::halfClosedGain,
    Scales::noiseGain, true, 5);
  addLabel(hcLeft0, hcTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    hcLeft1, hcTop2, labelWidth, labelHeight, uiTextSize, ID::halfClosedDecaySecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(hcLeft0, hcTop3, labelWidth, labelHeight, uiTextSize, "Sustain [dB]");
  addTextKnob(
    hcLeft1, hcTop3, labelWidth, labelHeight, uiTextSize, ID::halfClosedSustainLevel,
    Scales::noiseGain, true, 5);
  addLabel(hcLeft0, hcTop4, labelWidth, labelHeight, uiTextSize, "Pulse Duration [s]");
  addTextKnob(
    hcLeft1, hcTop4, labelWidth, labelHeight, uiTextSize, ID::halfClosedPulseSecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(hcLeft0, hcTop5, labelWidth, labelHeight, uiTextSize, "Density [Hz]");
  addTextKnob(
    hcLeft1, hcTop5, labelWidth, labelHeight, uiTextSize, ID::halfClosedDensityHz,
    Scales::halfClosedDensityHz, false, 5);
  addLabel(hcLeft0, hcTop6, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    hcLeft1, hcTop6, labelWidth, labelHeight, uiTextSize, ID::halfClosedHighpassHz,
    Scales::halfClosedDensityHz, false, 5);

  // Closing Noise.
  constexpr auto clTop0 = hcTop6 + 1 * labelY;
  constexpr auto clTop1 = clTop0 + 1 * labelY;
  constexpr auto clTop2 = clTop0 + 2 * labelY;
  constexpr auto clTop3 = clTop0 + 3 * labelY;
  constexpr auto clTop4 = clTop0 + 4 * labelY;
  constexpr auto clLeft0 = left4;
  constexpr auto clLeft1 = clLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    clLeft0, clTop0, groupLabelWidth, labelHeight, uiTextSize, "Closing Noise");
  addLabel(clLeft0, clTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    clLeft1, clTop1, labelWidth, labelHeight, uiTextSize, ID::closingGain,
    Scales::noiseGain, true, 5);
  addLabel(clLeft0, clTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    clLeft1, clTop2, labelWidth, labelHeight, uiTextSize, ID::closingAttackSecond,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(clLeft0, clTop3, labelWidth, labelHeight, uiTextSize, "Release Ratio");
  addTextKnob(
    clLeft1, clTop3, labelWidth, labelHeight, uiTextSize, ID::closingReleaseRatio,
    Scales::closingReleaseRatio, false, 5);
  addLabel(clLeft0, clTop4, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    clLeft1, clTop4, labelWidth, labelHeight, uiTextSize, ID::closingHighpassHz,
    Scales::halfClosedDensityHz, false, 5);

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

  addLabel(loopLeft0, loopTop9, labelWidth, labelHeight, uiTextSize, "Mix Spike");
  addTextKnob(
    loopLeft1, loopTop9, labelWidth, labelHeight, uiTextSize, ID::allpassMixSpike,
    Scales::defaultScale, false, 5);
  addLabel(loopLeft0, loopTop10, labelWidth, labelHeight, uiTextSize, "Mix Alt. Sign");
  addTextKnob(
    loopLeft1, loopTop10, labelWidth, labelHeight, uiTextSize, ID::allpassMixAltSign,
    Scales::defaultScale, false, 5);

  // Filter.
  constexpr auto filterTop0 = loopTop10 + 1 * labelY;
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
