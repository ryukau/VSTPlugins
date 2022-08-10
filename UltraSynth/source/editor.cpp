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
constexpr float splashWidth = int(1.5 * labelWidth) + margin;
constexpr float splashHeight = labelY;

constexpr float barboxWidth = 500.0f;
constexpr float barboxHeight = 160.0f;

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 8 * knobX + 2 * margin);
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

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
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
  constexpr auto left4 = left3 + knobX;
  constexpr auto left5 = left4 + knobX;
  constexpr auto left6 = left5 + knobX + 2 * margin;
  constexpr auto left7 = left6 + knobX;

  // Gain.
  addGroupLabel(left0, top0, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Gain");
  addKnob(left0, top1, knobWidth, margin, uiTextSize, "Gain", ID::outputGain);
  addKnob(left0, top2, knobWidth, margin, uiTextSize, "Sat.", ID::saturationMix);
  addKnob(left1, top1, knobWidth, margin, uiTextSize, "A", ID::gainAttackSecond);
  addKnob(left1, top2, knobWidth, margin, uiTextSize, "D", ID::gainDecaySecond);
  addKnob(left1, top3, knobWidth, margin, uiTextSize, "S", ID::gainSustainAmplitude);
  addKnob(left1, top4, knobWidth, margin, uiTextSize, "R", ID::gainReleaseSecond);

  // Oscillator.
  addGroupLabel(
    left2, top0, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "Oscillator");
  addNumberKnob(
    left2, top1, knobWidth, margin, uiTextSize, "O1 Oct.", ID::osc1Octave, Scales::octave,
    -octaveOffset);
  addNumberKnob(
    left3, top1, knobWidth, margin, uiTextSize, "O1 OT", ID::osc1Overtone,
    Scales::overtone, 1);
  addKnob(left4, top1, knobWidth, margin, uiTextSize, "O1 Cent", ID::osc1FineTuneCent);
  addKnob(left5, top1, knobWidth, margin, uiTextSize, "O1 Shape", ID::osc1WaveShape);

  addNumberKnob(
    left2, top2, knobWidth, margin, uiTextSize, "O2 Oct.", ID::osc2Octave, Scales::octave,
    -octaveOffset);
  addNumberKnob(
    left3, top2, knobWidth, margin, uiTextSize, "O2 OT", ID::osc2Overtone,
    Scales::overtone, 1);
  addKnob(left4, top2, knobWidth, margin, uiTextSize, "O2 Cent", ID::osc2FineTuneCent);
  addKnob(left5, top2, knobWidth, margin, uiTextSize, "O2 Shape", ID::osc2WaveShape);

  addKnob(left2, top3, knobWidth, margin, uiTextSize, "Mix", ID::oscMix);

  addKnob(
    left2, top4, knobWidth, margin, uiTextSize, "LP>O1", ID::phaseModFromLowpassToOsc1);
  addKnob(
    left3, top4, knobWidth, margin, uiTextSize, "O1>O2", ID::phaseModFromOsc1ToOsc2);
  addKnob(
    left4, top4, knobWidth, margin, uiTextSize, "O2>O1", ID::phaseModFromOsc2ToOsc1);

  // Filter.
  addGroupLabel(left6, top0, 2 * knobX - 2 * margin, labelHeight, uiTextSize, "Filter");
  addKnob(left6, top1, knobWidth, margin, uiTextSize, "Cut", ID::lowpassCutoffHz);
  addKnob(left6, top2, knobWidth, margin, uiTextSize, "Q", ID::lowpassQ);
  addKnob(left6, top3, knobWidth, margin, uiTextSize, "Rect.", ID::lowpassRectification);

  addKnob(left7, top1, knobWidth, margin, uiTextSize, "A", ID::lowpassCutoffAttackSecond);
  addKnob(left7, top2, knobWidth, margin, uiTextSize, "D", ID::lowpassCutoffDecaySecond);
  addKnob(
    left7, top3, knobWidth, margin, uiTextSize, "Env>Cut",
    ID::lowpassCutoffEnvelopeAmount);
  addKnob(left7, top4, knobWidth, margin, uiTextSize, "Key", ID::lowpassKeyFollow);

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

  // Misc.
  auto miscLeft0 = left4 + 2 * margin;
  auto miscLeft1 = miscLeft0 + 2 * knobX;
  addGroupLabel(
    miscLeft0, top5, 4 * knobX - 2 * margin, labelHeight, uiTextSize, "Misc.");

  addCheckbox(
    miscLeft0, top6, labelWidth, labelHeight, uiTextSize, "Phase Reset",
    ID::resetPhaseAtNoteOn);
  addLabel(miscLeft0, top7, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    miscLeft1, top7, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

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
