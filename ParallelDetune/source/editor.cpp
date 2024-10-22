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

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 60.0f;
constexpr float halfKnobWidth = int(knobWidth / 2);
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 100.0f;
constexpr float labelX = labelWidth + margin;

constexpr float barboxWidth = 256.0f;
constexpr float barboxHeight = 2 * knobY;

constexpr int_least32_t defaultWidth
  = int_least32_t(5 * uiMargin + 4 * knobX + 2 * barboxWidth + 2 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 10 * labelY + 2 * knobY);

enum tabIndex { tabTuning, tabTime, tabGain, tabHighpass, tabLowpass };

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
  constexpr auto left0 = uiMargin;

  // Shifter.
  constexpr auto psTop0 = top0;
  constexpr auto psTop1 = psTop0 + labelY;
  constexpr auto psTop2 = psTop1 + knobY;
  constexpr auto psTop3 = psTop2 + labelY;
  constexpr auto psTop4 = psTop3 + labelY;
  constexpr auto psTop5 = psTop4 + labelY + 2 * margin;
  constexpr auto psLeft0 = left0;
  constexpr auto psLeft1 = psLeft0 + knobX;
  constexpr auto psLeft2 = psLeft1 + knobX;
  constexpr auto psLeft3 = psLeft2 + knobX;
  constexpr auto psLeft4 = psLeft3 + knobX;
  constexpr auto psLabelWidth = int(1.5 * knobX) - margin;
  constexpr auto psLabelLeft1 = psLeft0 + psLabelWidth;
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

  constexpr auto psSmallMargin = int(halfKnobWidth * 1 / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop1, halfKnobWidth, labelHeight, uiTextSize, "HP");
  addSmallKnob(
    psLeft3 + psSmallMargin, psTop1 + labelHeight, labelHeight, labelHeight,
    ID::highpassHz);
  addLabel(psLeft3 + halfKnobWidth, psTop1, halfKnobWidth, labelHeight, uiTextSize, "LP");
  addSmallKnob(
    psLeft3 + psSmallMargin + halfKnobWidth, psTop1 + labelHeight, labelHeight,
    labelHeight, ID::lowpassNormalizedCutoff);

  constexpr auto psSmallLeft = psLeft3 + int(knobWidth / 2 - labelHeight / 2);
  addLabel(psLeft3, psTop2 - labelY, knobWidth, labelHeight, uiTextSize, "LFO");
  addSmallKnob(psSmallLeft, psTop2, labelHeight, labelHeight, ID::lfoToDelayTime);
  addSmallKnob(psSmallLeft, psTop3, labelHeight, labelHeight, ID::lfoToShiftPitch);
  addSmallKnob(psSmallLeft, psTop4, labelHeight, labelHeight, ID::lfoToPan);

  constexpr auto tremoloOffset = halfKnobWidth + psSmallMargin;
  addLabel(psLeft0, psTop5, psLabelWidth, labelHeight, uiTextSize, "Tremolo");

  addLabel(
    psLeft1 + tremoloOffset, psTop5, halfKnobWidth, labelHeight, uiTextSize, "Mix");
  addSmallKnob(psLeft2 + psSmallMargin, psTop5, labelHeight, labelHeight, ID::tremoloMix);

  addLabel(
    psLeft2 + tremoloOffset, psTop5, halfKnobWidth, labelHeight, uiTextSize, "Lean");
  addSmallKnob(
    psLeft3 + psSmallMargin, psTop5, labelHeight, labelHeight, ID::tremoloLean);

  // Shifter TabView.
  constexpr auto tabViewTop = psTop0;
  constexpr auto tabViewLeft = psLeft4 + 2 * margin;
  constexpr auto tabInsideTop0 = tabViewTop + labelY + uiMargin;
  constexpr auto tabInsideLeft0 = tabViewLeft + uiMargin;
  constexpr auto tabInsideLeft1 = tabInsideLeft0 + barboxWidth + uiMargin;

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

    constexpr size_t nSnap = 16;
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
  constexpr auto lfoTop0 = psTop0 + labelY + barboxHeight + 2 * uiMargin + 2 * margin;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + knobY;
  constexpr auto lfoTop3 = lfoTop2 + labelY;
  constexpr auto lfoLeft0 = psLeft0;
  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX;
  constexpr auto lfoLeft3 = lfoLeft2 + knobX;
  constexpr auto lfoLeft4 = lfoLeft3 + knobX + 2 * margin;
  constexpr auto lfoLabelLeft0 = lfoLeft0 + halfKnobWidth;
  constexpr auto lfoLabelLeft1 = lfoLabelLeft0 + psLabelWidth;
  addGroupLabel(
    lfoLeft0, lfoTop0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "LFO");

  addRotaryKnob(
    lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "L-R Offset", ID::lfoPhaseOffset);
  addRotaryKnob(
    lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Phase", ID::lfoPhaseConstant);

  constexpr auto lfoTopSync = lfoTop1 + 2 * labelHeight;
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

    constexpr size_t nSnap = 16;
    for (size_t idx = 0; idx < nSnap; ++idx) {
      barboxLfoWavetable->snapValue.push_back(double(idx) / double(nSnap));
    }
  }

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = 2 * knobX - 2 * margin;
  constexpr auto splashHeight = labelY;
  constexpr auto splashTop = lfoTop3 + labelY + 2 * margin;
  constexpr auto splashLeft = left0 + knobX;
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
