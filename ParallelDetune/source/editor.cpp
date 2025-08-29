// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

enum tabIndex { tabTuning, tabTime, tabGain, tabHighpass, tabLowpass };

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

  // Shifter.
  const auto psTop0 = top0;
  const auto psTop1 = psTop0 + labelY;
  const auto psTop2 = psTop1 + knobY;
  const auto psTop3 = psTop2 + labelY;
  const auto psTop4 = psTop3 + labelY;
  const auto psTop5 = psTop4 + labelY + 2 * margin;
  const auto psLeft0 = left0;
  const auto psLeft1 = psLeft0 + knobX;
  const auto psLeft2 = psLeft1 + knobX;
  const auto psLeft3 = psLeft2 + knobX;
  const auto psLeft4 = psLeft3 + knobX;
  const auto psLabelWidth = int(1.5 * knobX) - margin;
  const auto psLabelLeft1 = psLeft0 + psLabelWidth;
  addGroupLabel(
    psLeft0, psTop0, 4 * knobX + 2 * margin + barboxWidth, labelHeight, uiTextSize,
    "Shifter");

  addKnob<Style::accent>(
    psLeft0, psTop1, knobWidth, margin, uiTextSize, "Dry", ID::dryGain);
  addKnob<Style::accent>(
    psLeft1, psTop1, knobWidth, margin, uiTextSize, "Wet", ID::wetGain);
  addKnob(psLeft2, psTop1, knobWidth, margin, uiTextSize, "Feedback", ID::feed);

  addLabel(psLeft0, psTop2, psLabelWidth, labelHeight, uiTextSize, "Delay Time [s]");
  addTextKnob(
    psLabelLeft1, psTop2, psLabelWidth, labelHeight, uiTextSize, ID::delayTimeSeconds,
    Scales::delayTimeSeconds, false, 5);
  addLabel(psLeft0, psTop3, psLabelWidth, labelHeight, uiTextSize, "Transpose [st.]");
  auto textKnobShiftTransposeSemitone = addTextKnob(
    psLabelLeft1, psTop3, psLabelWidth, labelHeight, uiTextSize,
    ID::shiftTransposeSemitone, Scales::shiftTransposeSemitone, false, 5);
  if (textKnobShiftTransposeSemitone) {
    double range
      = Scales::shiftTransposeSemitone.getMax() - Scales::shiftTransposeSemitone.getMin();
    textKnobShiftTransposeSemitone->sensitivity = 0.25 / range;
    textKnobShiftTransposeSemitone->lowSensitivity = 0.001 / range;
    textKnobShiftTransposeSemitone->wheelSensitivity = 1.0 / range;
  }
  addLabel(psLeft0, psTop4, psLabelWidth, labelHeight, uiTextSize, "Pan. Spread");
  addTextKnob(
    psLabelLeft1, psTop4, psLabelWidth, labelHeight, uiTextSize, ID::panSpread,
    Scales::defaultScale, false, 5);

  const auto psSmallMargin = int(halfKnobWidth * 1 / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop1, halfKnobWidth, labelHeight, uiTextSize, "HP");
  addSmallKnob(
    psLeft3 + psSmallMargin, psTop1 + labelHeight, labelHeight, labelHeight,
    ID::highpassHz);
  addLabel(psLeft3 + halfKnobWidth, psTop1, halfKnobWidth, labelHeight, uiTextSize, "LP");
  addSmallKnob(
    psLeft3 + psSmallMargin + halfKnobWidth, psTop1 + labelHeight, labelHeight,
    labelHeight, ID::lowpassNormalizedCutoff);

  const auto psSmallLeft = psLeft3 + int(knobWidth / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop2 - labelY, knobWidth, labelHeight, uiTextSize, "LFO");
  addSmallKnob(psSmallLeft, psTop2, labelHeight, labelHeight, ID::lfoToDelayTime);
  addSmallKnob(psSmallLeft, psTop3, labelHeight, labelHeight, ID::lfoToShiftPitch);
  addSmallKnob(psSmallLeft, psTop4, labelHeight, labelHeight, ID::lfoToPan);

  const auto tremoloOffset = halfKnobWidth + psSmallMargin;
  addLabel(psLeft0, psTop5, psLabelWidth, labelHeight, uiTextSize, "Tremolo");

  addLabel(
    psLeft1 + tremoloOffset, psTop5, halfKnobWidth, labelHeight, uiTextSize, "Mix");
  addSmallKnob(psLeft2 + psSmallMargin, psTop5, labelHeight, labelHeight, ID::tremoloMix);

  addLabel(
    psLeft2 + tremoloOffset, psTop5, halfKnobWidth, labelHeight, uiTextSize, "Lean");
  addSmallKnob(
    psLeft3 + psSmallMargin, psTop5, labelHeight, labelHeight, ID::tremoloLean);

  // Shifter TabView.
  const auto tabViewTop = psTop0;
  const auto tabViewLeft = psLeft4 + 2 * margin;
  const auto tabInsideTop0 = tabViewTop + labelY + uiMargin;
  const auto tabInsideLeft0 = tabViewLeft + uiMargin;
  const auto tabInsideLeft1 = tabInsideLeft0 + barboxWidth + uiMargin;

  std::vector<std::string> tabs{"Tuning", "Time", "Gain", "HP", "LP"};
  auto tabView = addTabView(
    tabViewLeft + uiMargin + barboxWidth, tabViewTop, 2 * uiMargin + barboxWidth,
    labelY + 2 * uiMargin + barboxHeight, uiTextSize, labelY, tabs);

  // Tuning tab.
  auto barboxShiftFineTuningCent = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::shiftFineTuningCent0,
    Synth::nShifter, Scales::shiftFineTuningCent, "Fine Tuning [cent]");
  if (barboxShiftFineTuningCent) {
    barboxShiftFineTuningCent->sliderZero = 0.5f;

    size_t nCent = std::ceil(
      Scales::shiftFineTuningCent.getMax() - Scales::shiftFineTuningCent.getMin());
    for (size_t idx = 0; idx < nCent; ++idx) {
      barboxShiftFineTuningCent->snapValue.push_back(double(idx) / double(nCent));
    }
  }
  tabView->addWidget(tabTuning, barboxShiftFineTuningCent);

  auto barboxShifterDelayTime = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight,
    ID::shifterDelayTimeMultiplier0, Synth::nShifter, Scales::delayTimeMultiplier,
    "Time Multiplier");
  if (barboxShifterDelayTime) {
    barboxShifterDelayTime->sliderZero = 0.5f;

    const size_t nSnap = 16;
    for (size_t idx = 0; idx < nSnap; ++idx) {
      barboxShifterDelayTime->snapValue.push_back(double(idx) / double(nSnap));
    }
  }
  tabView->addWidget(tabTime, barboxShifterDelayTime);

  auto barboxShifterGain = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::shifterGain0,
    Synth::nShifter, Scales::tremoloMix, "Gain");
  tabView->addWidget(tabGain, barboxShifterGain);

  auto barboxShifterHighpassOffset = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::shifterHighpassOffset0,
    Synth::nShifter, Scales::shiftOctave, "HP Offset [oct.]");
  if (barboxShifterHighpassOffset) {
    barboxShifterHighpassOffset->sliderZero = 0.5f;
  }
  tabView->addWidget(tabHighpass, barboxShifterHighpassOffset);

  auto barboxShifterLowpassOffset = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::shifterLowpassOffset0,
    Synth::nShifter, Scales::shiftOctave, "LP Offset [oct.]");
  if (barboxShifterLowpassOffset) {
    barboxShifterLowpassOffset->sliderZero = 0.5f;
  }
  tabView->addWidget(tabLowpass, barboxShifterLowpassOffset);

  // `barboxShiftOctave` is drawn over `tabView`.
  auto barboxShiftOctave = addBarBox(
    tabInsideLeft0 - uiMargin, tabInsideTop0, barboxWidth, barboxHeight,
    ID::shiftSemitone0, Synth::nShifter, Scales::shiftSemitone, "Pitch Shift [st.]");
  if (barboxShiftOctave) {
    barboxShiftOctave->sliderZero = 0.5f;

    size_t nOctave = size_t(
      std::ceil(Scales::shiftSemitone.getMax() - Scales::shiftSemitone.getMin()));
    for (size_t idx = 0; idx < nOctave; ++idx) {
      barboxShiftOctave->snapValue.push_back(double(idx) / double(nOctave));
    }
  }

  // LFO.
  const auto lfoTop0 = psTop0 + labelY + barboxHeight + 2 * uiMargin + 2 * margin;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + knobY;
  const auto lfoTop3 = lfoTop2 + labelY;
  const auto lfoLeft0 = psLeft0;
  const auto lfoLeft1 = lfoLeft0 + knobX;
  const auto lfoLeft2 = lfoLeft1 + knobX;
  const auto lfoLeft3 = lfoLeft2 + knobX;
  const auto lfoLeft4 = lfoLeft3 + knobX + 2 * margin;
  const auto lfoLabelLeft0 = lfoLeft0 + halfKnobWidth;
  const auto lfoLabelLeft1 = lfoLabelLeft0 + psLabelWidth;
  addGroupLabel(
    lfoLeft0, lfoTop0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "LFO");

  addRotaryKnob(
    lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "L-R Offset", ID::lfoPhaseOffset);
  addRotaryKnob(
    lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Phase", ID::lfoPhaseConstant);

  const auto lfoTopSync = lfoTop1 + 2 * labelHeight;
  addCheckbox(
    lfoLeft2, lfoTop1, knobWidth, labelHeight, uiTextSize, "Sync.", ID::lfoTempoSync);
  addTextKnob(
    lfoLeft2, lfoTopSync - 2 * margin, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoUpper, Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft2, lfoTopSync + 2 * margin, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoLower, Scales::lfoTempoLower, false, 0, 1);
  addKnob(lfoLeft3, lfoTop1, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate);

  addLabel(lfoLabelLeft0, lfoTop2, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  std::vector<std::string> lfoInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu(
    lfoLabelLeft1, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoInterpolation,
    lfoInterpolationItems);
  addLabel(lfoLabelLeft0, lfoTop3, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    lfoLabelLeft1, lfoTop3, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 5);

  auto barboxLfoWavetable = addBarBox(
    lfoLeft4, lfoTop0 + 2 * margin, 2 * barboxWidth + Synth::nLfoWavetable,
    barboxHeight + labelY, ID::lfoWavetable0, Synth::nLfoWavetable, Scales::bipolarScale,
    "LFO Wave");
  if (barboxLfoWavetable) {
    barboxLfoWavetable->sliderZero = 0.5f;

    const size_t nSnap = 16;
    for (size_t idx = 0; idx < nSnap; ++idx) {
      barboxLfoWavetable->snapValue.push_back(double(idx) / double(nSnap));
    }
  }

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashWidth = 2 * knobX - 2 * margin;
  const auto splashHeight = labelY;
  const auto splashTop = lfoTop3 + labelY + 2 * margin;
  const auto splashLeft = left0 + knobX;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "ParallelDetune");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  tabView->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
