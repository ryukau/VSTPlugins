// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
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

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Gain.
  const auto gainLeft0 = left0;
  const auto gainTop0 = top0;
  addGroupLabel(gainLeft0, gainTop0, 2 * knobX, labelHeight, midTextSize, "Gain");

  const auto gainTop1 = gainTop0 + labelY;
  addKnob(gainLeft0, gainTop1, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(gainLeft0 + knobX, gainTop1, knobWidth, margin, uiTextSize, "Boost", ID::boost);

  // Compressor.
  const auto compressorLeft0 = gainLeft0;
  const auto compressorTop0 = gainTop0 + labelY + knobY + 2 * margin;
  addToggleButton(
    compressorLeft0, compressorTop0, 2 * knobX, labelHeight, uiTextSize, "Compressor",
    ID::compressor);

  const auto compressorLeft1 = compressorLeft0 + knobX;
  const auto compressorTop1 = compressorTop0 + labelY + 2 * margin;
  addLabel(compressorLeft0, compressorTop1, knobX, labelHeight, uiTextSize, "Time");
  addTextKnob(
    compressorLeft1, compressorTop1, knobX, labelHeight, uiTextSize, ID::compressorTime,
    Scales::compressorTime, false, 5);

  const auto compressorTop2 = compressorTop1 + labelY;
  addLabel(compressorLeft0, compressorTop2, knobX, labelHeight, uiTextSize, "Threshold");
  addTextKnob(
    compressorLeft1, compressorTop2, knobX, labelHeight, uiTextSize,
    ID::compressorThreshold, Scales::compressorThreshold, false, 5);

  // Tuning.
  const auto tuningLeft0 = gainLeft0 + 2 * knobX + 2 * margin;
  const auto tuningTop0 = gainTop0;
  addGroupLabel(
    tuningLeft0, tuningTop0, 2 * knobX - 2 * margin, labelHeight, midTextSize, "Tuning");

  const auto tuningLeft1 = tuningLeft0 + knobX - 2 * margin;
  const auto tuningTop1 = tuningTop0 + labelY;
  const auto tuningLabelWidth = knobX - 2 * margin;
  addLabel(tuningLeft0, tuningTop1, tuningLabelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    tuningLeft1, tuningTop1, knobX, labelHeight, uiTextSize, ID::octave, Scales::octave,
    false, 0, -12);

  const auto tuningTop2 = tuningTop1 + labelY;
  addLabel(tuningLeft0, tuningTop2, tuningLabelWidth, labelHeight, uiTextSize, "Semi");
  addTextKnob(
    tuningLeft1, tuningTop2, knobX, labelHeight, uiTextSize, ID::semitone,
    Scales::semitone, false, 0, -120);

  const auto tuningTop3 = tuningTop2 + labelY;
  addLabel(tuningLeft0, tuningTop3, tuningLabelWidth, labelHeight, uiTextSize, "Milli");
  auto knobOscMilli = addTextKnob(
    tuningLeft1, tuningTop3, knobX, labelHeight, uiTextSize, ID::milli, Scales::milli,
    false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;

  const auto tuningTop4 = tuningTop3 + labelY;
  addLabel(tuningLeft0, tuningTop4, tuningLabelWidth, labelHeight, uiTextSize, "ET");
  addTextKnob(
    tuningLeft1, tuningTop4, knobX, labelHeight, uiTextSize, ID::equalTemperament,
    Scales::equalTemperament, false, 0, 1);

  const auto tuningTop5 = tuningTop4 + labelY;
  addLabel(tuningLeft0, tuningTop5, tuningLabelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    tuningLeft1, tuningTop5, knobX, labelHeight, uiTextSize, ID::pitchA4Hz,
    Scales::pitchA4Hz, false, 0, 100);

  // Unison.
  const auto unisonLeft0 = gainLeft0;
  const auto unisonTop0 = compressorTop0 + 3 * labelY + 6 * margin;
  const auto unisonWidth = 4 * knobX;
  addGroupLabel(unisonLeft0, unisonTop0, unisonWidth, labelHeight, midTextSize, "Unison");

  const auto unisonLeft1 = unisonLeft0 + knobX;
  const auto unisonTop1 = unisonTop0 + labelY;
  const auto unisonLabelWidth = knobX - 2 * margin;
  addLabel(
    unisonLeft0, unisonTop1 + margin, unisonLabelWidth, labelHeight, uiTextSize,
    "nUnison");
  addTextKnob(
    unisonLeft0, unisonTop1 + labelY + margin, unisonLabelWidth, labelHeight, uiTextSize,
    ID::nUnison, Scales::nUnison, false, 0, 1);

  addKnob(
    unisonLeft1 + 0 * knobX, unisonTop1, knobWidth, margin, uiTextSize, "Detune",
    ID::unisonDetune);
  addKnob(
    unisonLeft1 + 1 * knobX, unisonTop1, knobWidth, margin, uiTextSize, "Spread",
    ID::unisonSpread);
  addKnob(
    unisonLeft1 + 2 * knobX, unisonTop1, knobWidth, margin, uiTextSize, "Gain",
    ID::unisonGainRandom);

  addCheckbox(
    unisonLeft0, unisonTop1 + knobY, 2 * knobX, labelHeight, uiTextSize, "Random Detune",
    ID::unisonDetuneRandom);

  // Random.
  const auto randomLeft0 = gainLeft0;
  const auto randomLeft1 = randomLeft0 + knobX;
  const auto randomLeft2 = randomLeft1 + knobX;
  const auto randomLeft3 = randomLeft2 + knobX;

  const auto randomTop0 = unisonTop0 + 2 * labelY + knobY + 2 * margin;
  const auto randomTop1 = randomTop0 + labelY;
  const auto randomTop2 = randomTop1 + labelY;
  const auto randomTop3 = randomTop2 + labelY;
  const auto randomTop4 = randomTop3 + labelY;
  const auto randomTop5 = randomTop4 + labelY;

  addGroupLabel(randomLeft0, randomTop0, 4 * knobX, labelHeight, midTextSize, "Random");

  addLabel(randomLeft0, randomTop2, knobX, labelHeight, uiTextSize, "Noise");
  addLabel(randomLeft0, randomTop3, knobX, labelHeight, uiTextSize, "Comb");
  addLabel(randomLeft0, randomTop4, knobX, labelHeight, uiTextSize, "Str.Freq");
  addLabel(randomLeft0, randomTop5, knobX, labelHeight, uiTextSize, "Unison");

  addLabel(randomLeft1, randomTop1, knobX, labelHeight, uiTextSize, "Amount");
  addTextKnob(
    randomLeft1, randomTop3, knobX, labelHeight, uiTextSize, ID::randomComb,
    Scales::defaultScale, false, 5);
  addTextKnob(
    randomLeft1, randomTop4, knobX, labelHeight, uiTextSize, ID::randomFrequency,
    Scales::defaultScale, false, 5);

  addLabel(randomLeft2, randomTop1, knobX, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    randomLeft2, randomTop2, knobX, labelHeight, uiTextSize, ID::seedNoise, Scales::seed);
  addTextKnob(
    randomLeft2, randomTop3, knobX, labelHeight, uiTextSize, ID::seedComb, Scales::seed);
  addTextKnob(
    randomLeft2, randomTop4, knobX, labelHeight, uiTextSize, ID::seedString,
    Scales::seed);
  addTextKnob(
    randomLeft2, randomTop5, knobX, labelHeight, uiTextSize, ID::seedUnison,
    Scales::seed);

  const auto retriggerOffsetX = floorf(knobX / 2 - 5.0f);
  addLabel(randomLeft3, randomTop1, knobX, labelHeight, uiTextSize, "Retrigger");
  addCheckbox(
    randomLeft3 + retriggerOffsetX, randomTop2, labelY, labelHeight, uiTextSize, "",
    ID::retriggerNoise);
  addCheckbox(
    randomLeft3 + retriggerOffsetX, randomTop3, labelY, labelHeight, uiTextSize, "",
    ID::retriggerComb);
  addCheckbox(
    randomLeft3 + retriggerOffsetX, randomTop4, labelY, labelHeight, uiTextSize, "",
    ID::retriggerString);
  addCheckbox(
    randomLeft3 + retriggerOffsetX, randomTop5, labelY, labelHeight, uiTextSize, "",
    ID::retriggerUnison);

  // Misc.
  const auto miscLeft0 = gainLeft0;
  const auto miscTop0 = randomTop0 + 6 * labelY;
  addGroupLabel(miscLeft0, miscTop0, 4 * knobX, labelHeight, midTextSize, "Misc.");

  const auto miscTop1 = miscTop0 + labelY;
  addLabel(miscLeft0, miscTop1, knobX, labelHeight, uiTextSize, "Poly");
  addTextKnob(
    miscLeft0 + knobX, miscTop1 + 0 * labelY, knobX, labelHeight, uiTextSize, ID::nVoice,
    Scales::nVoice, false, 0, 1);

  // Exciter.
  const auto exciterLeft0 = left0 + 4 * knobX + labelY;
  const auto exciterTop0 = top0;

  const auto exciterWidth = 8 * knobX;
  addGroupLabel(
    exciterLeft0, exciterTop0, exciterWidth, labelHeight, midTextSize, "Exciter");

  const auto exciterTop1 = exciterTop0 + labelY;

  const auto exciterLabelWidth = floorf(1.3f * knobWidth);
  const auto exciterLabelLeft1 = exciterLeft0 + floorf(1.5f * knobWidth);
  const auto exciterLabelTop = exciterTop1 + floorf(knobHeight / 2);
  addLabel(
    exciterLeft0, exciterLabelTop, exciterLabelWidth, labelHeight, uiTextSize,
    "LP Cutoff");
  addTextKnob(
    exciterLabelLeft1, exciterLabelTop, exciterLabelWidth, labelHeight, uiTextSize,
    ID::exciterLowpassCutoff, Scales::exciterLowpassCutoff, false, 2);

  const auto exciterLeft1 = exciterLeft0 + 3 * knobX;
  addKnob(
    exciterLeft1 + 0 * knobX, exciterTop1, knobWidth, margin, uiTextSize, "Gain",
    ID::exciterGain);
  addKnob(
    exciterLeft1 + 1 * knobX, exciterTop1, knobWidth, margin, uiTextSize, "A",
    ID::exciterAttack);
  addKnob(
    exciterLeft1 + 2 * knobX, exciterTop1, knobWidth, margin, uiTextSize, "D",
    ID::exciterDecay);
  addKnob(
    exciterLeft1 + 3 * knobX, exciterTop1, knobWidth, margin, uiTextSize, "Saw/Noise",
    ID::exciterNoiseMix);

  // Comb.
  const auto combLeft0 = exciterLeft0;
  const auto combLeft1 = combLeft0 + labelY;
  const auto combTop0 = exciterTop0 + labelY + knobY + 2 * margin;
  addGroupVerticalLabel(
    combLeft0, combTop0, barboxHeight, labelHeight, uiTextSize, "Comb Time");

  const auto barboxCombWidth = floorf((exciterWidth - labelY) / nComb) * nComb;
  addBarBox(
    combLeft1, combTop0, barboxCombWidth, barboxHeight, ID::combTime0, nComb,
    Scales::combTime, "Comb Time [s]");

  // String.
  const auto stringLeft0 = exciterLeft0;
  const auto stringTop0 = exciterTop0 + labelY + knobY + barboxHeight + 6 * margin;

  const auto stringWidth = 8 * knobX;
  addGroupLabel(stringLeft0, stringTop0, stringWidth, labelHeight, midTextSize, "String");

  const auto stringTop1 = stringTop0 + labelY;
  const auto stringLabelWidth = floorf(1.3f * knobWidth);
  const auto stringLabelLeft1 = stringLeft0 + floorf(1.5f * knobWidth);
  addLabel(
    stringLeft0, stringTop1, stringLabelWidth, labelHeight, uiTextSize, "LP Cutoff");
  addTextKnob(
    stringLabelLeft1, stringTop1, stringLabelWidth, labelHeight, uiTextSize,
    ID::lowpassCutoff, Scales::lowpassCutoff, false, 2);

  const auto stringTop2 = stringTop1 + labelY;
  addLabel(
    stringLeft0, stringTop2, stringLabelWidth, labelHeight, uiTextSize, "HP Cutoff");
  addTextKnob(
    stringLabelLeft1, stringTop2, stringLabelWidth, labelHeight, uiTextSize,
    ID::highpassCutoff, Scales::highpassCutoff, false, 2);

  const auto stringTop3 = stringTop1 + knobY;
  addLabel(
    stringLeft0, stringTop3, stringLabelWidth, labelHeight, uiTextSize, "Distance");
  addTextKnob(
    stringLabelLeft1, stringTop3, stringLabelWidth, labelHeight, uiTextSize, ID::distance,
    Scales::distance, false, 6);

  addLabel(
    stringLeft0 + floorf(3.25f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, "Propagation");
  addTextKnob(
    stringLeft0 + floorf(4.75f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, ID::propagation, Scales::propagation, false, 6);

  addLabel(
    stringLeft0 + floorf(6.5f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, "Connection");
  std::vector<std::string> connectionItems = {"Parallel", "Serial"};
  addOptionMenu(
    stringLeft0 + floorf(8.0f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, ID::connection, connectionItems);

  // String lowpass envelope.
  const auto stringLeft1 = stringLeft0 + 3 * knobX;
  addKnob(
    stringLeft1 + 0 * knobX, stringTop1, knobWidth, margin, uiTextSize, "A",
    ID::lowpassA);
  addKnob(
    stringLeft1 + 1 * knobX, stringTop1, knobWidth, margin, uiTextSize, "D",
    ID::lowpassD);
  addKnob(
    stringLeft1 + 2 * knobX, stringTop1, knobWidth, margin, uiTextSize, "S",
    ID::lowpassS);
  addKnob(
    stringLeft1 + 3 * knobX, stringTop1, knobWidth, margin, uiTextSize, "R",
    ID::lowpassR);

  // String Frequency.
  const auto strFreqLeft0 = stringLeft0;
  const auto strFreqLeft1 = strFreqLeft0 + labelY;
  const auto strFreqTop0 = stringTop0 + 4 * labelY + 2 * margin;
  addGroupVerticalLabel(
    strFreqLeft0, strFreqTop0, barboxHeight, labelHeight, uiTextSize, "Frequency");

  const auto barboxStrFreqWidth = floorf((stringWidth - labelY) / nDelay) * nDelay;
  addBarBox(
    strFreqLeft1, strFreqTop0, barboxStrFreqWidth, barboxHeight, ID::frequency0, nDelay,
    Scales::frequency, "Frequency [Hz]");

  // Plugin name.
  const auto splashTop = innerHeight - splashHeight + uiMargin;
  const auto splashLeft = uiMargin;
  addSplashScreen(
    splashLeft, splashTop, 4 * knobX, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "CollidingCombSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
