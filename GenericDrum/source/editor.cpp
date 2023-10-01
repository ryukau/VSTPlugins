// (c) 2021-2023 Takamitsu Endo
//
// This file is part of GenericDrum.
//
// GenericDrum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenericDrum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenericDrum.  If not, see <https://www.gnu.org/licenses/>.

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
constexpr float knobWidth = 80.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 2 * knobWidth;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;
constexpr float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
constexpr float splashHeight = int(labelHeight * 3 / 2);

constexpr float barBoxWidth = groupLabelWidth;
constexpr float barBoxHeight = 5 * labelY - 2 * margin;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
constexpr float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 3 * groupLabelWidth);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 20 * labelY - 2 * margin);

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
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::tuningCent,
    Scales::cent, false, 5);
  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Equal Temp.");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::tuningET,
    Scales::equalTemperament, false, 0, 1);
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
  constexpr auto slideAtWidth = int(groupLabelWidth / 3);
  constexpr auto slideAtLeft1 = tuningLeft0 + 1 * slideAtWidth;
  constexpr auto slideAtLeft2 = tuningLeft0 + 2 * slideAtWidth;
  addLabel(tuningLeft0, tuningTop6, slideAtWidth, labelHeight, uiTextSize, "Slide at");
  addCheckbox(
    slideAtLeft1, tuningTop6, slideAtWidth, labelHeight, uiTextSize, "Note-on",
    ID::slideAtNoteOn);
  addCheckbox(
    slideAtLeft2, tuningTop6, slideAtWidth, labelHeight, uiTextSize, "Note-off",
    ID::slideAtNoteOff);

  // Impact.
  constexpr auto impactTop0 = top0 + 0 * labelY;
  constexpr auto impactTop1 = impactTop0 + 1 * labelY;
  constexpr auto impactTop2 = impactTop0 + 2 * labelY;
  constexpr auto impactTop3 = impactTop0 + 3 * labelY;
  constexpr auto impactTop4 = impactTop0 + 4 * labelY;
  constexpr auto impactLeft0 = left4;
  constexpr auto impactLeft1 = impactLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    impactLeft0, impactTop0, groupLabelWidth, labelHeight, uiTextSize, "Impact");

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
    impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Noise Lowpass [Hz]");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize, ID::noiseLowpassHz,
    Scales::delayTimeHz, false, 5);
  addLabel(impactLeft0, impactTop4, labelWidth, labelHeight, uiTextSize, "Echo [Hz]");
  addTextKnob(
    impactLeft1, impactTop4, labelWidth, labelHeight, uiTextSize,
    ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz, false, 5);

  // Wire.
  constexpr auto wireTop0 = top0 + 5 * labelY;
  constexpr auto wireTop1 = wireTop0 + 1 * labelY;
  constexpr auto wireTop2 = wireTop0 + 2 * labelY;
  constexpr auto wireTop3 = wireTop0 + 3 * labelY;
  constexpr auto wireTop4 = wireTop0 + 4 * labelY;
  constexpr auto wireTop5 = wireTop0 + 5 * labelY;
  constexpr auto wireTop6 = wireTop0 + 6 * labelY;
  constexpr auto wireTop7 = wireTop0 + 7 * labelY;
  constexpr auto wireLeft0 = left4;
  constexpr auto wireLeft1 = wireLeft0 + labelWidth + 2 * margin;
  addGroupLabel(wireLeft0, wireTop0, groupLabelWidth, labelHeight, uiTextSize, "Wire");

  addLabel(wireLeft0, wireTop1, labelWidth, labelHeight, uiTextSize, "Impact-Wire Mix");
  addTextKnob(
    wireLeft1, wireTop1, labelWidth, labelHeight, uiTextSize, ID::impactWireMix,
    Scales::defaultScale, false, 5);
  addLabel(wireLeft0, wireTop2, labelWidth, labelHeight, uiTextSize, "Membrane-Wire Mix");
  addTextKnob(
    wireLeft1, wireTop2, labelWidth, labelHeight, uiTextSize, ID::membraneWireMix,
    Scales::defaultScale, false, 5);
  addLabel(wireLeft0, wireTop3, labelWidth, labelHeight, uiTextSize, "Frequency [Hz]");
  addTextKnob(
    wireLeft1, wireTop3, labelWidth, labelHeight, uiTextSize, ID::wireFrequencyHz,
    Scales::wireFrequencyHz, false, 5);
  addLabel(wireLeft0, wireTop4, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    wireLeft1, wireTop4, labelWidth, labelHeight, uiTextSize, ID::wireDecaySeconds,
    Scales::wireDecaySeconds, false, 5);
  addLabel(
    wireLeft0, wireTop5, labelWidth, labelHeight, uiTextSize, "Collision Distance");
  addTextKnob(
    wireLeft1, wireTop5, labelWidth, labelHeight, uiTextSize, ID::wireDistance,
    Scales::collisionDistance, false, 5);
  addLabel(wireLeft0, wireTop6, labelWidth, labelHeight, uiTextSize, "Ruttle-Squeak Mix");
  addTextKnob(
    wireLeft1, wireTop6, labelWidth, labelHeight, uiTextSize, ID::wireCollisionTypeMix,
    Scales::defaultScale, false, 5);
  addLabel(
    wireLeft0, wireTop7, 2 * labelWidth, labelHeight, uiTextSize,
    "Wire collision status.");

  // Primary Membrane.
  constexpr auto primaryTop0 = top0 + 13 * labelY;
  constexpr auto primaryTop1 = primaryTop0 + 1 * labelY;
  constexpr auto primaryTop2 = primaryTop0 + 2 * labelY;
  constexpr auto primaryLeft0 = left4;
  constexpr auto primaryLeft1 = primaryLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    primaryLeft0, primaryTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Primary Membrane");

  addLabel(
    primaryLeft0, primaryTop1, labelWidth, labelHeight, uiTextSize,
    "Cross Feedback Gain [dB]");
  addTextKnob(
    primaryLeft1, primaryTop1, labelWidth, labelHeight, uiTextSize, ID::crossFeedbackGain,
    Scales::crossFeedbackGain, false, 5);
  addBarBox(
    primaryLeft0, primaryTop2, barBoxWidth, barBoxHeight, ID::crossFeedbackRatio0,
    maxFdnSize, Scales::defaultScale, "Cross Feedback Ratio");

  // Pitch Texture.
  constexpr auto textureTop0 = top0;
  constexpr auto textureTop1 = textureTop0 + 1 * labelY;
  constexpr auto textureTop2 = textureTop0 + 2 * labelY;
  constexpr auto textureTop3 = textureTop0 + 3 * labelY;
  constexpr auto textureLeft0 = left8;
  constexpr auto textureLeft1 = textureLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    textureLeft0, textureTop0, groupLabelWidth, labelHeight, uiTextSize, "Pitch Texture");

  addLabel(
    textureLeft0, textureTop1, labelWidth, labelHeight, uiTextSize, "Delay Time Spread");
  addTextKnob(
    textureLeft1, textureTop1, labelWidth, labelHeight, uiTextSize, ID::delayTimeSpread,
    Scales::defaultScale, false, 5);
  addLabel(
    textureLeft0, textureTop2, labelWidth, labelHeight, uiTextSize, "BP Cut Spread");
  addTextKnob(
    textureLeft1, textureTop2, labelWidth, labelHeight, uiTextSize, ID::bandpassCutSpread,
    Scales::defaultScale, false, 5);
  addLabel(
    textureLeft0, textureTop3, labelWidth, labelHeight, uiTextSize,
    "Pitch Random [cent]");
  addTextKnob(
    textureLeft1, textureTop3, labelWidth, labelHeight, uiTextSize, ID::pitchRandomCent,
    Scales::pitchRandomCent, false, 5);

  // Pitch Envelope.
  constexpr auto envTop0 = top0 + 4 * labelY;
  constexpr auto envTop1 = envTop0 + 1 * labelY;
  constexpr auto envTop2 = envTop0 + 2 * labelY;
  constexpr auto envTop3 = envTop0 + 3 * labelY;
  constexpr auto envLeft0 = left8;
  constexpr auto envLeft1 = envLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    envLeft0, envTop0, groupLabelWidth, labelHeight, uiTextSize, "Pitch Envelope");

  addLabel(envLeft0, envTop1, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    envLeft1, envTop1, labelWidth, labelHeight, uiTextSize, ID::envelopeAttackSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    envLeft1, envTop2, labelWidth, labelHeight, uiTextSize, ID::envelopeDecaySeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop3, labelWidth, labelHeight, uiTextSize, "Amount [oct]");
  addTextKnob(
    envLeft1, envTop3, labelWidth, labelHeight, uiTextSize, ID::envelopeModAmount,
    Scales::envelopeModAmount, false, 5);

  // Pitch Main.
  constexpr auto mainTop0 = top0 + 8 * labelY;
  constexpr auto mainTop1 = mainTop0 + 1 * labelY;
  constexpr auto mainTop2 = mainTop0 + 2 * labelY;
  constexpr auto mainTop3 = mainTop0 + 3 * labelY;
  constexpr auto mainTop4 = mainTop0 + 4 * labelY;
  constexpr auto mainTop5 = mainTop0 + 5 * labelY;
  constexpr auto mainLeft0 = left8;
  constexpr auto mainLeft1 = mainLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mainLeft0, mainTop0, groupLabelWidth, labelHeight, uiTextSize, "Pitch Main");

  addLabel(mainLeft0, mainTop1, labelWidth, labelHeight, uiTextSize, "Pitch Type");
  std::vector<std::string> pitchTypeItems{
    "Harmonic",
    "Harmonic+12",
    "Harmonic*5",
    "Harmonic Cycle(1, 5)",
    "Harmonic Odd",
    "Semitone (1, 2, 7, 9)",
    "Circular Membrane Mode",
    "Prime Number",
    "Octave",
  };
  addOptionMenu(
    mainLeft1, mainTop1, labelWidth, labelHeight, uiTextSize, ID::pitchType,
    {"Harmonic", "Harmonic+12", "Harmonic*5", "Harmonic Cycle(1, 5)", "Harmonic Odd",
     "Semitone (1, 2, 7, 9)", "Circular Membrane Mode", "Prime Number", "Octave"});
  addLabel(mainLeft0, mainTop2, labelWidth, labelHeight, uiTextSize, "Delay [Hz]");
  addTextKnob(
    mainLeft1, mainTop2, labelWidth, labelHeight, uiTextSize, ID::delayTimeHz,
    Scales::delayTimeHz, false, 5);
  addLabel(
    mainLeft0, mainTop3, labelWidth, labelHeight, uiTextSize,
    "Delay Moddulation [sample]");
  addTextKnob(
    mainLeft1, mainTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);
  addLabel(mainLeft0, mainTop4, labelWidth, labelHeight, uiTextSize, "BP Cut [oct]");
  addTextKnob(
    mainLeft1, mainTop4, labelWidth, labelHeight, uiTextSize, ID::bandpassCutRatio,
    Scales::bandpassCutRatio, false, 5);
  addLabel(mainLeft0, mainTop5, labelWidth, labelHeight, uiTextSize, "BP Q");
  addTextKnob(
    mainLeft1, mainTop5, labelWidth, labelHeight, uiTextSize, ID::bandpassQ,
    Scales::bandpassQ, false, 5);

  // Secondary Membrane.
  constexpr auto secondaryTop0 = top0 + 14 * labelY;
  constexpr auto secondaryTop1 = secondaryTop0 + 1 * labelY;
  constexpr auto secondaryTop2 = secondaryTop0 + 2 * labelY;
  constexpr auto secondaryTop3 = secondaryTop0 + 3 * labelY;
  constexpr auto secondaryTop4 = secondaryTop0 + 4 * labelY;
  constexpr auto secondaryTop5 = secondaryTop0 + 5 * labelY;
  constexpr auto secondaryLeft0 = left8;
  constexpr auto secondaryLeft1 = secondaryLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    secondaryLeft0, secondaryTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Secondary Membrane");

  addLabel(secondaryLeft0, secondaryTop1, labelWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob(
    secondaryLeft1, secondaryTop1, labelWidth, labelHeight, uiTextSize,
    ID::secondaryFdnMix, Scales::defaultScale, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop2, labelWidth, labelHeight, uiTextSize,
    "Pitch Offset [oct]");
  addTextKnob(
    secondaryLeft1, secondaryTop2, labelWidth, labelHeight, uiTextSize,
    ID::secondaryPitchOffset, Scales::bandpassCutRatio, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop3, labelWidth, labelHeight, uiTextSize, "Q Offset [oct]");
  addTextKnob(
    secondaryLeft1, secondaryTop3, labelWidth, labelHeight, uiTextSize,
    ID::secondaryQOffset, Scales::bandpassCutRatio, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop4, labelWidth, labelHeight, uiTextSize,
    "Collision Distance");
  addTextKnob(
    secondaryLeft1, secondaryTop4, labelWidth, labelHeight, uiTextSize,
    ID::secondaryDistance, Scales::collisionDistance, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop5, 2 * labelWidth, labelHeight, uiTextSize,
    "Membrane collision status.");

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = top0 + 13 * labelY + int(labelHeight / 4) + 2 * margin;
  constexpr auto splashLeft = left0 + int(labelWidth / 4);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "GenericDrum", true);

  return true;
}

} // namespace Vst
} // namespace Steinberg
