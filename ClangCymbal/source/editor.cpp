// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "gui/randomizebutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

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

  const auto sc = palette.guiScale();

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Gain.
  const auto gainLeft0 = left0;
  const auto gainLeft1 = gainLeft0 + labelWidth;
  const auto gainTop0 = top0;
  const auto gainTop1 = gainTop0 + labelY;
  const auto gainTop2 = gainTop1 + labelY;
  const auto gainTop3 = gainTop2 + labelY;
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
  const auto tuningLeft0 = left0;
  const auto tuningLeft1 = tuningLeft0 + labelWidth;
  const auto tuningTop0 = gainTop3 + labelY;
  const auto tuningTop1 = tuningTop0 + labelY;
  const auto tuningTop2 = tuningTop1 + labelY;
  const auto tuningTop3 = tuningTop2 + labelY;
  const auto tuningTop4 = tuningTop3 + labelY;
  const auto tuningTop5 = tuningTop4 + labelY;
  const auto tuningTop6 = tuningTop5 + labelY;
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

  // Tremolo.
  const auto tremoloLeft0 = tuningLeft0;
  const auto tremoloLeft1 = tremoloLeft0 + labelWidth;
  const auto tremoloTop0 = tuningTop6 + labelY;
  const auto tremoloTop1 = tremoloTop0 + labelY;
  const auto tremoloTop2 = tremoloTop1 + labelY;
  const auto tremoloTop3 = tremoloTop2 + labelY;
  const auto tremoloTop4 = tremoloTop3 + labelY;
  const auto tremoloTop5 = tremoloTop4 + labelY;

  addGroupLabel(
    tremoloLeft0, tremoloTop0, 2 * labelWidth, labelHeight, uiTextSize, "Tremolo");

  addLabel(tremoloLeft0, tremoloTop1, labelWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob<Style::accent>(
    tremoloLeft1, tremoloTop1, labelWidth, labelHeight, uiTextSize, ID::tremoloMix,
    Scales::defaultScale, false, 5);
  addLabel(tremoloLeft0, tremoloTop2, labelWidth, labelHeight, uiTextSize, "Depth");
  addTextKnob<Style::accent>(
    tremoloLeft1, tremoloTop2, labelWidth, labelHeight, uiTextSize, ID::tremoloDepth,
    Scales::tremoloDepth, false, 5);
  addLabel(tremoloLeft0, tremoloTop3, labelWidth, labelHeight, uiTextSize, "Delay [s]");
  addTextKnob<Style::accent>(
    tremoloLeft1, tremoloTop3, labelWidth, labelHeight, uiTextSize, ID::tremoloDelayTime,
    Scales::tremoloDelayTime, false, 5);
  addLabel(
    tremoloLeft0, tremoloTop4, labelWidth, labelHeight, uiTextSize, "Delay Offset");
  addTextKnob<Style::accent>(
    tremoloLeft1, tremoloTop4, labelWidth, labelHeight, uiTextSize,
    ID::tremoloModulationToDelayTimeOffset, Scales::defaultScale, false, 5);
  addLabel(tremoloLeft0, tremoloTop5, labelWidth, labelHeight, uiTextSize, "Rate [Hz]");
  addTextKnob<Style::accent>(
    tremoloLeft1, tremoloTop5, labelWidth, labelHeight, uiTextSize,
    ID::tremoloModulationRateHz, Scales::tremoloModulationRateHz, false, 5);

  // Misc.
  const auto miscLeft0 = tuningLeft0;
  const auto miscLeft1 = miscLeft0 + labelWidth;
  const auto miscTop0 = tremoloTop5 + labelY;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscTop2 = miscTop1 + labelY;
  const auto miscTop3 = miscTop2 + labelY;
  const auto miscTop4 = miscTop3 + labelY;

  addGroupLabel(miscLeft0, miscTop0, 2 * labelWidth, labelHeight, uiTextSize, "Misc.");

  addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob<Style::accent>(
    miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize,
    ID::commonSmoothingTimeSecond, Scales::smoothingTimeSecond, false, 5);
  addLabel(miscLeft0, miscTop2, labelWidth, labelHeight, uiTextSize, "Slide [s]");
  addTextKnob<Style::accent>(
    miscLeft1, miscTop2, labelWidth, labelHeight, uiTextSize, ID::slideTimeSecond,
    Scales::smoothingTimeSecond, false, 5);
  addLabel(miscLeft0, miscTop3, labelWidth, labelHeight, uiTextSize, "Slide Type");
  std::vector<std::string> slideTypeItems{"Sustain", "Always", "Reset to 0"};
  addOptionMenu(
    miscLeft1, miscTop3, labelWidth, labelHeight, uiTextSize, ID::slideType,
    slideTypeItems);
  addCheckbox(
    miscLeft1, miscTop4, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);

  // Randomize button.
  const auto randomButtonTop = miscTop4;
  const auto randomButtonLeft = miscLeft0;
  auto panicButton = new RandomizeButton(
    CRect(
      randomButtonLeft, randomButtonTop, randomButtonLeft + labelWidth - 2 * margin,
      randomButtonTop + labelHeight),
    this, 0, "Random", getFont(uiTextSize), palette, this);
  frame->addView(panicButton);

  // Oscillator.
  const auto oscLeft0 = gainLeft0 + 2 * labelWidth + 4 * margin;
  const auto oscLeft1 = oscLeft0 + labelWidth;
  const auto oscLeft2 = oscLeft1 + labelWidth + 2 * margin;
  const auto oscLeft3 = oscLeft2 + labelWidth;

  const auto oscTop0 = top0;
  const auto oscTop1 = oscTop0 + labelY;
  const auto oscTop2 = oscTop1 + labelY;
  const auto oscTop3 = oscTop2 + labelY;
  const auto oscTop4 = oscTop3 + labelY;
  const auto oscTop5 = oscTop4 + labelY;
  const auto oscTop6 = oscTop5 + labelY;
  const auto oscTop7 = oscTop6 + labelY;
  const auto oscTop8 = oscTop7 + labelY;
  const auto oscTop9 = oscTop8 + labelY;
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
  addLabel(oscLeft0, oscTop3, labelWidth, labelHeight, uiTextSize, "Noise/Pulse");
  addTextKnob<Style::accent>(
    oscLeft1, oscTop3, labelWidth, labelHeight, uiTextSize, ID::oscNoisePulseRatio,
    Scales::defaultScale, false, 5);
  addLabel(oscLeft0, oscTop4, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    oscLeft1, oscTop4, labelWidth, labelHeight, uiTextSize, ID::oscAttack,
    Scales::oscAttack, false, 5);
  addLabel(oscLeft0, oscTop5, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    oscLeft1, oscTop5, labelWidth, labelHeight, uiTextSize, ID::oscDecay,
    Scales::oscDecay, false, 5);
  addLabel(oscLeft0, oscTop7, labelWidth, labelHeight, uiTextSize, "LP Cutoff [st.]");
  addTextKnob<Style::accent>(
    oscLeft1, oscTop7, labelWidth, labelHeight, uiTextSize, ID::oscLowpassCutoffSemi,
    Scales::filterCutoffSemi, false, 5);
  addLabel(oscLeft0, oscTop8, labelWidth, labelHeight, uiTextSize, "LP Q");
  addTextKnob<Style::accent>(
    oscLeft1, oscTop8, labelWidth, labelHeight, uiTextSize, ID::oscLowpassQ,
    Scales::oscLowpassQ, false, 5);
  addLabel(oscLeft0, oscTop9, labelWidth, labelHeight, uiTextSize, "LP Key");
  addTextKnob(
    oscLeft1, oscTop9, labelWidth, labelHeight, uiTextSize, ID::oscLowpassKeyFollow,
    Scales::defaultScale, false, 5);

  addLabel(oscLeft2, oscTop1, labelWidth, labelHeight, uiTextSize, "Density [Hz]");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop1, labelWidth, labelHeight, uiTextSize, ID::oscDensityHz,
    Scales::oscDensityHz, false, 5);
  addLabel(oscLeft2, oscTop2, labelWidth, labelHeight, uiTextSize, "Density Key");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop2, labelWidth, labelHeight, uiTextSize, ID::oscDensityKeyFollow,
    Scales::oscDensityKeyFollow, false, 5);

  addLabel(oscLeft2, oscTop4, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop4, labelWidth, labelHeight, uiTextSize, ID::oscNoiseDecay,
    Scales::oscDecay, false, 5);
  addLabel(oscLeft2, oscTop5, labelWidth, labelHeight, uiTextSize, "Bounce");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop5, labelWidth, labelHeight, uiTextSize, ID::oscBounce,
    Scales::defaultScale, false, 5);
  addLabel(oscLeft2, oscTop6, labelWidth, labelHeight, uiTextSize, "Bounce Curve");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop6, labelWidth, labelHeight, uiTextSize, ID::oscBounceCurve,
    Scales::oscBounceCurve, false, 5);
  addLabel(oscLeft2, oscTop7, labelWidth, labelHeight, uiTextSize, "Jitter");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop7, labelWidth, labelHeight, uiTextSize, ID::oscJitter,
    Scales::defaultScale, false, 5);
  addLabel(oscLeft2, oscTop8, labelWidth, labelHeight, uiTextSize, "Amp. Rand.");
  addTextKnob<Style::accent>(
    oscLeft3, oscTop8, labelWidth, labelHeight, uiTextSize, ID::oscPulseAmpRandomness,
    Scales::defaultScale, false, 5);

  // FDN.
  const auto fdnLeft0 = oscLeft0;
  const auto fdnLeft1 = fdnLeft0 + labelWidth;
  const auto fdnLeft2 = fdnLeft1 + labelWidth + 2 * margin;
  const auto fdnLeft3 = fdnLeft2 + labelWidth;

  const auto fdnTop0 = oscTop8 + 3 * labelHeight + 2 * margin;
  const auto fdnTop1 = fdnTop0 + labelY;
  const auto fdnTop2 = fdnTop1 + labelY;
  const auto fdnTop3 = fdnTop2 + labelY;
  const auto fdnTop4 = fdnTop3 + labelY;
  const auto fdnTop5 = fdnTop4 + labelY;
  const auto fdnTop6 = fdnTop5 + labelY;
  const auto fdnTop7 = fdnTop6 + labelY + 1 * margin;
  const auto fdnTop8 = fdnTop7 + labelY;
  const auto fdnTop9 = fdnTop8 + labelY;
  const auto fdnTop10 = fdnTop9 + labelY;
  const auto fdnTop11 = fdnTop10 + labelY;
  const auto fdnTop12 = fdnTop11 + labelY;
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
  addCheckbox<Style::warning>(
    fdnLeft2, fdnTop6, 2 * labelWidth, labelHeight, uiTextSize, "Reset at Note On",
    ID::resetAtNoteOn);

  const auto fdnFilterLeft0 = fdnLeft0 + int(labelWidth / 4);
  const auto fdnFilterLeft1 = fdnFilterLeft0 + labelWidth + 2 * margin;
  const auto fdnFilterLeft2 = fdnFilterLeft1 + labelWidth + 2 * margin;

  addLabel(fdnFilterLeft1, fdnTop7, labelWidth, labelHeight, uiTextSize, "Lowpass");
  addLabel(fdnFilterLeft2, fdnTop7, labelWidth, labelHeight, uiTextSize, "Highpass");

  addLabel(fdnFilterLeft0, fdnTop8, labelWidth, labelHeight, uiTextSize, "Cutoff [st.]");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop8, labelWidth, labelHeight, uiTextSize,
    ID::fdnLowpassCutoffSemi, Scales::filterCutoffSemi, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop8, labelWidth, labelHeight, uiTextSize,
    ID::fdnHighpassCutoffSemi, Scales::filterCutoffSemi, false, 5);

  addLabel(fdnFilterLeft0, fdnTop9, labelWidth, labelHeight, uiTextSize, "Cutoff Slope");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop9, labelWidth, labelHeight, uiTextSize,
    ID::fdnLowpassCutoffSlope, Scales::filterCutoffSlope, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop9, labelWidth, labelHeight, uiTextSize,
    ID::fdnHighpassCutoffSlope, Scales::filterCutoffSlope, false, 5);

  addLabel(fdnFilterLeft0, fdnTop10, labelWidth, labelHeight, uiTextSize, "Q");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop10, labelWidth, labelHeight, uiTextSize, ID::fdnLowpassQ,
    Scales::filterQ, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop10, labelWidth, labelHeight, uiTextSize, ID::fdnHighpassQ,
    Scales::filterQ, false, 5);

  addLabel(fdnFilterLeft0, fdnTop11, labelWidth, labelHeight, uiTextSize, "Q Slope");
  addTextKnob<Style::accent>(
    fdnFilterLeft1, fdnTop11, labelWidth, labelHeight, uiTextSize, ID::fdnLowpassQSlope,
    Scales::filterQSlope, false, 5);
  addTextKnob<Style::accent>(
    fdnFilterLeft2, fdnTop11, labelWidth, labelHeight, uiTextSize, ID::fdnHighpassQSlope,
    Scales::filterQSlope, false, 5);

  addLabel(fdnFilterLeft0, fdnTop12, labelWidth, labelHeight, uiTextSize, "Key Follow");
  addCheckbox(
    fdnFilterLeft1 + int((labelWidth - checkBoxWidth) / 2), fdnTop12, labelWidth,
    labelHeight, uiTextSize, "", ID::fdnLowpassKeyFollow);
  addCheckbox(
    fdnFilterLeft2 + int((labelWidth - checkBoxWidth) / 2), fdnTop12, labelWidth,
    labelHeight, uiTextSize, "", ID::fdnHighpassKeyFollow);

  const auto fdnSvfBarboxWidth = int(sc * 256);
  const auto fdnSvfBarboxHeight = 6 * labelY - 2 * margin;
  const auto fdnSvfTop0 = fdnTop1;
  const auto fdnSvfTop1 = fdnSvfTop0 + fdnSvfBarboxHeight + 4 * margin;
  const auto fdnSvfLeft0 = oscLeft0 + 4 * labelWidth + 6 * margin;
  const auto fdnSvfLeft1 = fdnSvfLeft0 + fdnSvfBarboxWidth + 0 * margin;
  addGroupLabel(fdnSvfLeft0, fdnTop0, barboxWidth, labelHeight, uiTextSize, "FDN Filter");

  auto barboxFdnLowpassCutoffSemiOffset = addBarBox(
    fdnSvfLeft0, fdnSvfTop0, fdnSvfBarboxWidth, fdnSvfBarboxHeight,
    ID::fdnLowpassCutoffSemiOffset0, fdnMatrixSize, Scales::filterCutoffSemiOffset,
    "LP Cut Offset [st.]");
  if (barboxFdnLowpassCutoffSemiOffset) {
    barboxFdnLowpassCutoffSemiOffset->sliderZero = 0.5f;
  }
  auto barboxFdnLowpassQOffset = addBarBox(
    fdnSvfLeft0, fdnSvfTop1, fdnSvfBarboxWidth, fdnSvfBarboxHeight,
    ID::fdnLowpassQOffset0, fdnMatrixSize, Scales::filterQOffset, "LP Q Offset");
  if (barboxFdnLowpassQOffset) {
    barboxFdnLowpassQOffset->sliderZero = 0.5f;
  }

  auto barboxFdnHighpassCutoffSemiOffset = addBarBox(
    fdnSvfLeft1, fdnSvfTop0, fdnSvfBarboxWidth, fdnSvfBarboxHeight,
    ID::fdnHighpassCutoffSemiOffset0, fdnMatrixSize, Scales::filterCutoffSemiOffset,
    "HP Cut Offset [st.]");
  if (barboxFdnHighpassCutoffSemiOffset) {
    barboxFdnHighpassCutoffSemiOffset->sliderZero = 0.5f;
  }
  auto barboxFdnHighpassQOffset = addBarBox(
    fdnSvfLeft1, fdnSvfTop1, fdnSvfBarboxWidth, fdnSvfBarboxHeight,
    ID::fdnHighpassQOffset0, fdnMatrixSize, Scales::filterQOffset, "HP Q Offset");
  if (barboxFdnHighpassQOffset) {
    barboxFdnHighpassQOffset->sliderZero = 0.5f;
  }

  // Envelope.
  const auto modEnvTop0 = oscTop0;
  const auto modEnvTop1 = modEnvTop0 + labelY;
  const auto modEnvTop2 = modEnvTop1 + labelY;
  const auto modEnvTop3 = modEnvTop2 + labelY;
  const auto modEnvTop4 = modEnvTop3 + labelY;

  const auto modEnvTopMid1 = modEnvTop1 + int(labelHeight / 2) + margin;
  const auto modEnvTopMid2 = modEnvTopMid1 + labelY;

  const auto modEnvLeft0 = oscLeft0 + 4 * labelWidth + 6 * margin;
  const auto modEnvLeft1 = modEnvLeft0 + labelWidth;
  const auto modEnvLeft2 = modEnvLeft1 + labelWidth + 2 * margin;
  const auto modEnvLeft3 = modEnvLeft2 + labelWidth;
  const auto modEnvLeft4 = modEnvLeft3 + labelWidth + 2 * margin;
  const auto modEnvLeft5 = modEnvLeft4 + labelWidth;

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
  std::vector<std::string> modWavetableInterpolationItems{"Step", "Linear", "PCHIP"};
  addOptionMenu<Style::accent>(
    modEnvLeft1, modEnvTopMid2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeInterpolation, modWavetableInterpolationItems);

  addLabel(modEnvLeft2, modEnvTop1, labelWidth, labelHeight, uiTextSize, "> LP Cut");
  addTextKnob<Style::accent>(
    modEnvLeft3, modEnvTop1, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnLowpassCutoff, Scales::lfoToPitchAmount, false, 5);
  addLabel(modEnvLeft2, modEnvTop2, labelWidth, labelHeight, uiTextSize, "> HP Cut");
  addTextKnob<Style::accent>(
    modEnvLeft3, modEnvTop2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnHighpassCutoff, Scales::lfoToPitchAmount, false, 5);
  addLabel(modEnvLeft2, modEnvTop3, labelWidth, labelHeight, uiTextSize, "> Jitter");
  addTextKnob<Style::accent>(
    modEnvLeft3, modEnvTop3, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToOscJitter, Scales::bipolarScale, false, 5);

  addLabel(modEnvLeft4, modEnvTop1, labelWidth, labelHeight, uiTextSize, "> FDN OT +");
  addTextKnob<Style::accent>(
    modEnvLeft5, modEnvTop1, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnOvertoneAdd, Scales::fdnOvertoneAdd, false, 5);
  addLabel(modEnvLeft4, modEnvTop2, labelWidth, labelHeight, uiTextSize, "> FDN Pitch");
  auto modEnvelopeToFdnPitchTextKnob = addTextKnob<Style::warning>(
    modEnvLeft5, modEnvTop2, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToFdnPitch, Scales::lfoToPitchAmount, false, 5);
  if (modEnvelopeToFdnPitchTextKnob) {
    modEnvelopeToFdnPitchTextKnob->setSensitivity(
      float(1) / float(2400), float(1) / float(24000), float(1) / float(240000));
  }
  addLabel(modEnvLeft4, modEnvTop3, labelWidth, labelHeight, uiTextSize, "> Noise/Pulse");
  addTextKnob<Style::accent>(
    modEnvLeft5, modEnvTop3, labelWidth, labelHeight, uiTextSize,
    ID::modEnvelopeToOscNoisePulseRatio, Scales::bipolarScale, false, 5);

  addBarBox(
    modEnvLeft0, modEnvTop4, barboxWidth, barboxHeight, ID::modEnvelopeWavetable0,
    nModEnvelopeWavetable, Scales::defaultScale, "Envelope Wave");

  // Plugin name.
  const auto splashTop = innerHeight - splashHeight + uiMargin;
  const auto splashLeft = uiMargin;
  addSplashScreen(
    splashLeft, splashTop, 2 * labelWidth, splashHeight, uiMargin, uiMargin,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "ClangCymbal");

  return true;
}

} // namespace Vst
} // namespace Steinberg
