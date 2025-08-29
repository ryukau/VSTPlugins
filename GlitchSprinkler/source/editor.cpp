// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "gui/randomizebutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
}

void Editor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);

  using ID = Synth::ParameterID::ID;
  if (id >= ID::polynomialPointX0 && id < ID::polynomialPointY0 + nPolyOscControl) {
    if (polyXYPad != nullptr) polyXYPad->linkControlFromId(id);
  }
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

  // Mix.
  const auto mixTop0 = top0;
  const auto mixTop1 = mixTop0 + 1 * labelY;
  const auto mixTop2 = mixTop0 + 2 * labelY;
  const auto mixTop4 = mixTop0 + 4 * labelY;
  const auto mixTop5 = mixTop0 + 5 * labelY;
  const auto mixTop6 = mixTop0 + 6 * labelY;
  const auto mixTop7 = mixTop0 + 7 * labelY;
  const auto mixTop8 = mixTop0 + 8 * labelY;
  const auto mixTop9 = mixTop0 + 9 * labelY;
  const auto mixTop10 = mixTop0 + 10 * labelY;
  const auto mixTop11 = mixTop0 + 11 * labelY;
  const auto mixLeft0 = left0;
  const auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Oscillator");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);
  addLabel(mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "Safety Filter Mix");
  addTextKnob(
    mixLeft1, mixTop2, labelWidth, labelHeight, uiTextSize, ID::safetyFilterMix,
    Scales::defaultScale, false, 5);

  addLabel(mixLeft0, mixTop4, labelWidth, labelHeight, uiTextSize, "Decay to [dB]");
  addTextKnob<Uhhyou::Style::warning>(
    mixLeft1, mixTop4, labelWidth, labelHeight, uiTextSize, ID::decayTargetGain,
    Scales::decayTargetGain, true, 5);
  addCheckbox(
    mixLeft0, mixTop5, labelWidthOneThird, labelHeight, uiTextSize, "Polyphonic",
    ID::polyphonic);
  addCheckbox(
    mixLeft0 + labelWidthOneThird * 1, mixTop5, labelWidthOneThird, labelHeight,
    uiTextSize, "Release", ID::release);
  addCheckbox(
    mixLeft0 + labelWidthOneThird * 2, mixTop5, labelWidthOneThird, labelHeight,
    uiTextSize, "Soft Envelope", ID::softEnvelopeSwitch);

  addLabel(mixLeft0, mixTop6, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    mixLeft1, mixTop6, labelWidth, labelHeight, uiTextSize, ID::transposeOctave,
    Scales::transposeOctave, false, 0, -transposeOctaveOffset);
  addLabel(mixLeft0, mixTop7, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    mixLeft1, mixTop7, labelWidth, labelHeight, uiTextSize, ID::transposeSemitone,
    Scales::transposeSemitone, false, 0, -transposeSemitoneOffset);
  addLabel(mixLeft0, mixTop8, labelWidth, labelHeight, uiTextSize, "Cent");
  auto transposeCentTextKnob = addTextKnob(
    mixLeft1, mixTop8, labelWidth, labelHeight, uiTextSize, ID::transposeCent,
    Scales::transposeCent, false, 5);
  if (transposeCentTextKnob) {
    transposeCentTextKnob->sensitivity = 1.0 / 7200.0;
    transposeCentTextKnob->lowSensitivity = 1.0 / 72000.0;
    transposeCentTextKnob->wheelSensitivity = 1.0 / 7200.0;
  }

  addLabel(mixLeft0, mixTop9, labelWidth, labelHeight, uiTextSize, "Tuning");
  std::vector<std::string> tuningItems{
    "ET 12",      "ET 5",           "ET 10",        "Just 5 Major", "Just 5 Minor",
    "Just 7",     "MT Pythagorean", "MT 1/3 Comma", "MT 1/4 Comma", "Discrete 2",
    "Discrete 3", "Discrete 5",     "Discrete 7",
  };
  for (size_t idx = tuningItems.size(); idx <= Scales::tuningType.getMax(); ++idx) {
    tuningItems.push_back("- Reserved " + std::to_string(idx) + " -");
  }
  addOptionMenu(
    mixLeft1, mixTop9, labelWidth, labelHeight, uiTextSize, ID::tuningType, tuningItems);

  addLabel(mixLeft0, mixTop10, labelWidth, labelHeight, uiTextSize, "Tuning Root [st.]");
  addTextKnob(
    mixLeft1, mixTop10, labelWidth, labelHeight, uiTextSize, ID::tuningRootSemitone,
    Scales::tuningRootSemitone, false, 0);

  // Filter.
  const auto filterLabelWidth = knobWidth + uiMargin;
  const auto filterTop0 = mixTop11 + labelY;
  const auto filterTop1 = filterTop0 + 1 * labelY;
  const auto filterTop2 = filterTop0 + 2 * labelY;
  const auto filterTop3 = filterTop0 + 3 * labelY;
  const auto filterTop4 = filterTop0 + 4 * labelY;
  const auto filterTop5 = filterTop0 + 5 * labelY;
  const auto filterTop6 = filterTop0 + 6 * labelY;
  const auto filterLeft0 = left0;
  const auto filterLeft1 = filterLeft0 + 1 * filterLabelWidth + 3 * margin;
  const auto filterLeft2 = filterLeft0 + 2 * filterLabelWidth + 6 * margin;
  addToggleButton(
    filterLeft0, filterTop0, groupLabelWidth, labelHeight, uiTextSize, "Filter",
    ID::filterSwitch);

  addLabel(
    filterLeft0, filterTop1, labelWidth, labelHeight, uiTextSize,
    "Decay Time Ratio [dB]");
  addTextKnob(
    filterLeft0 + labelWidth + 2 * margin, filterTop1, labelWidth, labelHeight,
    uiTextSize, ID::filterDecayRatio, Scales::filterDecayRatio, false, 5);
  addLabel(filterLeft0, filterTop2, labelWidth, labelHeight, uiTextSize, "Key Follow");
  addTextKnob(
    filterLeft0 + labelWidth + 2 * margin, filterTop2, labelWidth, labelHeight,
    uiTextSize, ID::filterCutoffKeyFollow, Scales::defaultScale, false, 5);

  addLabel(filterLeft1, filterTop3, filterLabelWidth, labelHeight, uiTextSize, "Base");
  addLabel(
    filterLeft2, filterTop3, filterLabelWidth, labelHeight, uiTextSize, "Env. Mod");

  addLabel(
    filterLeft0, filterTop4, filterLabelWidth, labelHeight, uiTextSize, "Cutoff [oct.]");
  addTextKnob(
    filterLeft1, filterTop4, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterCutoffBaseOctave, Scales::filterCutoffBaseOctave, false, 5);
  addTextKnob(
    filterLeft2, filterTop4, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterCutoffModOctave, Scales::filterCutoffModOctave, false, 5);

  addLabel(
    filterLeft0, filterTop5, filterLabelWidth, labelHeight, uiTextSize, "Resonance");
  addTextKnob(
    filterLeft1, filterTop5, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterResonanceBase, Scales::defaultScale, false, 5);
  addTextKnob(
    filterLeft2, filterTop5, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterResonanceMod, Scales::defaultScale, false, 5);

  addLabel(
    filterLeft0, filterTop6, filterLabelWidth, labelHeight, uiTextSize, "Notch [oct.]");
  addTextKnob(
    filterLeft1, filterTop6, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterNotchBaseOctave, Scales::filterNotchBaseOctave, false, 5);
  addTextKnob(
    filterLeft2, filterTop6, filterLabelWidth, labelHeight, uiTextSize,
    ID::filterNotchModOctave, Scales::filterNotchModOctave, false, 5);

  // Waveform.
  const auto waveformTop0 = top0 + 0 * labelY;
  const auto waveformTop1 = waveformTop0 + 1 * labelY;
  const auto waveformTop2 = waveformTop1 + barBoxWidth;
  const auto waveformTop3 = waveformTop2 + 1 * labelY;
  const auto waveformTop4 = waveformTop2 + 2 * labelY;
  const auto waveformTop5 = waveformTop2 + 3 * labelY;
  const auto waveformTop6 = waveformTop2 + 4 * labelY;
  const auto waveformTop7 = waveformTop2 + 5 * labelY;
  const auto waveformTop8 = waveformTop2 + 6 * labelY;
  const auto waveformLeft0 = left4;
  const auto waveformLeft1 = left4 + labelWidth + 2 * margin;
  addGroupLabel(
    waveformLeft0, waveformTop0, groupLabelWidth, labelHeight, uiTextSize, "Waveform");

  {
    const auto halfLabelWidth = int(labelWidth / 2);
    polyXControl = addTextKnob(
      waveformLeft0 + halfLabelWidth, waveformTop2, halfLabelWidth, labelHeight,
      uiTextSize, ID::polynomialPointX0, Scales::defaultScale, false, 5);
    polyYControl = addTextKnob(
      waveformLeft1 + halfLabelWidth, waveformTop2, halfLabelWidth, labelHeight,
      uiTextSize, ID::polynomialPointY0, Scales::polynomialPointY, false, 5);

    // A wasteful hack to avoid shadowing. Using `polyXYPad->linkControlFromId()` in
    // `updateUI` for linking parameters.
    controlMap.erase(ID::polynomialPointX0);
    controlMap.erase(ID::polynomialPointY0);

    polyXLabel = addLabel(
      waveformLeft0, waveformTop2, halfLabelWidth, labelHeight, uiTextSize, "X0");
    polyYLabel = addLabel(
      waveformLeft1, waveformTop2, halfLabelWidth, labelHeight, uiTextSize, "Y0");

    std::vector<ParamID> id(2 * nPolyOscControl);
    for (size_t i = 0; i < id.size(); ++i) id[i] = ID::polynomialPointX0 + ParamID(i);
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i) {
      value[i] = controller->getParamNormalized(id[i]);
    }
    std::vector<double> defaultValue(id.size());
    for (size_t i = 0; i < defaultValue.size(); ++i) {
      defaultValue[i] = param->getDefaultNormalized(id[i]);
    }
    polyXYPad = new PolynomialXYPad(
      this,
      CRect(
        waveformLeft0, waveformTop1, waveformLeft0 + barBoxWidth,
        waveformTop1 + barBoxWidth),
      id, value, defaultValue, palette, polyXLabel, polyYLabel, polyXControl,
      polyYControl);

    for (ParamID i = 0; i < id.size(); ++i) {
      arrayControlMap.emplace(std::make_pair(id[i], polyXYPad));
    }
    frame->addView(polyXYPad);
  }

  addLabel(
    waveformLeft0, waveformTop3, labelWidth, labelHeight, uiTextSize, "Osc. Sync.");
  addTextKnob(
    waveformLeft1, waveformTop3, labelWidth, labelHeight, uiTextSize, ID::oscSync,
    Scales::defaultScale, false, 5);
  addLabel(waveformLeft0, waveformTop4, labelWidth, labelHeight, uiTextSize, "FM Index");
  addTextKnob(
    waveformLeft1, waveformTop4, labelWidth, labelHeight, uiTextSize, ID::fmIndex,
    Scales::fmIndex, false, 5);
  addLabel(
    waveformLeft0, waveformTop5, labelWidth, labelHeight, uiTextSize, "Saturation [dB]");
  addTextKnob(
    waveformLeft1, waveformTop5, labelWidth, labelHeight, uiTextSize, ID::saturationGain,
    Scales::gain, true, 5);
  addLabel(
    waveformLeft0, waveformTop6, labelWidth, labelHeight, uiTextSize,
    "Pulse Width / Bit Mask");
  addTextKnob<Uhhyou::Style::warning>(
    waveformLeft1, waveformTop6, labelWidth, labelHeight, uiTextSize, ID::pulseWidthRatio,
    Scales::defaultScale, false, 5);
  addLabel(
    waveformLeft0, waveformTop7, labelWidth, labelHeight, uiTextSize, "Modulation Rate");
  addTextKnob(
    waveformLeft1, waveformTop7, labelWidth, labelHeight, uiTextSize,
    ID::pulseWidthModRate, Scales::pulseWidthModRate, false, 0, -pwmOneCyclePoint);

  addCheckbox<Uhhyou::Style::warning>(
    waveformLeft0, waveformTop8, labelWidthOneThird, labelHeight, uiTextSize, "PWM",
    ID::pulseWidthModulation);
  addCheckbox(
    waveformLeft0 + labelWidthOneThird * 1, waveformTop8, labelWidthOneThird, labelHeight,
    uiTextSize, "Bidirectional", ID::pulseWidthModBidirectionalSwitch);
  addCheckbox(
    waveformLeft0 + labelWidthOneThird * 2, waveformTop8, labelWidthOneThird, labelHeight,
    uiTextSize, "Bitwise And", ID::pulseWidthBitwiseAnd);

  // Arpeggio.
  const auto arpTop0 = top0;
  const auto arpTop1 = arpTop0 + 1 * labelY;
  const auto arpTop3 = arpTop0 + 3 * labelY;
  const auto arpTop4 = arpTop0 + 4 * labelY;
  const auto arpTop5 = arpTop0 + 5 * labelY;
  const auto arpTop6 = arpTop0 + 6 * labelY;
  const auto arpTop8 = arpTop0 + 8 * labelY;
  const auto arpTop9 = arpTop0 + 9 * labelY;
  const auto arpTop10 = arpTop0 + 10 * labelY;
  const auto arpTop11 = arpTop0 + 11 * labelY;
  const auto arpTop12 = arpTop0 + 12 * labelY;
  const auto arpLeft0 = left8;
  const auto arpLeft1 = arpLeft0 + labelWidth + 2 * margin;
  addToggleButton(
    arpLeft0, arpTop0, groupLabelWidth, labelHeight, uiTextSize, "Arpeggio",
    ID::arpeggioSwitch);
  addLabel(arpLeft0, arpTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob<Uhhyou::Style::accent>(
    arpLeft1, arpTop1, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed, false,
    0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }

  addLabel(arpLeft0, arpTop3, labelWidth, labelHeight, uiTextSize, "Note / Beat");
  addTextKnob(
    arpLeft1, arpTop3, labelWidth, labelHeight, uiTextSize, ID::arpeggioNotesPerBeat,
    Scales::arpeggioNotesPerBeat, false, 0, 1);
  addLabel(arpLeft0, arpTop4, labelWidth, labelHeight, uiTextSize, "Loop Length [beat]");
  addTextKnob(
    arpLeft1, arpTop4, labelWidth, labelHeight, uiTextSize, ID::arpeggioLoopLengthInBeat,
    Scales::arpeggioLoopLengthInBeat, false, 0, 0);
  addLabel(arpLeft0, arpTop5, labelWidth, labelHeight, uiTextSize, "Duration Variation");
  addTextKnob(
    arpLeft1, arpTop5, labelWidth, labelHeight, uiTextSize, ID::arpeggioDurationVariation,
    Scales::arpeggioDurationVariation, false, 0, 1);
  addLabel(arpLeft0, arpTop6, labelWidth, labelHeight, uiTextSize, "Rest Chance");
  addTextKnob(
    arpLeft1, arpTop6, labelWidth, labelHeight, uiTextSize, ID::arpeggioRestChance,
    Scales::defaultScale, false, 5);

  std::vector<std::string> arpeggioScaleItems{
    "Octave",
    "ET 5 Chromatic",
    "Church C (Major Scale)",
    "Church D",
    "Church E",
    "Church F",
    "Church G",
    "Church A (Minor Scale)",
    "Church B",
    "Suspended 2",
    "Suspended 4",
    "Major 7",
    "Minor 7",
    "Major 7 Extended",
    "Minor 7 Extended",
    "Whole Tone 2",
    "Whole Tone 3",
    "Whole Tone 4",
    "Blues",
    "Overtone 32",
    "The 42 Melody",
    "Overtone Odd 16",
  };
  for (size_t idx = arpeggioScaleItems.size(); idx <= Scales::arpeggioScale.getMax();
       ++idx)
  {
    arpeggioScaleItems.push_back("- Reserved " + std::to_string(idx) + " -");
  }
  addLabel(arpLeft0, arpTop8, labelWidth, labelHeight, uiTextSize, "Scale");
  addOptionMenu(
    arpLeft1, arpTop8, labelWidth, labelHeight, uiTextSize, ID::arpeggioScale,
    arpeggioScaleItems);

  addLabel(arpLeft0, arpTop9, labelWidth, labelHeight, uiTextSize, "Pitch Drift [cent]");
  addTextKnob(
    arpLeft1, arpTop9, labelWidth, labelHeight, uiTextSize, ID::arpeggioPicthDriftCent,
    Scales::arpeggioPicthDriftCent, false, 5);
  addLabel(arpLeft0, arpTop10, labelWidth, labelHeight, uiTextSize, "Octave Range");
  addTextKnob(
    arpLeft1, arpTop10, labelWidth, labelHeight, uiTextSize, ID::arpeggioOctave,
    Scales::arpeggioOctave, false, 0, 1);
  addCheckbox(
    arpLeft0, arpTop11, labelWidth, labelHeight, uiTextSize, "Start From Root",
    ID::arpeggioStartFromRoot);
  addCheckbox(
    arpLeft1, arpTop11, labelWidth, labelHeight, uiTextSize, "Reset Modulation",
    ID::arpeggioResetModulation);

  addLabel(arpLeft0, arpTop12, labelWidth, labelHeight, uiTextSize, "Random FM Index");
  addTextKnob(
    arpLeft1, arpTop12, labelWidth, labelHeight, uiTextSize, ID::randomizeFmIndex,
    Scales::randomizeFmIndex, false, 5);

  // Unison.
  const auto unisonTop0 = arpTop12 + labelY;
  const auto unisonTop1 = unisonTop0 + 1 * labelY;
  const auto unisonTop2 = unisonTop0 + 2 * labelY;
  const auto unisonTop3 = unisonTop0 + 3 * labelY;
  const auto unisonTop4 = unisonTop0 + 4 * labelY;
  const auto unisonLeft0 = left8;
  const auto unisonLeft1 = unisonLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    unisonLeft0, unisonTop0, groupLabelWidth, labelHeight, uiTextSize, "Unison");

  addLabel(unisonLeft0, unisonTop1, labelWidth, labelHeight, uiTextSize, "nVoice");
  addTextKnob<Uhhyou::Style::warning>(
    unisonLeft1, unisonTop1, labelWidth, labelHeight, uiTextSize, ID::unisonVoice,
    Scales::unisonVoice, false, 0, 1);
  addLabel(unisonLeft0, unisonTop2, labelWidth, labelHeight, uiTextSize, "Detune [cent]");
  auto unisonDetuneCentTextKnob = addTextKnob(
    unisonLeft1, unisonTop2, labelWidth, labelHeight, uiTextSize, ID::unisonDetuneCent,
    Scales::unisonDetuneCent, false, 5);
  if (unisonDetuneCentTextKnob) {
    unisonDetuneCentTextKnob->sensitivity = 1.0 / 1200.0;
    unisonDetuneCentTextKnob->lowSensitivity = 1.0 / 120000.0;
  }
  addLabel(unisonLeft0, unisonTop3, labelWidth, labelHeight, uiTextSize, "Pan Spread");
  addTextKnob(
    unisonLeft1, unisonTop3, labelWidth, labelHeight, uiTextSize, ID::unisonPanSpread,
    Scales::defaultScale, false, 5);
  addCheckbox(
    unisonLeft0, unisonTop4, labelWidth, labelHeight, uiTextSize, "Scatter Arpeggio",
    ID::unisonScatterArpeggio);
  addCheckbox(
    unisonLeft1, unisonTop4, labelWidth, labelHeight, uiTextSize, "Gain Sqrt.",
    ID::unisonGainSqrt);

  // Plugin name and randomize button.
  const auto splashMargin = uiMargin;
  const auto splashTop = top0 + 18 * labelY;
  const auto splashLeft = left8;
  const auto randomButtonTop = splashTop;
  const auto randomButtonLeft = splashLeft;
  auto panicButton = new RandomizeButton(
    CRect(
      randomButtonLeft, randomButtonTop, randomButtonLeft + labelWidth,
      randomButtonTop + splashHeight),
    this, 0, "Randomize", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  addSplashScreen(
    splashLeft + labelWidth + 2 * margin, splashTop, labelWidth, splashHeight,
    splashMargin, splashMargin, defaultWidth - 2 * splashMargin,
    defaultHeight - 2 * splashMargin, pluginNameTextSize, "GlitchSprinkler", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
