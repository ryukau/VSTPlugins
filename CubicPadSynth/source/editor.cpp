// (c) 2020 Takamitsu Endo
//
// This file is part of CubicPadSynth.
//
// CubicPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CubicPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CubicPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 14.0f;
constexpr float midTextSize = 16.0f;
constexpr float infoTextSize = 18.0f;
constexpr float pluginNameTextSize = 22.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float scrollBarHeight = 10.0f;
constexpr float barboxWidth = 12.0f * knobX;
constexpr float barboxHeight = 2.0f * knobY - scrollBarHeight + margin;
constexpr float barboxY = barboxHeight + scrollBarHeight + margin;
constexpr float checkboxWidth = 60.0f;
constexpr float splashHeight = 40.0f;
constexpr uint32_t defaultWidth
  = uint32_t(barboxWidth + labelY + 2 * knobX + 12 * margin + 40);
constexpr uint32_t defaultHeight = uint32_t(40 + labelY + 4 * barboxY + 9 * margin);

enum tabIndex { tabMain, tabPadSynth, tabInfo };

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

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // TabView test.
  const auto tabTop0 = top0;
  const auto tabLeft0 = left0;

  std::vector<std::string> tabs = {"Main", "Wavetable", "Information"};
  const auto tabWidth = defaultWidth - 40.0f;
  const auto tabHeight = labelY + 4.0f * barboxY + 9.0f * margin;
  auto tabview
    = addTabView(tabLeft0, tabTop0, tabWidth, tabHeight, uiTextSize, labelY, tabs);

  const auto tabInsideTop0 = tabTop0 + labelY + 4.0f * margin;
  const auto tabInsideLeft0 = tabLeft0 + 4.0f * margin;
  const auto tabCenterX = tabInsideLeft0 + labelY + 7.0f * knobX + 4.0f * margin;

  // Tuning.
  const auto tuningTop = tabInsideTop0;
  const auto tuningLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(
    tabMain,
    addGroupLabel(
      tuningLeft, tuningTop, 2.0f * knobX - 4.0 * margin, labelHeight, midTextSize,
      "Tuning"));

  const auto tuningLabelWidth = knobX - 2.0f * margin;
  const auto tuningLeft1 = tuningLeft;
  const auto tuningLeft2 = tuningLeft1 + tuningLabelWidth;

  const auto tuningTop1 = tuningTop + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(
      tuningLeft1, tuningTop1, tuningLabelWidth, labelHeight, uiTextSize, "Octave"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop1, knobWidth, labelHeight, uiTextSize, ID::oscOctave,
      Scales::oscOctave, false, 0, -12));

  const auto tuningTop2 = tuningTop1 + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(tuningLeft1, tuningTop2, tuningLabelWidth, labelHeight, uiTextSize, "Semi"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop2, knobWidth, labelHeight, uiTextSize, ID::oscSemi,
      Scales::oscSemi, false, 0, -120));

  const auto tuningTop3 = tuningTop2 + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(
      tuningLeft1, tuningTop3, tuningLabelWidth, labelHeight, uiTextSize, "Milli"));
  auto knobOscMilli = addTextKnob(
    tuningLeft2, tuningTop3, knobWidth, labelHeight, uiTextSize, ID::oscMilli,
    Scales::oscMilli, false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;
  tabview->addWidget(tabMain, knobOscMilli);

  const auto tuningTop4 = tuningTop3 + 1.5f * labelY;
  tabview->addWidget(
    tabMain,
    addLabel(tuningLeft1, tuningTop4, tuningLabelWidth, labelHeight, uiTextSize, "ET"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop4, knobWidth, labelHeight, uiTextSize, ID::equalTemperament,
      Scales::equalTemperament, false, 0, 1));

  const auto tuningTop5 = tuningTop4 + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(
      tuningLeft1, tuningTop5, tuningLabelWidth, labelHeight, uiTextSize, "A4 [Hz]"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop5, knobWidth, labelHeight, uiTextSize, ID::pitchA4Hz,
      Scales::pitchA4Hz, false, 0, 100));

  const auto tuningOffsetX = 2.0f * knobX;

  // Gain.
  const auto gainTop = tabInsideTop0;
  const auto gainLeft = tabInsideLeft0 + labelY + tuningOffsetX;
  tabview->addWidget(
    tabMain,
    addGroupLabel(gainLeft, gainTop, 5.0f * knobX, labelHeight, midTextSize, "Gain"));
  const auto gainKnobTop = gainTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(gainLeft, gainKnobTop, knobWidth, margin, uiTextSize, "A", ID::gainA));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 1.0f * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "D",
      ID::gainD));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 2.0f * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "S",
      ID::gainS));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 3.0f * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "R",
      ID::gainR));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 4.0f * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Gain",
      ID::gain));

  // Lowpass.
  const auto filterTop = tabInsideTop0;
  const auto filterLeft = tabCenterX;
  tabview->addWidget(
    tabMain,
    addGroupLabel(
      filterLeft, filterTop, 7.0f * knobX, labelHeight, midTextSize, "Lowpass"));
  const auto filterKnobTop = filterTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft, filterKnobTop, knobWidth, margin, uiTextSize, "Cutoff",
      ID::tableLowpass));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + knobX, filterKnobTop, knobWidth, margin, uiTextSize, "KeyFollow",
      ID::tableLowpassKeyFollow));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 2.0f * knobX, filterKnobTop, knobWidth, margin, uiTextSize, "A",
      ID::tableLowpassA));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 3.0f * knobX, filterKnobTop, knobWidth, margin, uiTextSize, "D",
      ID::tableLowpassD));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 4.0f * knobX, filterKnobTop, knobWidth, margin, uiTextSize, "S",
      ID::tableLowpassS));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 5.0f * knobX, filterKnobTop, knobWidth, margin, uiTextSize, "R",
      ID::tableLowpassR));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 6.0f * knobX, filterKnobTop, knobWidth, margin, uiTextSize, "Amount",
      ID::tableLowpassEnvelopeAmount));

  // Pitch.
  const auto pitchTop = filterTop + labelY + knobY;
  const auto pitchLeft = tabInsideLeft0 + labelY + tuningOffsetX;
  tabview->addWidget(
    tabMain,
    addGroupLabel(pitchLeft, pitchTop, 5.0f * knobX, labelHeight, midTextSize, "Pitch"));

  const auto pitchKnobTop = pitchTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(pitchLeft, pitchKnobTop, knobWidth, margin, uiTextSize, "A", ID::pitchA));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 1.0f * knobX, pitchKnobTop, knobWidth, margin, uiTextSize, "D",
      ID::pitchD));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 2.0f * knobX, pitchKnobTop, knobWidth, margin, uiTextSize, "S",
      ID::pitchS));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 3.0f * knobX, pitchKnobTop, knobWidth, margin, uiTextSize, "R",
      ID::pitchR));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 4.0f * knobX, pitchKnobTop, knobWidth, margin, uiTextSize, "Amount",
      ID::pitchEnvelopeAmount));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      floor(pitchLeft + 4.0f * knobX - 1.5f * margin), pitchKnobTop + knobY,
      1.5f * knobWidth, labelHeight, uiTextSize, "Negative",
      ID::pitchEnvelopeAmountNegative));

  // Unison.
  const auto unisonTop = pitchTop;
  const auto unisonLeft = tabCenterX;
  tabview->addWidget(
    tabMain,
    addGroupLabel(
      unisonLeft, unisonTop, 7.0f * knobX, labelHeight, midTextSize, "Unison"));
  const auto unisonKnobTop = unisonTop + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(unisonLeft, unisonKnobTop, knobWidth, labelHeight, uiTextSize, "nUnison"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      unisonLeft, unisonKnobTop + labelHeight + margin, knobWidth, labelHeight,
      uiTextSize, ID::nUnison, Scales::nUnison, false, 0, 1));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "Detune",
      ID::unisonDetune));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 2.0f * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "GainRnd",
      ID::unisonGainRandom));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 3.0f * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "Phase",
      ID::unisonPhase));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 4.0f * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "Spread",
      ID::unisonPan));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      floor(unisonLeft + knobX - 1.5f * margin), unisonKnobTop + knobY,
      floor(2.75f * knobWidth), labelHeight, uiTextSize, "Random Detune",
      ID::unisonDetuneRandom));

  std::vector<std::string> unisonPanTypeOptions{
    "Alternate L-R", "Alternate M-S", "Ascend L -> R", "Ascend R -> L", "High on Mid",
    "High on Side",  "Random",        "Rotate L",      "Rotate R",      "Shuffle"};
  tabview->addWidget(
    tabMain,
    addLabel(
      unisonLeft + 5.0f * knobX, unisonKnobTop, 2.0f * knobWidth, labelHeight, uiTextSize,
      "Spread Type"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      unisonLeft + 5.0f * knobX, unisonKnobTop + labelY, 2.0f * knobWidth, labelHeight,
      uiTextSize, ID::unisonPanType, unisonPanTypeOptions));

  // LFO.
  const auto lfoTop = unisonTop + 2.0f * labelY + knobY;
  const auto lfoLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(
    tabMain,
    addGroupLabel(lfoLeft, lfoTop, 10.0f * knobX, labelHeight, midTextSize, "LFO"));
  const auto lfoKnobTop = lfoTop + labelY;

  tabview->addWidget(
    tabMain,
    addKickButton<Style::warning>(
      lfoLeft, lfoKnobTop + 2.0f * margin, 2.0f * knobX, 2.0f * labelHeight, midTextSize,
      "Refresh LFO", ID::refreshLFO));

  std::vector<std::string> lfoWavetableTypeOptions{"Step", "Linear", "Cubic"};
  tabview->addWidget(
    tabMain,
    addLabel(
      lfoLeft + 2.5f * knobX, lfoKnobTop, 1.55f * knobWidth, labelHeight, uiTextSize,
      "Interpolation"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      lfoLeft + 2.5f * knobX, lfoKnobTop + labelY, 1.55f * knobWidth, labelHeight,
      uiTextSize, ID::lfoWavetableType, lfoWavetableTypeOptions));

  const auto lfoLeft1 = lfoLeft + 4.0f * knobX;
  tabview->addWidget(
    tabMain,
    addLabel(
      lfoLeft1 + margin, lfoKnobTop + margin + 0.5f * labelHeight, knobWidth, labelHeight,
      uiTextSize, "Tempo"));

  auto knobLfoTempoNumerator = addTextKnob(
    lfoLeft1 + knobX, lfoKnobTop + margin, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoNumerator, Scales::lfoTempoNumerator, false, 0, 1);
  knobLfoTempoNumerator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoNumerator);

  auto knobLfoTempoDenominator = addTextKnob(
    lfoLeft1 + knobX, lfoKnobTop + labelHeight + 1.0f + margin, knobWidth, labelHeight,
    uiTextSize, ID::lfoTempoDenominator, Scales::lfoTempoDenominator, false, 0, 1);
  knobLfoTempoDenominator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoDenominator);

  tabview->addWidget(
    tabMain,
    addCheckbox(
      lfoLeft1 + 0.5f * knobWidth, lfoKnobTop + labelHeight + labelY, 1.5f * knobX,
      labelHeight, uiTextSize, "Retrigger", ID::lfoPhaseReset));

  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 2.0f * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Multiply",
      ID::lfoFrequencyMultiplier));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 3.0f * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Amount",
      ID::lfoPitchAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 4.0f * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Lowpass",
      ID::lfoLowpass));

  // Phase.
  const auto phaseTop = lfoTop;
  const auto phaseLeft = lfoLeft + 10.0f * knobX + 2.0f * margin;
  tabview->addWidget(
    tabMain,
    addGroupLabel(phaseLeft, phaseTop, 2.0f * knobX, labelHeight, midTextSize, "Phase"));
  const auto phaseKnobTop = phaseTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(
      phaseLeft, phaseKnobTop, knobWidth, margin, uiTextSize, "Phase",
      ID::oscInitialPhase));

  const auto phaseLeft1 = phaseLeft + knobX;
  tabview->addWidget(
    tabMain,
    addCheckbox(
      phaseLeft1, phaseKnobTop, 1.25f * knobWidth, labelHeight, uiTextSize, "Reset",
      ID::oscPhaseReset));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      phaseLeft1, phaseKnobTop + labelY, 1.25f * knobWidth, labelHeight, uiTextSize,
      "Random", ID::oscPhaseRandom));

  // Misc.
  const auto miscTop = phaseTop;
  const auto miscLeft = phaseLeft + 2.0f * knobX + 2.0f * margin;
  tabview->addWidget(
    tabMain,
    addGroupLabel(miscLeft, miscTop, 2.0f * knobX, labelHeight, midTextSize, "Misc."));

  const auto miscTop0 = miscTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(miscLeft, miscTop0, knobWidth, margin, uiTextSize, "Smooth", ID::smoothness));

  const auto miscLeft0 = miscLeft + knobX - (checkboxWidth - knobWidth) / 2.0f;
  std::vector<std::string> nVoiceOptions
    = {"16 Voices", "32 Voices", "48 Voices",  "64 Voices",
       "80 Voices", "96 Voices", "112 Voices", "128 Voices"};
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      miscLeft0, miscTop0, checkboxWidth, labelHeight, uiTextSize, ID::nVoice,
      nVoiceOptions));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      miscLeft0, miscTop0 + labelY, knobWidth, labelHeight, uiTextSize, "Pool",
      ID::voicePool));

  // LFO wavetable.
  const auto lfoWaveTop = lfoKnobTop + knobY + 0.5f * labelY;
  const auto lfoWaveLeft = tabInsideLeft0;
  const auto lfoBarboxHeight = barboxHeight + 3.5f * labelY + 3.0f * margin;
  tabview->addWidget(
    tabMain,
    addGroupVerticalLabel(
      lfoWaveLeft, lfoWaveTop, lfoBarboxHeight, labelHeight, midTextSize, "LFO Wave"));
  // 832 = 64 * 13.
  auto barboxLfoWavetable = addBarBox(
    lfoWaveLeft + labelY, lfoWaveTop, 832, lfoBarboxHeight, ID::lfoWavetable0,
    nLFOWavetable, Scales::lfoWavetable, "LFO Wave");
  barboxLfoWavetable->sliderZero = 0.5f;
  tabview->addWidget(tabMain, barboxLfoWavetable);

  // Wavetable pitch.
  const auto tablePitchTop = tabInsideTop0;
  const auto tablePitchLeft0 = tabInsideLeft0;
  const auto tablePitchLeft1 = tablePitchLeft0 + knobX;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tablePitchLeft0, tablePitchTop, 2.0f * knobX, labelHeight, midTextSize, "Pitch"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tablePitchLeft0, tablePitchTop + labelY, knobX, labelHeight, uiTextSize,
      "Base Freq."));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + labelY, knobX, labelHeight, uiTextSize,
      ID::tableBaseFrequency, Scales::tableBaseFrequency, false, 2));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tablePitchLeft0, tablePitchTop + 2.0f * labelY, knobX, labelHeight, uiTextSize,
      "Multiply"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 2.0f * labelY, knobX, labelHeight, uiTextSize,
      ID::overtonePitchMultiply, Scales::overtonePitchMultiply, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tablePitchLeft0, tablePitchTop + 3.0f * labelY, knobX, labelHeight, uiTextSize,
      "Modulo"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 3.0f * labelY, knobX, labelHeight, uiTextSize,
      ID::overtonePitchModulo, Scales::overtonePitchModulo, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePitchLeft0, tablePitchTop + 4.0f * labelY, 2.0f * checkboxWidth, labelHeight,
      uiTextSize, "Random", ID::overtonePitchRandom));

  const auto tableSpectrumTop = tablePitchTop + 5.0f * labelY;
  const auto tableSpectrumLeft0 = tablePitchLeft0;
  const auto tableSpectrumLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableSpectrumLeft0, tableSpectrumTop, 2.0f * knobX, labelHeight, midTextSize,
      "Spectrum"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + labelY, knobX, labelHeight, uiTextSize,
      "Expand"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + labelY, knobX, labelHeight, uiTextSize,
      ID::spectrumExpand, Scales::spectrumExpand, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + 2.0f * labelY, knobX, labelHeight,
      uiTextSize, "Shift"));
  auto knobSpectrumShift = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 2.0f * labelY, knobX, labelHeight, uiTextSize,
    ID::spectrumShift, Scales::spectrumShift, false, 0, -spectrumSize);
  knobSpectrumShift->sensitivity = 1.0f / spectrumSize;
  knobSpectrumShift->lowSensitivity = 0.08f / spectrumSize;
  tabview->addWidget(tabPadSynth, knobSpectrumShift);

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + 3.0 * labelY, knobX, labelHeight, uiTextSize,
      "Comb"));
  auto knobProfileComb = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 3.0 * labelY, knobX, labelHeight, uiTextSize,
    ID::profileComb, Scales::profileComb);
  knobProfileComb->sensitivity = 0.002;
  knobProfileComb->lowSensitivity = 0.0002;
  tabview->addWidget(tabPadSynth, knobProfileComb);

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + 4.0 * labelY, knobX, labelHeight, uiTextSize,
      "Shape"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 4.0 * labelY, knobX, labelHeight, uiTextSize,
      ID::profileShape, Scales::profileShape, false, 4, 0));

  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tableSpectrumLeft0, tableSpectrumTop + 5.0f * labelY, 2.0f * checkboxWidth,
      labelHeight, uiTextSize, "Invert", ID::spectrumInvert));

  const auto tablePhaseTop = tableSpectrumTop + 6.0f * labelY;
  const auto tablePhaseLeft0 = tablePitchLeft0;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tablePhaseLeft0, tablePhaseTop, 2.0f * knobX, labelHeight, midTextSize, "Phase"));
  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePhaseLeft0, tablePhaseTop + labelY, 2.0f * checkboxWidth, labelHeight,
      uiTextSize, "UniformPhase", ID::uniformPhaseProfile));

  // Wavetable random.
  const auto tableRandomTop = tablePhaseTop + 2.0f * labelY;
  const auto tableRandomLeft0 = tablePitchLeft0;
  const auto tableRandomLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableRandomLeft0, tableRandomTop, 2.0f * knobX, labelHeight, midTextSize,
      "Random"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableRandomLeft0, tableRandomTop + labelY, knobX, labelHeight, uiTextSize, "Seed"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableRandomLeft1, tableRandomTop + labelY, knobX, labelHeight, uiTextSize,
      ID::padSynthSeed, Scales::seed));

  // Wavetable modifier.
  const auto tableModifierTop = tableRandomTop + 2.0f * labelY;
  const auto tableModifierLeft0 = tablePitchLeft0;
  const auto tableModifierLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableModifierLeft0, tableModifierTop, 2.0f * knobX, labelHeight, midTextSize,
      "Modifier"));

  const auto tableModifierTop0 = tableModifierTop + labelY;
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft0, tableModifierTop0, knobWidth, margin, uiTextSize, "Gain^",
      ID::overtoneGainPower));
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft1, tableModifierTop0, knobWidth, margin, uiTextSize, "Width*",
      ID::overtoneWidthMultiply));

  // Refresh button.
  const auto refreshTop = tabTop0 + tabHeight - 2.0f * labelY;
  const auto refreshLeft = tabInsideLeft0;
  tabview->addWidget(
    tabPadSynth,
    addKickButton<Style::warning>(
      refreshLeft, refreshTop, 2.0f * knobX, 2.0f * labelHeight, midTextSize,
      "Refresh Table", ID::refreshTable));

  // Overtone Gain.
  const auto otGainTop = tabInsideTop0;
  const auto otGainLeft = tabInsideLeft0 + 2.0f * knobX + 4.0f * margin;
  tabview->addWidget(
    tabPadSynth,
    addGroupVerticalLabel(
      otGainLeft, otGainTop, barboxHeight, labelHeight, midTextSize, "Gain"));

  const auto otGainLeft0 = otGainLeft + labelY;
  auto barboxOtGain = addBarBox(
    otGainLeft0, otGainTop, barboxWidth, barboxHeight, ID::overtoneGain0, nOvertone,
    Scales::overtoneGain, "Gain");
  barboxOtGain->liveUpdateLineEdit = false;
  tabview->addWidget(tabPadSynth, barboxOtGain);

  tabview->addWidget(
    tabPadSynth,
    addScrollBar(
      otGainLeft0, otGainTop + barboxHeight, barboxWidth, scrollBarHeight, barboxOtGain));

  // Overtone Width.
  const auto otWidthTop = otGainTop + barboxY + margin;
  const auto otWidthLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth,
    addGroupVerticalLabel(
      otWidthLeft, otWidthTop, barboxHeight, labelHeight, midTextSize, "Width"));

  const auto otWidthLeft0 = otWidthLeft + labelY;
  auto barboxOtWidth = addBarBox(
    otWidthLeft0, otWidthTop, barboxWidth, barboxHeight, ID::overtoneWidth0, nOvertone,
    Scales::overtoneWidth, "Width");
  barboxOtWidth->liveUpdateLineEdit = false;
  tabview->addWidget(tabPadSynth, barboxOtWidth);

  tabview->addWidget(
    tabPadSynth,
    addScrollBar(
      otGainLeft0, otWidthTop + barboxHeight, barboxWidth, scrollBarHeight,
      barboxOtWidth));

  // Overtone Pitch.
  const auto otPitchTop = otWidthTop + barboxY + margin;
  const auto otPitchLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth,
    addGroupVerticalLabel(
      otPitchLeft, otPitchTop, barboxHeight, labelHeight, midTextSize, "Pitch"));

  const auto otPitchLeft0 = otPitchLeft + labelY;
  auto barboxOtPitch = addBarBox(
    otPitchLeft0, otPitchTop, barboxWidth, barboxHeight, ID::overtonePitch0, nOvertone,
    Scales::overtonePitch, "Pitch");
  barboxOtPitch->liveUpdateLineEdit = false;
  tabview->addWidget(tabPadSynth, barboxOtPitch);

  tabview->addWidget(
    tabPadSynth,
    addScrollBar(
      otGainLeft0, otPitchTop + barboxHeight, barboxWidth, scrollBarHeight,
      barboxOtPitch));

  // Overtone Phase.
  const auto otPhaseTop = otPitchTop + barboxY + margin;
  const auto otPhaseLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth,
    addGroupVerticalLabel(
      otPhaseLeft, otPhaseTop, barboxHeight, labelHeight, midTextSize, "Phase"));

  const auto otPhaseLeft0 = otPhaseLeft + labelY;
  auto barboxOtPhase = addBarBox(
    otPhaseLeft0, otPhaseTop, barboxWidth, barboxHeight, ID::overtonePhase0, nOvertone,
    Scales::overtonePhase, "Phase");
  barboxOtPhase->liveUpdateLineEdit = false;
  tabview->addWidget(tabPadSynth, barboxOtPhase);

  tabview->addWidget(
    tabPadSynth,
    addScrollBar(
      otGainLeft0, otPhaseTop + barboxHeight, barboxWidth, scrollBarHeight,
      barboxOtPhase));

  auto textKnobControl = R"(- Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0, 400.0f, 400.0f, infoTextSize, textKnobControl,
      150.0f));

  auto textNumberControl = R"(- Number -
