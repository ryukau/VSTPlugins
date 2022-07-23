// (c) 2022 Takamitsu Endo
//
// This file is part of ClangSynth.
//
// ClangSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 14.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float uiMargin = 20.0f;
constexpr float margin = 5.0f;
constexpr float checkBoxWidth = 10.0f;
constexpr float labelWidth = 80.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float splashHeight = 40.0f;
constexpr float barboxWidth = int(64) * int(1 + (7 * labelWidth + 6 * margin) / 64);
constexpr float barboxHeight = 8 * labelHeight + 14 * margin;
constexpr float lfoWidthFix = 25.0f;
constexpr float innerWidth = 6 * labelWidth + 10 * margin + barboxWidth;
constexpr float innerHeight = 8 * labelY + 2 * barboxHeight + 9 * margin;
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
  constexpr auto gainTop3 = gainTop2 + labelY;
  addGroupLabel(gainLeft0, gainTop0, 2 * labelWidth, labelHeight, uiTextSize, "Gain");

  addLabel(gainLeft0, gainTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob<Style::accent>(
    gainLeft1, gainTop1, labelWidth, labelHeight, uiTextSize, ID::gain, Scales::gain,
    true, 5);
  addLabel(gainLeft0, gainTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob<Style::accent>(
    gainLeft1, gainTop2, labelWidth, labelHeight, uiTextSize, ID::gateAttackSecond,
    Scales::gateAttackSecond, false, 5);
  addLabel(gainLeft0, gainTop3, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob<Style::accent>(
    gainLeft1, gainTop3, labelWidth, labelHeight, uiTextSize, ID::gateReleaseSecond,
    Scales::gateReleaseSecond, false, 5);

  // Tuning.
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth;
  constexpr auto tuningTop0 = gainTop3 + labelY;
  constexpr auto tuningTop1 = tuningTop0 + labelY;
  constexpr auto tuningTop2 = tuningTop1 + labelY;
  constexpr auto tuningTop3 = tuningTop2 + labelY;
  constexpr auto tuningTop4 = tuningTop3 + labelY;
  constexpr auto tuningTop5 = tuningTop4 + labelY;
  constexpr auto tuningTop6 = tuningTop5 + labelY;
  addGroupLabel(
    tuningLeft0, tuningTop0, 2 * labelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob<Style::accent>(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::octave,
    Scales::octave, false, 0, -12);

  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob<Style::accent>(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::semitone,
    Scales::semitone, false, 0, -120);

  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Milli");
  auto knobOscMilli = addTextKnob<Style::accent>(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::milli,
    Scales::milli, false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;

  addLabel(tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize, "ET");
  addTextKnob<Style::accent>(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::equalTemperament,
    Scales::equalTemperament, false, 0, 1);

  addLabel(tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob<Style::accent>(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::pitchA4Hz,
    Scales::pitchA4Hz, false, 0, 100);

  addLabel(tuningLeft0, tuningTop6, labelWidth, labelHeight, uiTextSize, "P.Bend Range");
  addTextKnob<Style::accent>(
    tuningLeft1, tuningTop6, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);

  // Unison.
  constexpr auto unisonLeft0 = tuningLeft0;
  constexpr auto unisonLeft1 = unisonLeft0 + labelWidth;
  constexpr auto unisonTop0 = tuningTop6 + labelY;
  constexpr auto unisonTop1 = unisonTop0 + labelY;
  constexpr auto unisonTop2 = unisonTop1 + labelY;
  constexpr auto unisonTop3 = unisonTop2 + labelY + 2 * margin;
  constexpr auto unisonTop4 = unisonTop3 + labelY;
  constexpr auto unisonTop5 = unisonTop4 + labelY + 2 * margin;
  constexpr auto unisonTop6 = unisonTop5 + labelY;
  constexpr auto unisonTop7 = unisonTop6 + labelY + 2 * margin;
  addGroupLabel(
    unisonLeft0, unisonTop0, 2 * labelWidth, labelHeight, uiTextSize, "Unison/Chord");

  addLabel(unisonLeft0, unisonTop1, labelWidth, labelHeight, uiTextSize, "nUnison");
  addTextKnob(
    unisonLeft1, unisonTop1, labelWidth, labelHeight, uiTextSize, ID::nUnison,
    Scales::nUnison, false, 0, 1);
  addLabel(unisonLeft0, unisonTop2, labelWidth, labelHeight, uiTextSize, "Pan");
  addTextKnob(
    unisonLeft1, unisonTop2, labelWidth, labelHeight, uiTextSize, ID::unisonPan,
    Scales::unisonPan, false, 5);

  addLabel(unisonLeft0, unisonTop3, labelWidth, labelHeight, uiTextSize, "Pitch *");
  addTextKnob(
    unisonLeft1, unisonTop3, labelWidth, labelHeight, uiTextSize, ID::unisonPitchMul,
    Scales::unisonPitchMul, false, 5);
  addLabel(unisonLeft0, unisonTop4, labelWidth, labelHeight, uiTextSize, "ET");
  addTextKnob(
    unisonLeft1, unisonTop4, labelWidth, labelHeight, uiTextSize,
    ID::unisonEqualTemperament, Scales::equalTemperament, false, 0, 1);

  addLabel(
    unisonLeft0, unisonTop5, labelWidth, labelHeight, uiTextSize, "Interval [st.]");
  auto halfLabelWidth = int(labelWidth / 2);
  for (size_t idx = 0; idx < nUnisonInterval; ++idx) {
    addTextKnob(
      unisonLeft0 + idx * halfLabelWidth, unisonTop6, halfLabelWidth, labelHeight,
      uiTextSize, ID::unisonIntervalSemitone0 + idx, Scales::unisonIntervalSemitone,
      false, 0);
  }
  addLabel(unisonLeft0, unisonTop7, labelWidth, labelHeight, uiTextSize, "Cycle At");
  auto unisonIntervalCycleAtTextKnob = addTextKnob(
    unisonLeft1, unisonTop7, labelWidth, labelHeight, uiTextSize,
    ID::unisonIntervalCycleAt, Scales::unisonIntervalCycleAt, false, 0);
  if (unisonIntervalCycleAtTextKnob) {
    unisonIntervalCycleAtTextKnob->setSensitivity(1.0 / 4.0, 1.0 / 32.0, 1.0 / 4.0);
  }

  // Misc.
  constexpr auto miscLeft0 = tuningLeft0;
  constexpr auto miscLeft1 = miscLeft0 + labelWidth;
  constexpr auto miscTop0 = unisonTop7 + labelY;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscTop2 = miscTop1 + labelY;

  addGroupLabel(miscLeft0, miscTop0, 2 * labelWidth, labelHeight, uiTextSize, "Misc.");

  addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "nVoice");
  addTextKnob(
    miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize, ID::nVoice, Scales::nVoice,
    false, 0, 1);

  addLabel(miscLeft0, miscTop2, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop2, labelWidth, labelHeight, uiTextSize, ID::smoothingTimeSecond,
    Scales::smoothingTimeSecond, false, 5);

  // Oscillator.
  constexpr auto oscLeft0 = gainLeft0 + 2 * labelWidth + 4 * margin;
  constexpr auto oscLeft1 = oscLeft0 + labelWidth;
  constexpr auto oscLeft2 = oscLeft1 + labelWidth + 2 * margin;
  constexpr auto oscLeft3 = oscLeft2 + labelWidth;

  constexpr auto oscTop0 = top0;
  constexpr auto oscTop1 = oscTop0 + labelY;
  constexpr auto oscTop2 = oscTop1 + labelY;
  constexpr auto oscTop3 = oscTop2 + labelY;
  constexpr auto oscTop4 = oscTop3 + labelY;
  constexpr auto oscTop5 = oscTop4 + labelY;
  constexpr auto oscTop6 = oscTop5 + labelY;
  constexpr auto oscTop7 = oscTop6 + labelY + 2 * margin;
  constexpr auto oscTop8 = oscTop7 + 4 * labelY;
  addGroupLabel(
    oscLeft0, oscTop0, 4 * labelWidth + 2 * margin, labelHeight, uiTextSize,
    "Oscillator");

  addLabel(oscLeft0, oscTop1, labelWidth, labelHeight, uiTextSize, "Impulse [dB]");
  addTextKnob(
    oscLeft1, oscTop1, labelWidth, labelHeight, uiTextSize, ID::impulseGain,
    Scales::impulseGain, true, 5);
  addLabel(oscLeft0, oscTop2, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    oscLeft1, oscTop2, labelWidth, labelHeight, uiTextSize, ID::oscGain,
    Scales::impulseGain, true, 5);
  addLabel(oscLeft0, oscTop3, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    oscLeft1, oscTop3, labelWidth, labelHeight, uiTextSize, ID::oscAttack,
    Scales::oscAttack, false, 5);
  addLabel(oscLeft0, oscTop4, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    oscLeft1, oscTop4, labelWidth, labelHeight, uiTextSize, ID::oscDecay,
    Scales::oscDecay, false, 5);
  addLabel(oscLeft0, oscTop5, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob<Style::accent>(
    oscLeft1, oscTop5, labelWidth, labelHeight, uiTextSize, ID::oscOctave,
    Scales::oscOctave, false, 0, -12);
  addLabel(oscLeft0, oscTop6, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob<Style::accent>(
    oscLeft1, oscTop6, labelWidth, labelHeight, uiTextSize, ID::oscFinePitch,
    Scales::oscFinePitch, false, 5);

  addLabel(oscLeft2, oscTop1, labelWidth, labelHeight, uiTextSize, "Denom. Slope");
  addTextKnob(
    oscLeft3, oscTop1, labelWidth, labelHeight, uiTextSize,
    ID::oscSpectrumDenominatorSlope, Scales::oscSpectrumDenominatorSlope, false, 5);
  addLabel(oscLeft2, oscTop2, labelWidth, labelHeight, uiTextSize, "Rot. Slope");
  addTextKnob(
    oscLeft3, oscTop2, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumRotationSlope,
    Scales::oscSpectrumRotationSlope, false, 5);
  addLabel(oscLeft2, oscTop3, labelWidth, labelHeight, uiTextSize, "Rot. Offset");
  addTextKnob(
    oscLeft3, oscTop3, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumRotationOffset,
    Scales::defaultScale, false, 5);
  addLabel(oscLeft2, oscTop4, labelWidth, labelHeight, uiTextSize, "Interval");
  addTextKnob(
    oscLeft3, oscTop4, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumInterval,
    Scales::oscSpectrumInterval, false, 0, 1);
  addLabel(oscLeft2, oscTop5, labelWidth, labelHeight, uiTextSize, "Harmonic HP");
  addTextKnob(
    oscLeft3, oscTop5, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumHighpass,
    Scales::oscSpectrumHighpass, false, 0);
  addLabel(oscLeft2, oscTop6, labelWidth, labelHeight, uiTextSize, "Blur");
  addTextKnob(
    oscLeft3, oscTop6, labelWidth, labelHeight, uiTextSize, ID::oscSpectrumBlur,
    Scales::oscSpectrumBlur, false, 5, 0);

  auto barboxOscOvertone = addBarBox(
    oscLeft0, oscTop7, 2 * labelWidth, 4 * labelHeight + 6 * margin, ID::oscOvertone0,
    oscOvertoneSize, Scales::oscOvertone, "OT Amp.");
  if (barboxOscOvertone) {
    barboxOscOvertone->sliderZero = 0.5f;
  }
  auto barboxOscRotation = addBarBox(
    oscLeft2, oscTop7, 2 * labelWidth, 4 * labelHeight + 6 * margin, ID::oscRotation0,
    oscOvertoneSize, Scales::oscOvertone, "Rot. [rad/pi]");
  if (barboxOscRotation) {
    barboxOscRotation->sliderZero = 0.5f;
  }

  addKickButton<Style::warning>(
    oscLeft1 + 2 * margin, oscTop8 + labelHeight / 2, 2 * labelWidth - 2 * margin,
    2 * labelHeight, midTextSize, "Refresh Wavetable", ID::refreshWavetable);

  // FDN.
  constexpr auto fdnLeft0 = oscLeft0;
  constexpr auto fdnLeft1 = fdnLeft0 + labelWidth;
  constexpr auto fdnLeft2 = fdnLeft1 + labelWidth + 2 * margin;
  constexpr auto fdnLeft3 = fdnLeft2 + labelWidth;

  constexpr auto fdnTop0 = oscTop8 + 3 * labelHeight + 2 * margin;
  constexpr auto fdnTop1 = fdnTop0 + labelY;
  constexpr auto fdnTop2 = fdnTop1 + labelY;
  constexpr auto fdnTop3 = fdnTop2 + labelY;
  constexpr auto fdnTop4 = fdnTop3 + labelY;
  constexpr auto fdnTop5 = fdnTop4 + labelY;
  constexpr auto fdnTop6 = fdnTop5 + labelY;
  constexpr auto fdnTop7 = fdnTop6 + labelY + 1 * margin;
  constexpr auto fdnTop8 = fdnTop7 + labelY;
  constexpr auto fdnTop9 = fdnTop8 + labelY;
  constexpr auto fdnTop10 = fdnTop9 + labelY;
  addToggleButton(
    fdnLeft0, fdnTop0, 4 * labelWidth + 2 * margin, labelHeight, uiTextSize, "FDN",
    ID::fdnEnable);

  addLabel(fdnLeft0, fdnTop1, labelWidth, labelHeight, uiTextSize, "Identity");
  addTextKnob(
    fdnLeft1, fdnTop1, labelWidth, labelHeight, uiTextSize, ID::fdnMatrixIdentityAmount,
    Scales::fdnMatrixIdentityAmount, false, 5);
  addLabel(fdnLeft0, fdnTop2, labelWidth, labelHeight, uiTextSize, "Feedback");
  addTextKnob<Style::accent>(
    fdnLeft1, fdnTop2, labelWidth, labelHeight, uiTextSize, ID::fdnFeedback,
    Scales::fdnFeedback, false, 5);
  addLabel(fdnLeft0, fdnTop3, labelWidth, labelHeight, uiTextSize, "Interp. Rate");
  addTextKnob<Style::accent>(
    fdnLeft1, fdnTop3, labelWidth, labelHeight, uiTextSize, ID::fdnInterpRate,
    Scales::fdnInterpRate, false, 5);
  addLabel(fdnLeft0, fdnTop4, labelWidth, labelHeight, uiTextSize, "Interp. LP [s]");
  addTextKnob<Style::accent>(
    fdnLeft1, fdnTop4, labelWidth, labelHeight, uiTextSize, ID::fdnInterpLowpassSecond,
    Scales::fdnInterpLowpassSecond, false, 5);
  addLabel(fdnLeft0, fdnTop5, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    fdnLeft1, fdnTop5, labelWidth, labelHeight, uiTextSize, ID::fdnSeed, Scales::seed,
    false, 0);
  if (seedTextKnob) {
    seedTextKnob->setSensitivity(
      2048 / double(1 << 24), 1 / double(1 << 24), 1 / double(1 << 24));
  }
  addLabel(fdnLeft0, fdnTop6, labelWidth, labelHeight, uiTextSize, "Randomize");
  addTextKnob(
    fdnLeft1, fdnTop6, labelWidth, labelHeight, uiTextSize, ID::fdnRandomizeRatio,
    Scales::defaultScale, false, 5);

  // TODO: Better abbreviation for "overtone" to use in labels.
  addLabel(fdnLeft2, fdnTop1, labelWidth, labelHeight, uiTextSize, "OT +");
  addTextKnob<Style::accent>(
    fdnLeft3, fdnTop1, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneAdd,
    Scales::fdnOvertoneAdd, false, 5);
  addLabel(fdnLeft2, fdnTop2, labelWidth, labelHeight, uiTextSize, "OT *");
  addTextKnob<Style::accent>(
    fdnLeft3, fdnTop2, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneMul,
    Scales::fdnOvertoneMul, false, 5);
  addLabel(fdnLeft2, fdnTop3, labelWidth, labelHeight, uiTextSize, "OT Offset");
  addTextKnob<Style::accent>(
    fdnLeft3, fdnTop3, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneOffset,
    Scales::fdnOvertoneOffset, false, 5);
  addLabel(fdnLeft2, fdnTop4, labelWidth, labelHeight, uiTextSize, "OT Modulo");
  addTextKnob<Style::accent>(
    fdnLeft3, fdnTop4, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneModulo,
    Scales::fdnOvertoneModulo, false, 5);
  addLabel(fdnLeft2, fdnTop5, labelWidth, labelHeight, uiTextSize, "OT Random");
  addTextKnob(
    fdnLeft3, fdnTop5, labelWidth, labelHeight, uiTextSize, ID::fdnOvertoneRandomness,
    Scales::defaultScale, false, 5);
  addCheckbox(
    fdnLeft2, fdnTop6, 2 * labelWidth, labelHeight, uiTextSize, "Reset at Note On",
    ID::resetAtNoteOn);

  constexpr auto fdnFilterLeft0 = fdnLeft0 + int(labelWidth / 4);
  constexpr auto fdnFilterLeft1 = fdnFilterLeft0 + labelWidth + 2 * margin;
  constexpr auto fdnFilterLeft2 = fdnFilterLeft1 + labelWidth + 2 * margin;

  addLabel(fdnFilterLeft1, fdnTop7, labelWidth, labelHeight, uiTextSize, "Lowpass");
  addLabel(fdnFilterLeft2, fdnTop7, labelWidth, labelHeight, uiTextSize, "Highpass");

  addLabel(fdnFilterLeft0, fdnTop8, labelWidth, labelHeight, uiTextSize, "Cutoff [st.]");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop8, labelWidth, labelHeight, uiTextSize, ID::lowpassCutoffSemi,
    Scales::filterCutoffSemi, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop8, labelWidth, labelHeight, uiTextSize, ID::highpassCutoffSemi,
    Scales::filterCutoffSemi, false, 5);

  addLabel(fdnFilterLeft0, fdnTop9, labelWidth, labelHeight, uiTextSize, "Q");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop9, labelWidth, labelHeight, uiTextSize, ID::lowpassQ,
    Scales::filterQ, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop9, labelWidth, labelHeight, uiTextSize, ID::highpassQ,
    Scales::filterQ, false, 5);

  addLabel(fdnFilterLeft0, fdnTop10, labelWidth, labelHeight, uiTextSize, "Key Follow");
  addCheckbox(
    fdnFilterLeft1 + int((labelWidth - checkBoxWidth) / 2), fdnTop10, labelWidth,
    labelHeight, uiTextSize, "", ID::lowpassKeyFollow);
  addCheckbox(
    fdnFilterLeft2 + int((labelWidth - checkBoxWidth) / 2), fdnTop10, labelWidth,
    labelHeight, uiTextSize, "", ID::highpassKeyFollow);

  // LFO.
  constexpr auto lfoTop0 = oscTop0;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + labelY;
  constexpr auto lfoTop3 = lfoTop2 + labelY;
  constexpr auto lfoTop4 = lfoTop3 + labelY + 2 * margin;

  constexpr auto lfoTopMid1 = lfoTop1 + int(labelHeight / 2) + margin;
  constexpr auto lfoTopMid2 = lfoTopMid1 + labelY;

  constexpr auto lfoLeft0 = oscLeft0 + 4 * labelWidth + 6 * margin;
  constexpr auto lfoLeft1 = lfoLeft0 + labelWidth + margin;
  constexpr auto lfoLeft2 = lfoLeft1 + labelWidth + margin + lfoWidthFix;
  constexpr auto lfoLeft3 = lfoLeft2 + labelWidth;
  constexpr auto lfoLeft4 = lfoLeft3 + labelWidth + +margin + lfoWidthFix;
  constexpr auto lfoLeft5 = lfoLeft4 + labelWidth + margin;
  constexpr auto lfoLeft6 = lfoLeft5 + labelWidth + margin;

  addGroupLabel(lfoLeft0, lfoTop0, barboxWidth, labelHeight, uiTextSize, "LFO");

  addCheckbox(
    lfoLeft0 + int(labelWidth / 8), lfoTopMid1, labelWidth, labelHeight, uiTextSize,
    "Retrigger", ID::lfoRetrigger);
  addLabel(lfoLeft0, lfoTopMid2, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  std::vector<std::string> modWavetableInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu(
    lfoLeft1, lfoTopMid2, labelWidth, labelHeight, uiTextSize, ID::lfoInterpolation,
    modWavetableInterpolationItems);

  constexpr auto lfoSyncTop = lfoTop1 + int(labelHeight / 2) + margin;
  addCheckbox<Style::accent>(
    lfoLeft2 + int(labelWidth / 4), lfoSyncTop, int(labelWidth / 2 + 2 * margin),
    labelHeight, uiTextSize, "Sync.", ID::lfoTempoSync);
  addTextKnob<Style::accent>(
    lfoLeft3, lfoSyncTop - 2 * margin, int(labelWidth / 2 + 2 * margin), labelHeight,
    uiTextSize, ID::lfoTempoUpper, Scales::lfoTempoUpper, false, 0, 1);
  addTextKnob<Style::accent>(
    lfoLeft3, lfoSyncTop + 2 * margin, int(labelWidth / 2 + 2 * margin), labelHeight,
    uiTextSize, ID::lfoTempoLower, Scales::lfoTempoLower, false, 0, 1);

  addLabel(
    lfoLeft2, lfoTop3, labelWidth - 2 * margin, labelHeight, uiTextSize, "Rate",
    kCenterText);
  addTextKnob<Style::accent>(
    lfoLeft3 - 2 * margin, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoRate,
    Scales::lfoRate, false, 5);

  addLabel(lfoLeft5, lfoTop1, labelWidth, labelHeight, uiTextSize, "Amount");
  addLabel(lfoLeft6, lfoTop1, labelWidth, labelHeight, uiTextSize, "Alignment");

  addLabel(lfoLeft4, lfoTop2, labelWidth, labelHeight, uiTextSize, "> Osc. Pitch");
  auto lfoToOscPitchAmountTextKnob = addTextKnob<Style::accent>(
    lfoLeft5, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoToOscPitchAmount,
    Scales::lfoToPitchAmount, false, 5);
  if (lfoToOscPitchAmountTextKnob) {
    lfoToOscPitchAmountTextKnob->setSensitivity(
      float(1) / float(2400), float(1) / float(24000), float(1) / float(240000));
  }
  addTextKnob<Style::accent>(
    lfoLeft6, lfoTop2, labelWidth, labelHeight, uiTextSize, ID::lfoToOscPitchAlignment,
    Scales::lfoToPitchAlignment, false, 0);

  addLabel(lfoLeft4, lfoTop3, labelWidth, labelHeight, uiTextSize, "> FDN Pitch");
  auto lfoToFdnPitchAmountTextKnob = addTextKnob<Style::warning>(
    lfoLeft5, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoToFdnPitchAmount,
    Scales::lfoToPitchAmount, false, 5);
  if (lfoToFdnPitchAmountTextKnob) {
    lfoToFdnPitchAmountTextKnob->setSensitivity(
      float(1) / float(2400), float(1) / float(24000), float(1) / float(240000));
  }
  addTextKnob<Style::accent>(
    lfoLeft6, lfoTop3, labelWidth, labelHeight, uiTextSize, ID::lfoToFdnPitchAlignment,
    Scales::lfoToPitchAlignment, false, 0);

  auto barboxLfoWavetable = addBarBox(
    lfoLeft0, lfoTop4, barboxWidth, barboxHeight, ID::lfoWavetable0, nLfoWavetable,
    Scales::wavetableAmp, "LFO Wave");
  if (barboxLfoWavetable) {
    barboxLfoWavetable->sliderZero = 0.5f;
  }

  constexpr auto modEnvTop0 = lfoTop0 + 4 * labelY + barboxHeight + 7 * margin;
  constexpr auto modEnvTop1 = modEnvTop0 + labelY;
  constexpr auto modEnvTop2 = modEnvTop1 + labelY;
  constexpr auto modEnvTop3 = modEnvTop2 + labelY;
  constexpr auto modEnvTop4 = modEnvTop3 + labelY + 2 * margin;

  constexpr auto modEnvTopMid1 = modEnvTop1 + int(labelHeight / 2) + margin;
  constexpr auto modEnvTopMid2 = modEnvTopMid1 + labelY;

  constexpr auto modEnvLeft0 = lfoLeft0;
  constexpr auto modEnvLeft1 = modEnvLeft0 + labelWidth + margin;
  constexpr auto modEnvLeft2 = modEnvLeft1 + labelWidth + margin + lfoWidthFix;
  constexpr auto modEnvLeft3 = modEnvLeft2 + labelWidth;
  constexpr auto modEnvLeft4 = modEnvLeft3 + labelWidth + margin + lfoWidthFix;
  constexpr auto modEnvLeft5 = modEnvLeft4 + labelWidth + margin;
  constexpr auto modEnvLeft6 = modEnvLeft5 + labelWidth + margin;

  addGroupLabel(
    modEnvLeft0, modEnvTop0, barboxWidth, labelHeight, uiTextSize, "Envelope");

  addLabel(
    modEnvLeft0, modEnvTopMid1, labelWidth, labelHeight, uiTextSize, "Time [s]",
    kCenterText);
  addTextKnob(
    modEnvLeft1, modEnvTopMid1, labelWidth, labelHeight, uiTextSize, ID::modEnvelopeTime,
    Scales::modEnvelopeTime, false, 5);
  addLabel(
    modEnvLeft0, modEnvTopMid2, labelWidth, labelHeight, uiTextSize, "Wave Interp.");
  addOptionMenu(
    modEnvLeft1, modEnvTopMid2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeInterpolation, modWavetableInterpolationItems);

  addLabel(modEnvLeft2, modEnvTopMid1, labelWidth, labelHeight, uiTextSize, "> LP Cut");
  addTextKnob<Style::accent>(
    modEnvLeft3, modEnvTopMid1, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnLowpassCutoff, Scales::lfoToPitchAmount, false, 5);
  addLabel(modEnvLeft2, modEnvTopMid2, labelWidth, labelHeight, uiTextSize, "> HP Cut");
  addTextKnob<Style::accent>(
    modEnvLeft3, modEnvTopMid2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnHighpassCutoff, Scales::lfoToPitchAmount, false, 5);

  addLabel(modEnvLeft5, modEnvTop1, labelWidth, labelHeight, uiTextSize, "> Osc. Pitch");
  auto modEnvelopeToOscPitchTextKnob = addTextKnob<Style::accent>(
    modEnvLeft6, modEnvTop1, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToOscPitch, Scales::lfoToPitchAmount, false, 5);
  if (modEnvelopeToOscPitchTextKnob) {
    modEnvelopeToOscPitchTextKnob->setSensitivity(
      float(1) / float(2400), float(1) / float(24000), float(1) / float(240000));
  }
  addLabel(modEnvLeft5, modEnvTop2, labelWidth, labelHeight, uiTextSize, "> FDN Pitch");
  auto modEnvelopeToFdnPitchTextKnob = addTextKnob<Style::warning>(
    modEnvLeft6, modEnvTop2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnPitch, Scales::lfoToPitchAmount, false, 5);
  if (modEnvelopeToFdnPitchTextKnob) {
    modEnvelopeToFdnPitchTextKnob->setSensitivity(
      float(1) / float(2400), float(1) / float(24000), float(1) / float(240000));
  }
  addLabel(modEnvLeft5, modEnvTop3, labelWidth, labelHeight, uiTextSize, "> FDN OT +");
  addTextKnob<Style::accent>(
    modEnvLeft6, modEnvTop3, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnOvertoneAdd, Scales::fdnOvertoneAdd, false, 5);

  auto barboxModEnvWavetable = addBarBox(
    modEnvLeft0, modEnvTop4, barboxWidth, barboxHeight, ID::modEnvelopeWavetable0,
    nModEnvelopeWavetable, Scales::wavetableAmp, "Envelope Wave");
  if (barboxModEnvWavetable) {
    barboxModEnvWavetable->sliderZero = 0.5f;
  }

  // Plugin name.
  const auto splashTop = innerHeight - splashHeight + uiMargin;
  const auto splashLeft = uiMargin;
  addSplashScreen(
    splashLeft, splashTop, 2 * labelWidth, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "ClangSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
