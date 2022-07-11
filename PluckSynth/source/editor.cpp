// (c) 2021 Takamitsu Endo
//
// This file is part of PluckSynth.
//
// PluckSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PluckSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PluckSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float uiMargin = 20.0f;
constexpr float margin = 5.0f;
constexpr float labelWidth = 80.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float splashHeight = 40.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float barboxWidth = 4 * labelWidth;
constexpr float barboxHeight = 200.0f;
constexpr float innerWidth = 16 * labelWidth + 10 * margin;
constexpr float innerHeight = 28 * labelY;
constexpr uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

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

  constexpr auto top0 = 20.0f;
  constexpr auto left0 = 20.0f;

  // Gain.
  constexpr auto gainLeft0 = left0;
  constexpr auto gainLeft1 = gainLeft0 + labelWidth;
  constexpr auto gainTop0 = top0;
  constexpr auto gainTop1 = gainTop0 + labelY;
  constexpr auto gainTop2 = gainTop1 + labelY;
  addGroupLabel(gainLeft0, gainTop0, 2 * labelWidth, labelHeight, midTextSize, "Gain");

  addLabel(gainLeft0, gainTop1, labelWidth, labelHeight, uiTextSize, "Gain");
  addTextKnob(
    gainLeft1, gainTop1, labelWidth, labelHeight, uiTextSize, ID::gain, Scales::gain,
    true, 5);

  addLabel(gainLeft0, gainTop2, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    gainLeft1, gainTop2, labelWidth, labelHeight, uiTextSize, ID::gateRelease,
    Scales::gateRelease, false, 5);

  // FDN.
  constexpr auto fdnLeft0 = left0;
  constexpr auto fdnLeft1 = gainLeft0 + labelWidth;
  constexpr auto fdnTop0 = gainTop2 + labelY;
  constexpr auto fdnTop1 = fdnTop0 + labelY;
  constexpr auto fdnTop2 = fdnTop1 + labelY;
  constexpr auto fdnTop3 = fdnTop2 + labelY;
  constexpr auto fdnTop4 = fdnTop3 + labelY;
  constexpr auto fdnTop5 = fdnTop4 + labelY;
  constexpr auto fdnTop6 = fdnTop5 + labelY;
  constexpr auto fdnTop7 = fdnTop6 + labelY;
  constexpr auto fdnTop8 = fdnTop7 + labelY;
  constexpr auto fdnTop9 = fdnTop8 + labelY;
  constexpr auto fdnTop10 = fdnTop9 + labelY;
  constexpr auto fdnTop11 = fdnTop10 + labelY;
  constexpr auto fdnTop12 = fdnTop11 + labelY;
  constexpr auto fdnTop13 = fdnTop12 + labelY;
  constexpr auto fdnTop14 = fdnTop13 + labelY;
  addToggleButton(
    fdnLeft0, fdnTop0, 2 * labelWidth, labelHeight, midTextSize, "FDN", ID::fdnEnable);

  addLabel(fdnLeft0, fdnTop1, labelWidth, labelHeight, uiTextSize, "Identity");
  addTextKnob(
    fdnLeft1, fdnTop1, labelWidth, labelHeight, uiTextSize, ID::fdnMatrixIdentityAmount,
    Scales::fdnMatrixIdentityAmount, false, 5);

  addLabel(fdnLeft0, fdnTop2, labelWidth, labelHeight, uiTextSize, "Feedback");
  addTextKnob(
    fdnLeft1, fdnTop2, labelWidth, labelHeight, uiTextSize, ID::fdnFeedback,
    Scales::defaultScale, false, 5);

  addLabel(fdnLeft0, fdnTop3, labelWidth, labelHeight, uiTextSize, "LP Cut [semi]");
  addTextKnob(
    fdnLeft1, fdnTop3, labelWidth, labelHeight, uiTextSize, ID::lowpassCutoffSemi,
    Scales::filterCutoffSemi, false, 5);
  addLabel(fdnLeft0, fdnTop4, labelWidth, labelHeight, uiTextSize, "LP Q");
  addTextKnob(
    fdnLeft1, fdnTop4, labelWidth, labelHeight, uiTextSize, ID::lowpassQ, Scales::filterQ,
    false, 5);

  addLabel(fdnLeft0, fdnTop5, labelWidth, labelHeight, uiTextSize, "HP Cut [semi]");
  addTextKnob(
    fdnLeft1, fdnTop5, labelWidth, labelHeight, uiTextSize, ID::highpassCutoffSemi,
    Scales::filterCutoffSemi, false, 5);
  addLabel(fdnLeft0, fdnTop6, labelWidth, labelHeight, uiTextSize, "HP Q");
  addTextKnob(
    fdnLeft1, fdnTop6, labelWidth, labelHeight, uiTextSize, ID::highpassQ,
    Scales::filterQ, false, 5);

  addLabel(fdnLeft0, fdnTop7, labelWidth, labelHeight, uiTextSize, "Key Follow");
  addCheckbox(
    fdnLeft1, fdnTop7, labelWidth, labelHeight, uiTextSize, "LP", ID::lowpassKeyFollow);
  addCheckbox(
    fdnLeft1 + labelWidth / 2, fdnTop7, labelWidth, labelHeight, uiTextSize, "HP",
    ID::highpassKeyFollow);

  // TODO: Better abbreviation for "overtone" to use in labels.
  addLabel(fdnLeft0, fdnTop8, labelWidth, labelHeight, uiTextSize, "OT +");
  addTextKnob(
    fdnLeft1, fdnTop8, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneAdd,
    Scales::fdnOvertoneAdd, false, 5);
  addLabel(fdnLeft0, fdnTop9, labelWidth, labelHeight, uiTextSize, "OT *");
  addTextKnob(
    fdnLeft1, fdnTop9, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneMul,
    Scales::fdnOvertoneMul, false, 5);
  addLabel(fdnLeft0, fdnTop10, labelWidth, labelHeight, uiTextSize, "OT offset");
  addTextKnob(
    fdnLeft1, fdnTop10, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneOffset,
    Scales::fdnOvertoneOffset, false, 5);

  addLabel(fdnLeft0, fdnTop11, labelWidth, labelHeight, uiTextSize, "Interp. Rate");
  addTextKnob(
    fdnLeft1, fdnTop11, labelWidth, labelHeight, uiTextSize, ID::fdnInterpRate,
    Scales::fdnInterpRate, false, 5);
  addLabel(fdnLeft0, fdnTop12, labelWidth, labelHeight, uiTextSize, "Interp. LP [s]");
  addTextKnob(
    fdnLeft1, fdnTop12, labelWidth, labelHeight, uiTextSize, ID::fdnInterpLowpassSecond,
    Scales::fdnInterpLowpassSecond, false, 5);

  addLabel(fdnLeft0, fdnTop13, labelWidth, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    fdnLeft1, fdnTop13, labelWidth, labelHeight, uiTextSize, ID::fdnSeed, Scales::seed,
    false, 0);
  addCheckbox(
    fdnLeft0, fdnTop14, 2 * labelWidth, labelHeight, uiTextSize, "Fixed Seed",
    ID::fdnFixedSeed);

  // Tuning.
  constexpr auto tuningLeft0 = gainLeft0 + 2 * labelWidth + 2 * margin;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth;
  constexpr auto tuningTop0 = gainTop0;
  constexpr auto tuningTop1 = tuningTop0 + labelY;
  constexpr auto tuningTop2 = tuningTop1 + labelY;
  constexpr auto tuningTop3 = tuningTop2 + labelY;
  constexpr auto tuningTop4 = tuningTop3 + labelY;
  constexpr auto tuningTop5 = tuningTop4 + labelY;
  constexpr auto tuningTop6 = tuningTop5 + labelY;
  addGroupLabel(
    tuningLeft0, tuningTop0, 2 * labelWidth, labelHeight, midTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::octave,
    Scales::octave, false, 0, -12);

  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semi");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::semitone,
    Scales::semitone, false, 0, -120);

  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Milli");
  auto knobOscMilli = addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::milli,
    Scales::milli, false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;

  addLabel(tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize, "ET");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::equalTemperament,
    Scales::equalTemperament, false, 0, 1);

  addLabel(tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::pitchA4Hz,
    Scales::pitchA4Hz, false, 0, 100);

  addLabel(tuningLeft0, tuningTop6, labelWidth, labelHeight, uiTextSize, "P.Bend Range");
  addTextKnob(
    tuningLeft1, tuningTop6, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);

  // Unison.
  constexpr auto unisonLeft0 = tuningLeft0;
  constexpr auto unisonLeft1 = unisonLeft0 + labelWidth;
  constexpr auto unisonTop0 = tuningTop6 + labelY;
  constexpr auto unisonTop1 = unisonTop0 + labelY;
  constexpr auto unisonTop2 = unisonTop1 + labelY;
  constexpr auto unisonTop3 = unisonTop2 + labelY;
  addGroupLabel(
    unisonLeft0, unisonTop0, 2 * labelWidth, labelHeight, midTextSize, "Unison");

  addLabel(unisonLeft0, unisonTop1, labelWidth, labelHeight, uiTextSize, "nUnison");
  addTextKnob(
    unisonLeft1, unisonTop1, labelWidth, labelHeight, uiTextSize, ID::nUnison,
    Scales::nUnison, false, 0, 1);

  addLabel(unisonLeft0, unisonTop2, labelWidth, labelHeight, uiTextSize, "Detune");
  addTextKnob(
    unisonLeft1, unisonTop2, labelWidth, labelHeight, uiTextSize, ID::unisonDetune,
    Scales::unisonDetune, false, 5);

  addLabel(unisonLeft0, unisonTop3, labelWidth, labelHeight, uiTextSize, "Pan");
  addTextKnob(
    unisonLeft1, unisonTop3, labelWidth, labelHeight, uiTextSize, ID::unisonPan,
    Scales::unisonPan, false, 5);

  // Oscillator.
  constexpr auto oscLeft0 = left0 + 4 * labelWidth + 4 * margin;
  constexpr auto oscLeft1 = oscLeft0 + labelWidth;
  constexpr auto oscTop0 = top0;
  constexpr auto oscTop1 = oscTop0 + labelY;
  constexpr auto oscTop2 = oscTop1 + labelY;
  constexpr auto oscTop3 = oscTop2 + labelY;
  constexpr auto oscTop4 = oscTop3 + labelY;
  constexpr auto oscTop5 = oscTop4 + labelY;
  constexpr auto oscTop6 = oscTop5 + labelY;
  constexpr auto oscTop7 = oscTop6 + labelY;
  constexpr auto oscTop8 = oscTop7 + labelY;
  constexpr auto oscTop9 = oscTop8 + labelY;
  constexpr auto oscTop10 = oscTop9 + labelY;
  constexpr auto oscTop11 = oscTop10 + labelY;
  constexpr auto oscTop12 = oscTop11 + labelY;
  constexpr auto oscTop13 = oscTop12 + labelY;
  addGroupLabel(
    oscLeft0, oscTop0, 2 * labelWidth, labelHeight, midTextSize, "Oscillator");

  addLabel(oscLeft0, oscTop1, labelWidth, labelHeight, uiTextSize, "Impulse [dB]");
  addTextKnob(
    oscLeft1, oscTop1, labelWidth, labelHeight, uiTextSize, ID::impulseGain,
    Scales::impulseGain, true, 5);
  addLabel(oscLeft0, oscTop2, labelWidth, labelHeight, uiTextSize, "Osc. [dB]");
  addTextKnob(
    oscLeft1, oscTop2, labelWidth, labelHeight, uiTextSize, ID::oscGain,
    Scales::impulseGain, true, 5);

  addLabel(oscLeft0, oscTop3, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    oscLeft1, oscTop3, labelWidth, labelHeight, uiTextSize, ID::oscOctave,
    Scales::oscOctave, false, 0, -12);
  addLabel(oscLeft0, oscTop4, labelWidth, labelHeight, uiTextSize, "Pitch");
  addTextKnob(
    oscLeft1, oscTop4, labelWidth, labelHeight, uiTextSize, ID::oscFinePitch,
    Scales::oscFinePitch, false, 5);

  addLabel(oscLeft0, oscTop5, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    oscLeft1, oscTop5, labelWidth, labelHeight, uiTextSize, ID::oscAttack,
    Scales::oscAttack, false, 5);
  addLabel(oscLeft0, oscTop6, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    oscLeft1, oscTop6, labelWidth, labelHeight, uiTextSize, ID::oscDecay,
    Scales::oscDecay, false, 5);

  addLabel(oscLeft0, oscTop7, labelWidth, labelHeight, uiTextSize, "Denom. Slope");
  addTextKnob(
    oscLeft1, oscTop7, labelWidth, labelHeight, uiTextSize,
    ID::oscSpectrumDenominatorSlope, Scales::oscSpectrumDenominatorSlope, false, 5);
  addLabel(oscLeft0, oscTop8, labelWidth, labelHeight, uiTextSize, "Rot. Slope");
  addTextKnob(
    oscLeft1, oscTop8, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumRotationSlope,
    Scales::oscSpectrumRotationSlope, false, 5);
  addLabel(oscLeft0, oscTop9, labelWidth, labelHeight, uiTextSize, "Rot. Offset");
  addTextKnob(
    oscLeft1, oscTop9, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumRotationOffset,
    Scales::defaultScale, false, 5);
  addLabel(oscLeft0, oscTop10, labelWidth, labelHeight, uiTextSize, "Interval");
  addTextKnob(
    oscLeft1, oscTop10, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumInterval,
    Scales::oscSpectrumInterval, false, 0, 1);
  addLabel(oscLeft0, oscTop11, labelWidth, labelHeight, uiTextSize, "Harmonic HP");
  addTextKnob(
    oscLeft1, oscTop11, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumHighpass,
    Scales::oscSpectrumHighpass, false, 0);
  addLabel(oscLeft0, oscTop12, labelWidth, labelHeight, uiTextSize, "Blur");
  addTextKnob(
    oscLeft1, oscTop12, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumBlur,
    Scales::oscSpectrumBlur, false, 5, 0);

  addKickButton<Style::warning>(
    oscLeft0, oscTop13, 2 * labelWidth, labelHeight, uiTextSize, "Refresh Wavetable",
    ID::refreshWavetable);

  // Overtone.
  constexpr auto overtoneLeft0 = oscLeft1 + labelWidth + 2 * margin;
  constexpr auto overtoneTop0 = top0;

  auto barboxOscOvertone = addBarBox(
    overtoneLeft0, overtoneTop0, barboxWidth, barboxHeight, ID::oscOvertone0,
    oscOvertoneSize, Scales::oscOvertone, "Osc Overtone");
  if (barboxOscOvertone) {
    barboxOscOvertone->sliderZero = 0.5f;
  }

  const auto lfoTop0 = overtoneTop0 + barboxHeight + 2 * margin;
  const auto lfoTop1 = lfoTop0 + labelY;
  const auto lfoTop2 = lfoTop1 + labelY;
  const auto lfoTop3 = lfoTop2 + labelY;
  const auto lfoTop4 = lfoTop3 + labelY;
  const auto lfoTop5 = lfoTop4 + labelY;

  const auto lfoLeft0 = overtoneLeft0;
  const auto lfoLeft1 = lfoLeft0 + labelWidth + margin;
  const auto lfoLeft2 = lfoLeft1 + labelWidth + margin;
  const auto lfoLeft3 = lfoLeft2 + labelWidth + margin;
  const auto lfoLeft4 = lfoLeft3 + labelWidth + margin;
  const auto lfoLeft5 = lfoLeft4 + labelWidth + margin;
  const auto lfoLeft6 = lfoLeft5 + labelWidth + margin;

  addGroupLabel(
    lfoLeft0, lfoTop0, 2 * labelWidth + 4 * margin, labelHeight, uiTextSize, "LFO");

  addCheckbox(
    lfoLeft0 + knobWidth / 2, lfoTop2, knobWidth, labelHeight, uiTextSize, "Sync.",
    ID::lfoTempoSync);
  addTextKnob(
    lfoLeft1, lfoTop1 + 4 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoUpper,
    Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob(
    lfoLeft1, lfoTop3 - 4 * margin, knobWidth, labelHeight, uiTextSize, ID::lfoTempoLower,
    Scales::lfoTempoLower, false, 0, 1);

  addLabel(lfoLeft2, lfoTop1, labelWidth, labelHeight, uiTextSize, "Rate", kCenterText);
  addTextKnob(
    lfoLeft3, lfoTop1, labelWidth, labelHeight, uiTextSize, ID::lfoRate, Scales::lfoRate,
    false, 5);

  addLabel(lfoLeft2, lfoTop2, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  std::vector<std::string> lfoInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu(
    lfoLeft3, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoInterpolation,
    lfoInterpolationItems);

  auto barboxLfoWavetable = addBarBox(
    lfoLeft0, lfoTop4, barboxWidth, barboxHeight, ID::lfoWavetable0, nLfoWavetable,
    Scales::lfoWavetable, "LFO Wave");
  if (barboxLfoWavetable) {
    barboxLfoWavetable->sliderZero = 0.5f;
  }

  addLabel(lfoLeft4, lfoTop2, labelWidth, labelHeight, uiTextSize, "To Osc. Pitch");
  addLabel(lfoLeft4, lfoTop3, labelWidth, labelHeight, uiTextSize, "To FDN Pitch");

  addLabel(lfoLeft5, lfoTop1, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob(
    lfoLeft5, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoToOscPitchAmount,
    Scales::lfoToPitchAmount, false, 5);
  addTextKnob<Style::warning>(
    lfoLeft5, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoToFdnPitchAmount,
    Scales::lfoToPitchAmount, false, 5);

  addLabel(lfoLeft6, lfoTop1, labelWidth, labelHeight, uiTextSize, "Alignment");
  addTextKnob(
    lfoLeft6, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoToOscPitchAlignment,
    Scales::lfoToPitchAlignment, false, 0);
  addTextKnob(
    lfoLeft6, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoToFdnPitchAlignment,
    Scales::lfoToPitchAlignment, false, 0);

  addCheckbox(
    lfoLeft4, lfoTop4, labelWidth, labelHeight, uiTextSize, "Retrigger",
    ID::lfoRetrigger);

  // Plugin name.
  const auto splashTop = innerHeight - splashHeight + uiMargin;
  const auto splashLeft = uiMargin;
  addSplashScreen(
    splashLeft, splashTop, 2 * labelWidth, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "PluckSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
