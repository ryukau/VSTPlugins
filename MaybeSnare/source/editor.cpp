// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

enum tabIndex { tabBatter, tabSnare };

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
  const auto left0 = uiMargin;
  const auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  const auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  // Gain.
  const auto gainTop0 = top0;
  const auto gainTop1 = gainTop0 + 1 * labelY;
  const auto gainTop2 = gainTop0 + 2 * labelY;
  const auto gainTop3 = gainTop0 + 3 * labelY;
  const auto gainLeft0 = left0;
  const auto gainLeft1 = gainLeft0 + labelWidth + 2 * margin;
  addGroupLabel(gainLeft0, gainTop0, groupLabelWidth, labelHeight, uiTextSize, "Gain");

  addLabel(gainLeft0, gainTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    gainLeft1, gainTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain,
    Scales::outputGain, true, 5);
  addLabel(gainLeft0, gainTop2, labelWidth, labelHeight, uiTextSize, "Batter-Snare Mix");
  addTextKnob(
    gainLeft1, gainTop2, labelWidth, labelHeight, uiTextSize, ID::fdnMix,
    Scales::defaultScale, false, 5);
  addCheckbox(
    gainLeft0, gainTop3, labelWidth, labelHeight, uiTextSize, "Normalize",
    ID::gainNormalization);
  addCheckbox(
    gainLeft1, gainTop3, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);

  // Tuning.
  const auto tuningTop0 = top0 + 4 * labelY;
  const auto tuningTop1 = tuningTop0 + 1 * labelY;
  const auto tuningTop2 = tuningTop0 + 2 * labelY;
  const auto tuningTop3 = tuningTop0 + 3 * labelY;
  const auto tuningTop4 = tuningTop0 + 4 * labelY;
  const auto tuningTop5 = tuningTop0 + 5 * labelY;
  const auto tuningTop6 = tuningTop0 + 6 * labelY;
  const auto tuningTop7 = tuningTop0 + 7 * labelY;
  const auto tuningTop8 = tuningTop0 + 8 * labelY;
  const auto tuningLeft0 = left0;
  const auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(
    tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Snare Side Octave");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize,
    ID::snareSideOctaveOffset, Scales::snareSideOctaveOffset, false, 5);
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
  const auto slideAtWidth = int(groupLabelWidth / 3);
  const auto slideAtLeft1 = tuningLeft0 + 1 * slideAtWidth;
  const auto slideAtLeft2 = tuningLeft0 + 2 * slideAtWidth;
  addLabel(tuningLeft0, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Slide at");
  addCheckbox(
    slideAtLeft1, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Note-on",
    ID::slideAtNoteOn);
  addCheckbox(
    slideAtLeft2, tuningTop8, slideAtWidth, labelHeight, uiTextSize, "Note-off",
    ID::slideAtNoteOff);

  // Impact.
  const auto impactTop0 = top0 + 0 * labelY;
  const auto impactTop1 = impactTop0 + 1 * labelY;
  const auto impactTop2 = impactTop0 + 2 * labelY;
  const auto impactTop3 = impactTop0 + 3 * labelY;
  const auto impactTop4 = impactTop0 + 4 * labelY;
  const auto impactTop5 = impactTop0 + 5 * labelY;
  const auto impactTop6 = impactTop0 + 6 * labelY;
  const auto impactLeft0 = left4;
  const auto impactLeft1 = impactLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    impactLeft0, impactTop0, groupLabelWidth, labelHeight, uiTextSize, "Impact");

  addLabel(
    impactLeft0, impactTop1, labelWidth, labelHeight, uiTextSize, "Amplitude [dB]");
  addTextKnob(
    impactLeft1, impactTop1, labelWidth, labelHeight, uiTextSize, ID::impactAmplitude,
    Scales::impactAmplitude, true, 5);
  addLabel(impactLeft0, impactTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    impactLeft1, impactTop2, labelWidth, labelHeight, uiTextSize, ID::impactDecaySeconds,
    Scales::impactDecaySeconds, false, 5);
  addLabel(impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Position");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize,
    ID::impactCenterRimPosition, Scales::defaultScale, false, 5);
  addLabel(impactLeft0, impactTop4, labelWidth, labelHeight, uiTextSize, "Noise Mix");
  addTextKnob(
    impactLeft1, impactTop4, labelWidth, labelHeight, uiTextSize, ID::impactNoiseMix,
    Scales::defaultScale, false, 5);
  addLabel(
    impactLeft0, impactTop5, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob(
    impactLeft1, impactTop5, labelWidth, labelHeight, uiTextSize,
    ID::impactNoiseDecaySeconds, Scales::impactDecaySeconds, false, 5);
  addLabel(impactLeft0, impactTop6, labelWidth, labelHeight, uiTextSize, "Noise LP [Hz]");
  addTextKnob(
    impactLeft1, impactTop6, labelWidth, labelHeight, uiTextSize,
    ID::impactNoiseLowpassHz, Scales::filterCutoffHz, false, 5);

  // Coupling
  const auto couplingTop0 = top0 + 7 * labelY;
  const auto couplingTop1 = couplingTop0 + 1 * labelY;
  const auto couplingTop2 = couplingTop0 + 2 * labelY;
  const auto couplingTop3 = couplingTop0 + 3 * labelY;
  const auto couplingLeft0 = left4;
  const auto couplingLeft1 = couplingLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    couplingLeft0, couplingTop0, groupLabelWidth, labelHeight, uiTextSize, "Coupling");

  addLabel(couplingLeft0, couplingTop1, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob<Style::warning>(
    couplingLeft1, couplingTop1, labelWidth, labelHeight, uiTextSize, ID::couplingAmount,
    Scales::couplingAmount, false, 5);
  addLabel(couplingLeft0, couplingTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob<Style::warning>(
    couplingLeft1, couplingTop2, labelWidth, labelHeight, uiTextSize,
    ID::couplingDecaySeconds, Scales::couplingDecaySeconds, false, 5);
  addLabel(couplingLeft0, couplingTop3, labelWidth, labelHeight, uiTextSize, "Reduction");
  addTextKnob<Style::warning>(
    couplingLeft1, couplingTop3, labelWidth, labelHeight, uiTextSize,
    ID::couplingSafetyReduction, Scales::fdnFeedback, false, 5);

  // Randomization.
  const auto randomTop0 = top0 + 11 * labelY;
  const auto randomTop1 = randomTop0 + 1 * labelY;
  const auto randomTop2 = randomTop0 + 2 * labelY;
  const auto randomTop3 = randomTop0 + 3 * labelY;
  const auto randomLeft0 = left4;
  const auto randomLeft1 = randomLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    randomLeft0, randomTop0, groupLabelWidth, labelHeight, uiTextSize, "Random");

  addLabel(randomLeft0, randomTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    randomLeft1, randomTop1, labelWidth, labelHeight, uiTextSize, ID::fdnSeed,
    Scales::seed);
  addLabel(randomLeft0, randomTop2, labelWidth, labelHeight, uiTextSize, "Matrix Rnd.");
  addTextKnob(
    randomLeft1, randomTop2, labelWidth, labelHeight, uiTextSize, ID::fdnRandomMatrix,
    Scales::defaultScale, false, 5);
  addLabel(randomLeft0, randomTop3, labelWidth, labelHeight, uiTextSize, "Overtone Rnd.");
  addTextKnob(
    randomLeft1, randomTop3, labelWidth, labelHeight, uiTextSize, ID::fdnRandomOvertone,
    Scales::defaultScale, false, 5);

  // Tab view.
  std::vector<std::string> tabs{"Batter Side", "Snare Side"};
  auto tabview
    = addTabView(left8, top0, tabViewWidth, tabViewHeight, uiTextSize, labelY, tabs);

  const auto tabInsideTop0 = top0 + labelY + uiMargin;
  const auto tabInsideTop1 = tabInsideTop0 + 1 * labelY;
  const auto tabInsideTop2 = tabInsideTop0 + 2 * labelY;
  const auto tabInsideTop3 = tabInsideTop0 + 3 * labelY;
  const auto tabInsideTop4 = tabInsideTop0 + 4 * labelY;
  const auto tabInsideTop5 = tabInsideTop0 + 5 * labelY;
  const auto tabInsideTop6 = tabInsideTop0 + 6 * labelY;
  const auto tabInsideTop7 = tabInsideTop0 + 7 * labelY;
  const auto tabInsideTop8 = tabInsideTop0 + 8 * labelY;
  const auto tabInsideTop9 = tabInsideTop0 + 9 * labelY;
  const auto tabInsideTop10 = tabInsideTop0 + 10 * labelY;
  const auto tabInsideLeft0 = left8 + uiMargin;
  const auto tabInsideLeft1 = tabInsideLeft0 + labelWidth + 2 * margin;
  const auto tabInsideLeft2 = tabInsideLeft1 + labelWidth + 4 * margin;
  const auto tabInsideLeft3 = tabInsideLeft2 + labelWidth + 2 * margin;

  // FDN Misc.
  addGroupLabel(
    tabInsideLeft0, tabInsideTop0, groupLabelWidth, labelHeight, uiTextSize, "Delay");

  addLabel(
    tabInsideLeft0, tabInsideTop1, labelWidth, labelHeight, uiTextSize, "Cross Feed");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft1, tabInsideTop1, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnMatrixIdentityAmount, Scales::fdnMatrixIdentityAmount, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft1, tabInsideTop1, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnMatrixIdentityAmount, Scales::fdnMatrixIdentityAmount, false, 5));

  addLabel(
    tabInsideLeft0, tabInsideTop2, labelWidth, labelHeight, uiTextSize, "Feedback");
  tabview->addWidget(
    tabBatter,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop2, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnFeedback, Scales::fdnFeedback, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop2, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnFeedback, Scales::fdnFeedback, false, 5));

  addLabel(tabInsideLeft0, tabInsideTop3, labelWidth, labelHeight, uiTextSize, "Shape");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft1, tabInsideTop3, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnShape, Scales::defaultScale, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft1, tabInsideTop3, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnShape, Scales::defaultScale, false, 5));

  //  FDN Modulation.
  addGroupLabel(
    tabInsideLeft0, tabInsideTop4, groupLabelWidth, labelHeight, uiTextSize,
    "Modulation");

  addLabel(tabInsideLeft0, tabInsideTop5, labelWidth, labelHeight, uiTextSize, "Amount");
  tabview->addWidget(
    tabBatter,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop5, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnModulationAmount, Scales::defaultScale, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop5, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnModulationAmount, Scales::defaultScale, false, 5));

  addLabel(
    tabInsideLeft0, tabInsideTop6, labelWidth, labelHeight, uiTextSize, "Interp. Rate");
  tabview->addWidget(
    tabBatter,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop6, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnInterpRate, Scales::fdnInterpRate, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob<Style::warning>(
      tabInsideLeft1, tabInsideTop6, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnInterpRate, Scales::fdnInterpRate, false, 5));

  addLabel(
    tabInsideLeft0, tabInsideTop7, labelWidth, labelHeight, uiTextSize, "Max Ratio");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft1, tabInsideTop7, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnMaxModulationRatio, Scales::fdnMaxModulationRatio, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft1, tabInsideTop7, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnMaxModulationRatio, Scales::fdnMaxModulationRatio, false, 5));

  tabview->addWidget(
    tabBatter,
    addToggleButton(
      tabInsideLeft0, tabInsideTop8, groupLabelWidth, labelHeight, uiTextSize,
      "Batter Mod. Envelope", ID::enableBatterModEnv));
  tabview->addWidget(
    tabSnare,
    addToggleButton(
      tabInsideLeft0, tabInsideTop8, groupLabelWidth, labelHeight, uiTextSize,
      "Snare Mod. Envelope", ID::enableSnareModEnv));

  addLabel(
    tabInsideLeft0, tabInsideTop9, labelWidth, labelHeight, uiTextSize, "Sustain [s]");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft1, tabInsideTop9, labelWidth, labelHeight, uiTextSize,
      ID::batterModEnvSustainSeconds, Scales::envelopeSeconds, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft1, tabInsideTop9, labelWidth, labelHeight, uiTextSize,
      ID::snareModEnvSustainSeconds, Scales::envelopeSeconds, false, 5));

  addLabel(
    tabInsideLeft0, tabInsideTop10, labelWidth, labelHeight, uiTextSize, "Release [s]");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft1, tabInsideTop10, labelWidth, labelHeight, uiTextSize,
      ID::batterModEnvReleaseSeconds, Scales::envelopeSeconds, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft1, tabInsideTop10, labelWidth, labelHeight, uiTextSize,
      ID::snareModEnvReleaseSeconds, Scales::envelopeSeconds, false, 5));

  // FDN Filter.
  addGroupLabel(
    tabInsideLeft2, tabInsideTop0, groupLabelWidth, labelHeight, uiTextSize, "Filter");

  addLabel(
    tabInsideLeft2, tabInsideTop1, labelWidth, labelHeight, uiTextSize, "LP Cut [Hz]");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft3, tabInsideTop1, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnLowpassCutoffHz, Scales::filterCutoffHz, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft3, tabInsideTop1, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnLowpassCutoffHz, Scales::filterCutoffHz, false, 5));

  tabview->addWidget(
    tabBatter,
    addBarBox(
      tabInsideLeft2, tabInsideTop2, barboxWidth, barboxHeight, ID::batterFdnLowpassQ0,
      fdnSize, Scales::filterQ, "LP Q"));
  tabview->addWidget(
    tabSnare,
    addBarBox(
      tabInsideLeft2, tabInsideTop2, barboxWidth, barboxHeight, ID::snareFdnLowpassQ0,
      fdnSize, Scales::filterQ, "LP Q"));

  addLabel(
    tabInsideLeft2, tabInsideTop7, labelWidth, labelHeight, uiTextSize, "HP Cut [Hz]");
  tabview->addWidget(
    tabBatter,
    addTextKnob(
      tabInsideLeft3, tabInsideTop7, labelWidth, labelHeight, uiTextSize,
      ID::batterFdnHighpassCutoffHz, Scales::filterCutoffHz, false, 5));
  tabview->addWidget(
    tabSnare,
    addTextKnob(
      tabInsideLeft3, tabInsideTop7, labelWidth, labelHeight, uiTextSize,
      ID::snareFdnHighpassCutoffHz, Scales::filterCutoffHz, false, 5));

  tabview->addWidget(
    tabBatter,
    addBarBox(
      tabInsideLeft2, tabInsideTop8, barboxWidth, barboxHeight, ID::batterFdnHighpassQ0,
      fdnSize, Scales::filterQ, "HP Q"));
  tabview->addWidget(
    tabSnare,
    addBarBox(
      tabInsideLeft2, tabInsideTop8, barboxWidth, barboxHeight, ID::snareFdnHighpassQ0,
      fdnSize, Scales::filterQ, "HP Q"));

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = top0 + 13 * labelY + int(labelHeight / 4) + 2 * margin;
  const auto splashLeft = left0 + int(labelWidth / 4);
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "MaybeSnare");

  tabview->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