Shares same controls with knob, and:
Right Click|Flip Minimum and Maximum)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0 + 80.0f, 400.0f, 400.0f, infoTextSize,
      textNumberControl, 150.0f));

  auto textOvertoneControl = R"(- Overtone & LFO Wave -
Ctrl + Left Click|Reset to Default
Right Drag|Draw Line
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert Value
Shift + I|Invert Value (Minimum to 0)
N|Normalize
Shift + N|Normalize (Minimum to 0)
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Subtle Randomize
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1|Decrease
2-9|Decrease 2n-9n)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0 + 160.0f, 400.0f, 480.0f, infoTextSize,
      textOvertoneControl, 150.0f));

  const auto tabInfoLeft1 = tabInsideLeft0 + tabWidth / 2.0f;

  auto textRefreshNotice = R"(Wavetables do not refresh automatically.
Press following button to apply changes.
- `Refresh LFO` at center-left in Main tab.
- `Refresh Table` at bottom-left in Wavetable tab.)";
  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInsideTop0, 400.0f, 400.0f, infoTextSize, textRefreshNotice));

  const auto tabInfoBottom = tabInsideTop0 + tabHeight - labelY;
  std::stringstream ssPluginName;
  ssPluginName << "\nCubicPadSynth " << VERSION_STR;
  auto pluginNameTextView = addTextView(
    tabInfoLeft1, tabInfoBottom - 160.0f, 400.0f, 400.0f, pluginNameTextSize,
    ssPluginName.str());
  tabview->addWidget(tabInfo, pluginNameTextView);

  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInfoBottom - 100.0f, 400.0f, 400.0f, infoTextSize,
      "© 2020 Takamitsu Endo (ryukau@gmail.com)\n\nHave a nice day!"));

  tabview->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
