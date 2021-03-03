// (c) 2020 Takamitsu Endo
//
// This file is part of CollidingCombSynth.
//
// CollidingCombSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CollidingCombSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CollidingCombSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float uiMargin = 20.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float splashHeight = 40.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float barboxHeight = 3.0f * knobY - 2 * margin;
constexpr float innerWidth = 12 * knobX + 6 * margin;
constexpr float innerHeight = 15 * labelY + 2 * knobY + 14 * margin;
constexpr uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  const auto &scale = param.scale;
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = 20.0f;
  constexpr auto left0 = 20.0f;

  // Gain.
  constexpr auto gainLeft0 = left0;
  constexpr auto gainTop0 = top0;
  addGroupLabel(gainLeft0, gainTop0, 2 * knobX, labelHeight, midTextSize, "Gain");

  constexpr auto gainTop1 = gainTop0 + labelY;
  addKnob(gainLeft0, gainTop1, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(gainLeft0 + knobX, gainTop1, knobWidth, margin, uiTextSize, "Boost", ID::boost);

  // Compressor.
  constexpr auto compressorLeft0 = gainLeft0;
  constexpr auto compressorTop0 = gainTop0 + labelY + knobY + 2 * margin;
  addToggleButton(
    compressorLeft0, compressorTop0, 2 * knobX, labelHeight, uiTextSize, "Compressor",
    ID::compressor);

  constexpr auto compressorLeft1 = compressorLeft0 + knobX;
  constexpr auto compressorTop1 = compressorTop0 + labelY + 2 * margin;
  addLabel(compressorLeft0, compressorTop1, knobX, labelHeight, uiTextSize, "Time");
  addTextKnob(
    compressorLeft1, compressorTop1, knobX, labelHeight, uiTextSize, ID::compressorTime,
    scale.compressorTime, false, 5);

  const auto compressorTop2 = compressorTop1 + labelY;
  addLabel(compressorLeft0, compressorTop2, knobX, labelHeight, uiTextSize, "Threshold");
  addTextKnob(
    compressorLeft1, compressorTop2, knobX, labelHeight, uiTextSize,
    ID::compressorThreshold, scale.compressorThreshold, false, 5);

  // Tuning.
  constexpr auto tuningLeft0 = gainLeft0 + 2 * knobX + 2 * margin;
  constexpr auto tuningTop0 = gainTop0;
  addGroupLabel(
    tuningLeft0, tuningTop0, 2 * knobX - 2 * margin, labelHeight, midTextSize, "Tuning");

  constexpr auto tuningLeft1 = tuningLeft0 + knobX - 2 * margin;
  constexpr auto tuningTop1 = tuningTop0 + labelY;
  constexpr auto tuningLabelWidth = knobX - 2 * margin;
  addLabel(tuningLeft0, tuningTop1, tuningLabelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    tuningLeft1, tuningTop1, knobX, labelHeight, uiTextSize, ID::octave, scale.octave,
    false, 0, -12);

  constexpr auto tuningTop2 = tuningTop1 + labelY;
  addLabel(tuningLeft0, tuningTop2, tuningLabelWidth, labelHeight, uiTextSize, "Semi");
  addTextKnob(
    tuningLeft1, tuningTop2, knobX, labelHeight, uiTextSize, ID::semitone, scale.semitone,
    false, 0, -120);

  constexpr auto tuningTop3 = tuningTop2 + labelY;
  addLabel(tuningLeft0, tuningTop3, tuningLabelWidth, labelHeight, uiTextSize, "Milli");
  auto knobOscMilli = addTextKnob(
    tuningLeft1, tuningTop3, knobX, labelHeight, uiTextSize, ID::milli, scale.milli,
    false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;

  constexpr auto tuningTop4 = tuningTop3 + labelY;
  addLabel(tuningLeft0, tuningTop4, tuningLabelWidth, labelHeight, uiTextSize, "ET");
  addTextKnob(
    tuningLeft1, tuningTop4, knobX, labelHeight, uiTextSize, ID::equalTemperament,
    scale.equalTemperament, false, 0, 1);

  constexpr auto tuningTop5 = tuningTop4 + labelY;
  addLabel(tuningLeft0, tuningTop5, tuningLabelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    tuningLeft1, tuningTop5, knobX, labelHeight, uiTextSize, ID::pitchA4Hz,
    scale.pitchA4Hz, false, 0, 100);

  // Unison.
  constexpr auto unisonLeft0 = gainLeft0;
  constexpr auto unisonTop0 = compressorTop0 + 3 * labelY + 6 * margin;
  constexpr auto unisonWidth = 4 * knobX;
  addGroupLabel(unisonLeft0, unisonTop0, unisonWidth, labelHeight, midTextSize, "Unison");

  constexpr auto unisonLeft1 = unisonLeft0 + knobX;
  constexpr auto unisonTop1 = unisonTop0 + labelY;
  constexpr auto unisonLabelWidth = knobX - 2 * margin;
  addLabel(
    unisonLeft0, unisonTop1 + margin, unisonLabelWidth, labelHeight, uiTextSize,
    "nUnison");
  addTextKnob(
    unisonLeft0, unisonTop1 + labelY + margin, unisonLabelWidth, labelHeight, uiTextSize,
    ID::nUnison, scale.nUnison, false, 0, 1);

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
  constexpr auto randomLeft0 = gainLeft0;
  constexpr auto randomLeft1 = randomLeft0 + knobX;
  constexpr auto randomLeft2 = randomLeft1 + knobX;
  constexpr auto randomLeft3 = randomLeft2 + knobX;

  constexpr auto randomTop0 = unisonTop0 + 2 * labelY + knobY + 2 * margin;
  constexpr auto randomTop1 = randomTop0 + labelY;
  constexpr auto randomTop2 = randomTop1 + labelY;
  constexpr auto randomTop3 = randomTop2 + labelY;
  constexpr auto randomTop4 = randomTop3 + labelY;
  constexpr auto randomTop5 = randomTop4 + labelY;

  addGroupLabel(randomLeft0, randomTop0, 4 * knobX, labelHeight, midTextSize, "Random");

  addLabel(randomLeft0, randomTop2, knobX, labelHeight, uiTextSize, "Noise");
  addLabel(randomLeft0, randomTop3, knobX, labelHeight, uiTextSize, "Comb");
  addLabel(randomLeft0, randomTop4, knobX, labelHeight, uiTextSize, "Str.Freq");
  addLabel(randomLeft0, randomTop5, knobX, labelHeight, uiTextSize, "Unison");

  addLabel(randomLeft1, randomTop1, knobX, labelHeight, uiTextSize, "Amount");
  addTextKnob(
    randomLeft1, randomTop3, knobX, labelHeight, uiTextSize, ID::randomComb,
    scale.defaultScale, false, 5);
  addTextKnob(
    randomLeft1, randomTop4, knobX, labelHeight, uiTextSize, ID::randomFrequency,
    scale.defaultScale, false, 5);

  addLabel(randomLeft2, randomTop1, knobX, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    randomLeft2, randomTop2, knobX, labelHeight, uiTextSize, ID::seedNoise, scale.seed);
  addTextKnob(
    randomLeft2, randomTop3, knobX, labelHeight, uiTextSize, ID::seedComb, scale.seed);
  addTextKnob(
    randomLeft2, randomTop4, knobX, labelHeight, uiTextSize, ID::seedString, scale.seed);
  addTextKnob(
    randomLeft2, randomTop5, knobX, labelHeight, uiTextSize, ID::seedUnison, scale.seed);

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
  constexpr auto miscLeft0 = gainLeft0;
  constexpr auto miscTop0 = randomTop0 + 6 * labelY;
  addGroupLabel(miscLeft0, miscTop0, 4 * knobX, labelHeight, midTextSize, "Misc.");

  constexpr auto miscTop1 = miscTop0 + labelY;
  addLabel(miscLeft0, miscTop1, knobX, labelHeight, uiTextSize, "Poly");
  addTextKnob(
    miscLeft0 + knobX, miscTop1 + 0 * labelY, knobX, labelHeight, uiTextSize, ID::nVoice,
    scale.nVoice, false, 0, 1);

  // Exciter.
  constexpr auto exciterLeft0 = left0 + 4 * knobX + labelY;
  constexpr auto exciterTop0 = top0;

  constexpr auto exciterWidth = 8 * knobX;
  addGroupLabel(
    exciterLeft0, exciterTop0, exciterWidth, labelHeight, midTextSize, "Exciter");

  constexpr auto exciterTop1 = exciterTop0 + labelY;

  const auto exciterLabelWidth = floorf(1.3f * knobWidth);
  const auto exciterLabelLeft1 = exciterLeft0 + floorf(1.5f * knobWidth);
  const auto exciterLabelTop = exciterTop1 + floorf(knobHeight / 2);
  addLabel(
    exciterLeft0, exciterLabelTop, exciterLabelWidth, labelHeight, uiTextSize,
    "LP Cutoff");
  addTextKnob(
    exciterLabelLeft1, exciterLabelTop, exciterLabelWidth, labelHeight, uiTextSize,
    ID::exciterLowpassCutoff, scale.exciterLowpassCutoff, false, 2);

  constexpr auto exciterLeft1 = exciterLeft0 + 3 * knobX;
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
  constexpr auto combLeft0 = exciterLeft0;
  constexpr auto combLeft1 = combLeft0 + labelY;
  constexpr auto combTop0 = exciterTop0 + labelY + knobY + 2 * margin;
  addGroupVerticalLabel(
    combLeft0, combTop0, barboxHeight, labelHeight, uiTextSize, "Comb Time");

  const auto barboxCombWidth = floorf((exciterWidth - labelY) / nComb) * nComb;
  addBarBox(
    combLeft1, combTop0, barboxCombWidth, barboxHeight, ID::combTime0, nComb,
    scale.combTime, "Comb Time [s]");

  // String.
  constexpr auto stringLeft0 = exciterLeft0;
  constexpr auto stringTop0 = exciterTop0 + labelY + knobY + barboxHeight + 6 * margin;

  constexpr auto stringWidth = 8 * knobX;
  addGroupLabel(stringLeft0, stringTop0, stringWidth, labelHeight, midTextSize, "String");

  constexpr auto stringTop1 = stringTop0 + labelY;
  const auto stringLabelWidth = floorf(1.3f * knobWidth);
  const auto stringLabelLeft1 = stringLeft0 + floorf(1.5f * knobWidth);
  addLabel(
    stringLeft0, stringTop1, stringLabelWidth, labelHeight, uiTextSize, "LP Cutoff");
  addTextKnob(
    stringLabelLeft1, stringTop1, stringLabelWidth, labelHeight, uiTextSize,
    ID::lowpassCutoff, scale.lowpassCutoff, false, 2);

  constexpr auto stringTop2 = stringTop1 + labelY;
  addLabel(
    stringLeft0, stringTop2, stringLabelWidth, labelHeight, uiTextSize, "HP Cutoff");
  addTextKnob(
    stringLabelLeft1, stringTop2, stringLabelWidth, labelHeight, uiTextSize,
    ID::highpassCutoff, scale.highpassCutoff, false, 2);

  constexpr auto stringTop3 = stringTop1 + knobY;
  addLabel(
    stringLeft0, stringTop3, stringLabelWidth, labelHeight, uiTextSize, "Distance");
  addTextKnob(
    stringLabelLeft1, stringTop3, stringLabelWidth, labelHeight, uiTextSize, ID::distance,
    scale.distance, false, 6);

  addLabel(
    stringLeft0 + floorf(3.25f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, "Propagation");
  addTextKnob(
    stringLeft0 + floorf(4.75f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, ID::propagation, scale.propagation, false, 6);

  addLabel(
    stringLeft0 + floorf(6.5f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, "Connection");
  std::vector<std::string> connectionItems = {"Parallel", "Serial"};
  addOptionMenu(
    stringLeft0 + floorf(8.0f * knobWidth), stringTop3, stringLabelWidth, labelHeight,
    uiTextSize, ID::connection, connectionItems);

  // String lowpass envelope.
  constexpr auto stringLeft1 = stringLeft0 + 3 * knobX;
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
  constexpr auto strFreqLeft0 = stringLeft0;
  constexpr auto strFreqLeft1 = strFreqLeft0 + labelY;
  const auto strFreqTop0 = stringTop0 + 4 * labelY + 2 * margin;
  addGroupVerticalLabel(
    strFreqLeft0, strFreqTop0, barboxHeight, labelHeight, uiTextSize, "Frequency");

  const auto barboxStrFreqWidth = floorf((stringWidth - labelY) / nDelay) * nDelay;
  addBarBox(
    strFreqLeft1, strFreqTop0, barboxStrFreqWidth, barboxHeight, ID::frequency0, nDelay,
    scale.frequency, "Frequency [Hz]");

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
