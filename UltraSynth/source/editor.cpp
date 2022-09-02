// (c) 2021-2022 Takamitsu Endo
//
// This file is part of UltraSynth.
//
// UltraSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// UltraSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with UltraSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

// 480 + 20
constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 2 * knobWidth;
constexpr float splashWidth = labelWidth + margin;
constexpr float splashHeight = labelY;

constexpr float barboxWidth = 500.0f;
constexpr float barboxHeight = 160.0f;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 10 * knobX + 3 * smallKnobX + 2 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 4 * knobY + 7 * labelY - 2 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::syncUI(ParamID id, float normalized)
{
  auto syncer = xyControlMap.find(id);
  if (syncer == xyControlMap.end()) return;
  syncer->second->sync(normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  PlugEditor::valueChanged(pControl);
  syncUI(pControl->getTag(), pControl->getValueNormalized());
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);
  syncUI(id, normalized);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto top1 = top0 + labelY;
  constexpr auto top2 = top1 + knobY;
  constexpr auto top3 = top2 + knobY;
  constexpr auto top4 = top3 + knobY;
  constexpr auto top5 = top4 + knobY;
  constexpr auto top6 = top5 + labelY;
  constexpr auto top7 = top6 + labelY;
  constexpr auto top8 = top7 + labelY;
  constexpr auto top9 = top8 + labelY;
  constexpr auto top10 = top9 + labelY;
  constexpr auto left0 = uiMargin;
  constexpr auto left1 = left0 + knobX;
  constexpr auto left2 = left1 + knobX + 2 * margin;
  constexpr auto left3 = left2 + knobX;
  constexpr auto left4 = left3 + knobX + smallKnobX;
  constexpr auto left5 = left4 + knobX;
  constexpr auto left6 = left5 + knobX + smallKnobX;
  constexpr auto left7 = left6 + knobX;
  constexpr auto left8 = left7 + knobX + 2 * margin + smallKnobX;
  constexpr auto left9 = left8 + knobX;

  // Gain.
  addGroupLabel(left0, top0, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Gain");

  addKnob(left0, top1, knobWidth, margin, uiTextSize, "Gain", ID::outputGain);
  addKnob(left0, top3, knobWidth, margin, uiTextSize, "Rect.", ID::rectificationMix);
  addKnob(left0, top4, knobWidth, margin, uiTextSize, "Sat.", ID::saturationMix);

  addKnob(left1, top1, knobWidth, margin, uiTextSize, "A", ID::gainAttackSecond);
  addKnob(left1, top2, knobWidth, margin, uiTextSize, "D", ID::gainDecaySecond);
  addKnob(left1, top3, knobWidth, margin, uiTextSize, "S", ID::gainSustainAmplitude);
  addKnob(left1, top4, knobWidth, margin, uiTextSize, "R", ID::gainReleaseSecond);

  // Oscillator.
  constexpr auto oscLeft1Half = left2 + int(knobX / 2);
  addGroupLabel(
    left2, top0, 6 * knobX + 3 * smallKnobX - 2 * margin, labelHeight, uiTextSize,
    "Oscillator");

  addLabel(left2, top1, labelWidth, labelHeight, uiTextSize, "Pitch");
  addXYControls(
    left2, top1 + labelY, 2 * knobX - 2 * margin, smallKnobWidth, margin, uiTextSize,
    ID::osc1FineTuneCent, ID::osc2FineTuneCent);

  addNumberKnob(
    left2, top3, knobWidth, margin, uiTextSize, "X Oct.", ID::osc1Octave, Scales::octave,
    -octaveOffset);
  addNumberKnob(
    left3, top3, knobWidth, margin, uiTextSize, "Y Oct.", ID::osc2Octave, Scales::octave,
    -octaveOffset);
  addKnob(left2, top4, knobWidth, margin, uiTextSize, "Mix", ID::oscMix);
  addKnob(
    left3, top4, knobWidth, margin, uiTextSize, "LP>Osc.X",
    ID::phaseModFromLowpassToOsc1);

  addLabel(left4, top1, labelWidth, labelHeight, uiTextSize, "Wave Shape");
  addXYControls(
    left4, top1 + labelY, 2 * knobX - 2 * margin, smallKnobWidth, margin, uiTextSize,
    ID::osc1WaveShape, ID::osc2WaveShape);
  addLabel(left6, top1, labelWidth, labelHeight, uiTextSize, "Saw-Pulse");
  addXYControls(
    left6, top1 + labelY, 2 * knobX - 2 * margin, smallKnobWidth, margin, uiTextSize,
    ID::osc1SawPulseMix, ID::osc2SawPulseMix);

  addLabel(left4, top3, labelWidth, labelHeight, uiTextSize, "Cross PM");
  addXYControls(
    left4, top3 + labelY, 2 * knobX - 2 * margin, smallKnobWidth, margin, uiTextSize,
    ID::pmPhase1ToPhase2, ID::pmPhase2ToPhase1);
  addLabel(left6, top3, labelWidth, labelHeight, uiTextSize, "Feedback PM");
  addXYControls(
    left6, top3 + labelY, 2 * knobX - 2 * margin, smallKnobWidth, margin, uiTextSize,
    ID::pmOsc1ToPhase2, ID::pmOsc2ToPhase1);

  // Filter.
  constexpr auto filterTop1Half = top1 + int(knobY / 2);
  constexpr auto filterTop2Half = filterTop1Half + knobY;
  addGroupLabel(left8, top0, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Filter");

  addKnob(
    left8, filterTop1Half, knobWidth, margin, uiTextSize, "Cut", ID::lowpassCutoffHz);
  addKnob(left8, filterTop2Half, knobWidth, margin, uiTextSize, "Q", ID::lowpassQ);

  addKnob(left9, top1, knobWidth, margin, uiTextSize, "A", ID::lowpassCutoffAttackSecond);
  addKnob(left9, top2, knobWidth, margin, uiTextSize, "D", ID::lowpassCutoffDecaySecond);
  addKnob(
    left9, top3, knobWidth, margin, uiTextSize, "Env>Cut",
    ID::lowpassCutoffEnvelopeAmount);
  addKnob(left9, top4, knobWidth, margin, uiTextSize, "Key", ID::lowpassKeyFollow);

  // Tuning.
  addGroupLabel(left0, top5, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "Tuning");

  addLabel(left0, top6, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    left2, top6, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(left0, top7, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    left2, top7, labelWidth, labelHeight, uiTextSize, ID::tuningCent, Scales::cent, false,
    5);
  addLabel(left0, top8, labelWidth, labelHeight, uiTextSize, "Equal Temp.");
  addTextKnob(
    left2, top8, labelWidth, labelHeight, uiTextSize, ID::tuningET,
    Scales::equalTemperament, false, 0, 1);
  addLabel(left0, top9, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    left2, top9, labelWidth, labelHeight, uiTextSize, ID::tuningA4Hz, Scales::a4Hz, false,
    0, a4HzOffset);
  addLabel(left0, top10, labelWidth, labelHeight, uiTextSize, "P.Bend Range [st.]");
  addTextKnob(
    left2, top10, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);

  // LFO.
  constexpr auto lfoLeft0 = left4;
  addGroupLabel(
    lfoLeft0, top5, 4 * knobX + smallKnobX - 2 * margin, labelHeight, uiTextSize, "LFO");

  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX + smallKnobX;
  constexpr auto lfoLeft3 = lfoLeft2 + knobX;
  addSmallKnob(
    lfoLeft0, top6, knobX, labelHeight, margin, uiTextSize, "Rate", ID::lfoRate);
  addSmallKnob(
    lfoLeft1 + 2 * margin, top6, knobX, labelHeight, margin, uiTextSize, "Shape",
    ID::lfoWaveShape);
  addCheckbox(
    lfoLeft0, top7, labelWidth, labelHeight, uiTextSize, "Retrigger", ID::lfoRetrigger);
  addCheckbox(
    lfoLeft2, top6 + int(labelY) / 2, knobWidth, labelHeight, uiTextSize, "Sync.",
    ID::lfoTempoSync);
  addTextKnob(
    lfoLeft3, top6 + margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoUpper,
    Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft3, top7 - margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoLower,
    Scales::lfoTempoLower, false, 0, 1);

  addSmallKnob(
    lfoLeft0, top8, labelWidth, labelHeight, margin, uiTextSize, ">Pitch",
    ID::lfoToPitch);
  addSmallKnob(
    lfoLeft0, top9, labelWidth, labelHeight, margin, uiTextSize, ">OscMix",
    ID::lfoToOscMix);
  addSmallKnob(
    lfoLeft0, top10, labelWidth, labelHeight, margin, uiTextSize, ">Cutoff",
    ID::lfoToCutoff);

  addSmallKnob(
    lfoLeft2, top8, labelWidth, labelHeight, margin, uiTextSize, ">Pre Sat.",
    ID::lfoToPreSaturation);
  addSmallKnob(
    lfoLeft2, top9, labelWidth, labelHeight, margin, uiTextSize, ">O1 Shape",
    ID::lfoToOsc1WaveShape);
  addSmallKnob(
    lfoLeft2, top10, labelWidth, labelHeight, margin, uiTextSize, ">O2 Shape",
    ID::lfoToOsc2WaveShape);

  // Misc.
  constexpr auto miscLeft0 = left8;
  constexpr auto miscLeft0Half = miscLeft0 + int(knobX / 2);
  addGroupLabel(
    miscLeft0, top5, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Misc.");

  addCheckbox(
    miscLeft0 + 3 * margin, top6, labelWidth - 3 * margin, labelHeight, uiTextSize,
    "Phase Reset", ID::resetPhaseAtNoteOn);
  addKnob(
    miscLeft0Half, top7, knobWidth, margin, uiTextSize, "Slide", ID::noteSlideTimeSecond);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = defaultHeight - uiMargin - splashHeight;
  constexpr auto splashLeft = defaultWidth - uiMargin - splashWidth;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "UltraSynth");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
