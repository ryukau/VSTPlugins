// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

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
  const auto borderWidth = int(sc);

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // TabView test.
  const auto tabTop0 = top0;
  const auto tabLeft0 = left0;

  std::vector<std::string> tabs = {"Main", "Wavetable", "Information"};
  const auto tabWidth = defaultWidth - 2 * uiMargin;
  const auto tabHeight = labelY + 4 * barboxY + 9 * margin;
  auto tabview
    = addTabView(tabLeft0, tabTop0, tabWidth, tabHeight, uiTextSize, labelY, tabs);

  const auto tabInsideTop0 = tabTop0 + labelY + 4 * margin;
  const auto tabInsideLeft0 = tabLeft0 + 4 * margin;

  // Gain.
  const auto gainTop = tabInsideTop0;
  const auto gainLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(
    tabMain,
    addGroupLabel(gainLeft, gainTop, 6 * knobX, labelHeight, midTextSize, "Gain"));
  const auto gainKnobTop = gainTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(gainLeft, gainKnobTop, knobWidth, margin, uiTextSize, "Gain", ID::gain));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 1 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "A", ID::gainA));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 2 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "D", ID::gainD));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 3 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "S", ID::gainS));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 4 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "R", ID::gainR));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 5 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Curve",
      ID::gainCurve));

  // Filter.
  const auto filterTop = gainTop;
  const auto filterLeft = gainLeft + 6 * knobX + 2 * margin;
  tabview->addWidget(
    tabMain,
    addGroupLabel(filterLeft, filterTop, 8 * knobX, labelHeight, midTextSize, "Filter"));

  const auto filterTop0 = filterTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft, filterTop0, knobWidth, margin, uiTextSize, "Cutoff", ID::filterCutoff));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 1 * knobX, filterTop0, knobWidth, margin, uiTextSize, "Resonance",
      ID::filterResonance));

  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 2 * knobX, filterTop0, knobWidth, margin, uiTextSize, "A",
      ID::filterA));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 3 * knobX, filterTop0, knobWidth, margin, uiTextSize, "D",
      ID::filterD));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 4 * knobX, filterTop0, knobWidth, margin, uiTextSize, "S",
      ID::filterS));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 5 * knobX, filterTop0, knobWidth, margin, uiTextSize, "R",
      ID::filterR));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 6 * knobX, filterTop0, knobWidth, margin, uiTextSize, "Amount",
      ID::filterAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 7 * knobX, filterTop0, knobWidth, margin, uiTextSize, "KeyFollow",
      ID::filterKeyFollow));

  // Tuning.
  const auto tuningTop = tabInsideTop0 + labelY + knobY;
  const auto tuningLeft = gainLeft;
  tabview->addWidget(
    tabMain,
    addGroupLabel(
      tuningLeft, tuningTop, 4 * knobX - 4 * margin, labelHeight, midTextSize, "Tuning"));

  const auto tuningLabelWidth = knobX - 2 * margin;
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

  const auto tuningLeft3 = tuningLeft + 2 * knobX - 2 * margin;
  const auto tuningLeft4 = tuningLeft3 + tuningLabelWidth;

  const auto tuningTop4 = tuningTop + 1 * labelY;
  tabview->addWidget(
    tabMain,
    addLabel(tuningLeft3, tuningTop4, tuningLabelWidth, labelHeight, uiTextSize, "ET"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft4, tuningTop4, knobWidth, labelHeight, uiTextSize, ID::equalTemperament,
      Scales::equalTemperament, false, 0, 1));

  const auto tuningTop5 = tuningTop4 + labelY;
  tabview->addWidget(
    tabMain,
    addLabel(
      tuningLeft3, tuningTop5, tuningLabelWidth, labelHeight, uiTextSize, "A4 [Hz]"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft4, tuningTop5, knobWidth, labelHeight, uiTextSize, ID::pitchA4Hz,
      Scales::pitchA4Hz, false, 0, 100));

  // Unison.
  const auto unisonTop = tuningTop;
  const auto unisonLeft = tuningLeft + 4 * knobX;
  tabview->addWidget(
    tabMain,
    addGroupLabel(unisonLeft, unisonTop, 5 * knobX, labelHeight, midTextSize, "Unison"));
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
      unisonLeft + 2 * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "GainRnd",
      ID::unisonGainRandom));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 3 * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "Phase",
      ID::unisonPhase));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 4 * knobX, unisonKnobTop, knobWidth, margin, uiTextSize, "Spread",
      ID::unisonPan));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      int(unisonLeft + 1 * knobX - 1.5 * margin), unisonKnobTop + knobY,
      int(2.75 * knobWidth), labelHeight, uiTextSize, "Random Detune",
      ID::unisonDetuneRandom));

  std::vector<std::string> unisonPanTypeOptions{
    "Alternate L-R", "Alternate M-S", "Ascend L -> R", "Ascend R -> L", "High on Mid",
    "High on Side",  "Random",        "Rotate L",      "Rotate R",      "Shuffle"};
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      int(unisonLeft + 3.5 * knobX + margin), unisonKnobTop + knobY, 2 * knobWidth,
      labelHeight, uiTextSize, ID::unisonPanType, unisonPanTypeOptions));

  // Phase.
  const auto phaseTop = unisonTop;
  const auto phaseLeft = unisonLeft + 5 * knobX + 2 * margin;
  tabview->addWidget(
    tabMain,
    addGroupLabel(phaseLeft, phaseTop, 2 * knobX, labelHeight, midTextSize, "Phase"));
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
      phaseLeft1, phaseKnobTop, int(1.25 * knobWidth), labelHeight, uiTextSize, "Reset",
      ID::oscPhaseReset));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      phaseLeft1, phaseKnobTop + labelY, int(1.25 * knobWidth), labelHeight, uiTextSize,
      "Random", ID::oscPhaseRandom));

  // Misc.
  const auto miscTop = phaseTop;
  const auto miscLeft = phaseLeft + 2 * knobX + 2 * margin;
  tabview->addWidget(
    tabMain,
    addGroupLabel(
      miscLeft, miscTop, 3 * knobX - 2 * margin, labelHeight, midTextSize, "Misc."));

  const auto miscTop0 = miscTop + labelY;
  const auto miscLeft0 = miscLeft;
  tabview->addWidget(
    tabMain,
    addKnob(
      miscLeft0 + margin, miscTop0, knobWidth, margin, uiTextSize, "Smooth",
      ID::smoothness));

  const auto nVoiceLeft = miscLeft0 + knobX + margin;
  std::vector<std::string> nVoiceOptions
    = {"16 Voices", "32 Voices", "48 Voices",  "64 Voices",
       "80 Voices", "96 Voices", "112 Voices", "128 Voices"};
  tabview->addWidget(
    tabMain, addLabel(nVoiceLeft, miscTop0, 8 * margin, labelHeight, uiTextSize, "Poly"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      nVoiceLeft + 8 * margin, miscTop0, knobX, labelHeight, uiTextSize, ID::nVoice,
      nVoiceOptions));

  tabview->addWidget(
    tabMain,
    addLabel(nVoiceLeft, miscTop0 + labelY, 8 * margin, labelHeight, uiTextSize, "Seed"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      nVoiceLeft + 8 * margin, miscTop0 + labelY, knobX, labelHeight, uiTextSize,
      ID::seed, Scales::seed));

  // Delay.
  const auto delayTop = unisonTop + 2 * labelY + knobY;
  const auto delayLeft = gainLeft;
  tabview->addWidget(
    tabMain,
    addGroupLabel(delayLeft, delayTop, 5 * knobX, labelHeight, midTextSize, "Delay"));
  const auto delayKnobTop = delayTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(delayLeft, delayKnobTop, knobWidth, margin, uiTextSize, "Mix", ID::delayMix));
  tabview->addWidget(
    tabMain,
    addKnob(
      delayLeft + 1 * knobX, delayKnobTop, knobWidth, margin, uiTextSize, "Feedback",
      ID::delayFeedback));
  tabview->addWidget(
    tabMain,
    addKnob(
      delayLeft + 2 * knobX, delayKnobTop, knobWidth, margin, uiTextSize, "Attack",
      ID::delayAttack));

  const auto delayDetuneLeft = delayLeft + 3 * knobX;
  tabview->addWidget(
    tabMain,
    addLabel(
      delayDetuneLeft, delayKnobTop, tuningLabelWidth, labelHeight, uiTextSize, "Semi"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      delayDetuneLeft + knobX, delayKnobTop, knobWidth, labelHeight, uiTextSize,
      ID::delayDetuneSemi, Scales::delayDetuneSemi, false, 0, -120));

  tabview->addWidget(
    tabMain,
    addLabel(
      delayDetuneLeft, delayKnobTop + labelY, tuningLabelWidth, labelHeight, uiTextSize,
      "Milli"));
  auto knobDelayMilli = addTextKnob(
    delayDetuneLeft + knobX, delayKnobTop + labelY, knobWidth, labelHeight, uiTextSize,
    ID::delayDetuneMilli, Scales::oscMilli, false, 0, -1000);
  knobDelayMilli->sensitivity = 0.001f;
  knobDelayMilli->lowSensitivity = 0.00025f;
  tabview->addWidget(tabMain, knobDelayMilli);

  // LFO.
  const auto lfoTop = delayTop;
  const auto lfoLeft = delayLeft + 5 * knobX + 2 * margin;
  tabview->addWidget(
    tabMain, addGroupLabel(lfoLeft, lfoTop, 9 * knobX, labelHeight, midTextSize, "LFO"));
  const auto lfoKnobTop = lfoTop + labelY;

  const auto lfoLeft1 = lfoLeft;
  const auto lfoTempoTop = lfoKnobTop;
  auto knobLfoTempoNumerator = addTextKnob(
    lfoLeft1, lfoTempoTop, knobWidth, labelHeight, uiTextSize, ID::lfoTempoNumerator,
    Scales::lfoTempoNumerator, false, 0, 1);
  knobLfoTempoNumerator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoNumerator);

  auto knobLfoTempoDenominator = addTextKnob(
    lfoLeft1, lfoTempoTop + labelHeight + borderWidth, knobWidth, labelHeight, uiTextSize,
    ID::lfoTempoDenominator, Scales::lfoTempoDenominator, false, 0, 1);
  knobLfoTempoDenominator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoDenominator);

  tabview->addWidget(
    tabMain,
    addLabel(
      lfoLeft1, lfoTempoTop + labelHeight + labelY - margin, knobWidth, labelHeight,
      uiTextSize, "Tempo"));

  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 1 * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Multiply",
      ID::lfoFrequencyMultiplier));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 2 * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Amount",
      ID::lfoDelayAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 3 * knobX, lfoKnobTop, knobWidth, margin, uiTextSize, "Lowpass",
      ID::lfoLowpass));

  const auto lfoLeft2 = lfoLeft + 5 * knobX;

  std::vector<std::string> lfoWavetableTypeOptions{"Step", "Linear", "Cubic"};
  tabview->addWidget(
    tabMain,
    addLabel(
      lfoLeft2 + int(0.375 * knobX), lfoKnobTop, int(knobWidth * 1.5), labelHeight,
      uiTextSize, "Interpolation"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      lfoLeft2 + int(0.375 * knobX), lfoKnobTop + labelY, int(knobWidth * 1.5),
      labelHeight, uiTextSize, ID::lfoWavetableType, lfoWavetableTypeOptions));

  tabview->addWidget(
    tabMain,
    addKickButton<Style::warning>(
      lfoLeft2 + 2 * knobX, lfoKnobTop + 2 * margin, 2 * knobX, 2 * labelHeight,
      midTextSize, "Refresh LFO", ID::refreshLFO));

  // LFO wavetable.
  const auto lfoWaveTop = int(lfoKnobTop + knobY + labelY / 2);
  const auto lfoWaveLeft = tabInsideLeft0;
  const auto lfoBarboxHeight = int(barboxHeight + 3.5 * labelY + 3 * margin);
  tabview->addWidget(
    tabMain,
    addGroupVerticalLabel(
      lfoWaveLeft, lfoWaveTop, lfoBarboxHeight, labelHeight, midTextSize, "LFO Wave"));
  // 832 = 64 * 13.
  auto barboxLfoWavetable = addBarBox(
    lfoWaveLeft + labelY, lfoWaveTop, int(sc * 832), lfoBarboxHeight, ID::lfoWavetable0,
    nLFOWavetable, Scales::lfoWavetable, "LFO Wave");
  barboxLfoWavetable->sliderZero = 0.5;
  tabview->addWidget(tabMain, barboxLfoWavetable);

  // Wavetable pitch.
  const auto tablePitchTop = tabInsideTop0;
  const auto tablePitchLeft0 = tabInsideLeft0;
  const auto tablePitchLeft1 = tablePitchLeft0 + knobX;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tablePitchLeft0, tablePitchTop, 2 * knobX, labelHeight, midTextSize, "Pitch"));

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
      tablePitchLeft0, tablePitchTop + 2 * labelY, knobX, labelHeight, uiTextSize,
      "Multiply"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 2 * labelY, knobX, labelHeight, uiTextSize,
      ID::overtonePitchMultiply, Scales::overtonePitchMultiply, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tablePitchLeft0, tablePitchTop + 3 * labelY, knobX, labelHeight, uiTextSize,
      "Modulo"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 3 * labelY, knobX, labelHeight, uiTextSize,
      ID::overtonePitchModulo, Scales::overtonePitchModulo, false, 4));

  const auto tableSpectrumTop = tablePitchTop + 4 * labelY;
  const auto tableSpectrumLeft0 = tablePitchLeft0;
  const auto tableSpectrumLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableSpectrumLeft0, tableSpectrumTop, 2 * knobX, labelHeight, midTextSize,
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
      tableSpectrumLeft0, tableSpectrumTop + 2 * labelY, knobX, labelHeight, uiTextSize,
      "Rotate"));
  auto knobSpectrumRotate = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 2 * labelY, knobX, labelHeight, uiTextSize,
    ID::spectrumRotate, Scales::defaultScale, false, 6);
  knobSpectrumRotate->lowSensitivity = 1.0 / oscSpectrumSize;
  tabview->addWidget(tabPadSynth, knobSpectrumRotate);

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + 3 * labelY, knobX, labelHeight, uiTextSize,
      "Comb"));
  auto knobProfileComb = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 3 * labelY, knobX, labelHeight, uiTextSize,
    ID::profileComb, Scales::profileComb);
  knobProfileComb->sensitivity = 0.002;
  knobProfileComb->lowSensitivity = 0.0002;
  tabview->addWidget(tabPadSynth, knobProfileComb);

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableSpectrumLeft0, tableSpectrumTop + 4 * labelY, knobX, labelHeight, uiTextSize,
      "Shape"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 4 * labelY, knobX, labelHeight, uiTextSize,
      ID::profileShape, Scales::profileShape, false, 4, 0));

  const auto tablePhaseTop = tableSpectrumTop + 5 * labelY;
  const auto tablePhaseLeft0 = tablePitchLeft0;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tablePhaseLeft0, tablePhaseTop, 2 * knobX, labelHeight, midTextSize, "Phase"));
  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePhaseLeft0, tablePhaseTop + labelY, 2 * checkboxWidth, labelHeight, uiTextSize,
      "UniformPhase", ID::uniformPhaseProfile));

  // Wavetable random.
  const auto tableRandomTop = tablePhaseTop + 2 * labelY;
  const auto tableRandomLeft0 = tablePitchLeft0;
  const auto tableRandomLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableRandomLeft0, tableRandomTop, 2 * knobX, labelHeight, midTextSize, "Random"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(
      tableRandomLeft0, tableRandomTop + labelY, knobX, labelHeight, uiTextSize, "Seed"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableRandomLeft1, tableRandomTop + labelY, knobX, labelHeight, uiTextSize,
      ID::padSynthSeed, Scales::seed));

  // Wavetable buffer size.
  const auto tableBufferTop = tableRandomTop + 2 * labelY;
  const auto tableBufferLeft0 = tablePitchLeft0;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableBufferLeft0, tableBufferTop, 2 * knobX, labelHeight, midTextSize,
      "BufferSize"));

  std::vector<std::string> bufferSizeItems{
    "2^10", "2^11", "2^12",           "2^13",           "2^14",           "2^15",
    "2^16", "2^17", "2^18 (128 MiB)", "2^19 (256 MiB)", "2^20 (512 MiB)", "2^21 (1 GiB)"};
  tabview->addWidget(
    tabPadSynth,
    addOptionMenu(
      tableRandomLeft0, tableBufferTop + labelY, 2 * knobX, labelHeight, uiTextSize,
      ID::tableBufferSize, bufferSizeItems));

  // Wavetable modifier.
  const auto tableModifierTop = tableBufferTop + 2 * labelY;
  const auto tableModifierLeft0 = tablePitchLeft0;
  const auto tableModifierLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(
      tableModifierLeft0, tableModifierTop, 2 * knobX, labelHeight, midTextSize,
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
  const auto refreshTop = tabTop0 + tabHeight - 2 * labelY;
  const auto refreshLeft = tabInsideLeft0;
  tabview->addWidget(
    tabPadSynth,
    addKickButton<Style::warning>(
      refreshLeft, refreshTop, 2 * knobX, 2 * labelHeight, midTextSize, "Refresh Table",
      ID::refreshTable));

  // Overtone Gain.
  const auto otGainTop = tabInsideTop0;
  const auto otGainLeft = tabInsideLeft0 + 2 * knobX + 4 * margin;
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
      otGainLeft0, otGainTop + barboxHeight - borderWidth, barboxWidth, scrollBarHeight,
      barboxOtGain));

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
      otGainLeft0, otWidthTop + barboxHeight - borderWidth, barboxWidth, scrollBarHeight,
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
      otGainLeft0, otPitchTop + barboxHeight - borderWidth, barboxWidth, scrollBarHeight,
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
      otGainLeft0, otPhaseTop + barboxHeight - borderWidth, barboxWidth, scrollBarHeight,
      barboxOtPhase));

  const auto infoTextWidth = int(sc * 400);
  const auto infoTextCellWidth = int(sc * 150);
  auto textOvertoneControl = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Shift + Left Drag|Skip Between Frames
Middle Drag|Draw Line
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert
Shift + I|Full Invert
N|Normalize (Preserve Min)
Shift + N|Normalize
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Random Walk
Shift + T|Random Walk to 0
Z|Undo
Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1-4|Decrease 1n-4n
5-9|Hold 2n-5n)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0, infoTextWidth, defaultHeight - 40, infoTextSize,
      textOvertoneControl, infoTextCellWidth));

  const auto tabInfoLeft1 = tabInsideLeft0 + tabWidth / 2.0f;

  auto textKnobControl = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInfoLeft1, tabInsideTop0, infoTextWidth, infoTextWidth, infoTextSize,
      textKnobControl, infoTextCellWidth));

  auto textRefreshNotice = R"(Wavetables do not refresh automatically.
Press following button to apply changes.
- `Refresh LFO` at center-left in Main tab.
- `Refresh Table` at bottom-left in Wavetable tab.)";
  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInsideTop0 + 8 * uiMargin, infoTextWidth, infoTextWidth,
      infoTextSize, textRefreshNotice));

  const auto tabInfoBottom = tabInsideTop0 + tabHeight - labelY;
  std::stringstream ssPluginName;
  ssPluginName << "LightPadSynth " << VERSION_STR;
  auto pluginNameTextView = addTextView(
    tabInfoLeft1, tabInfoBottom - 7 * uiMargin, infoTextWidth, infoTextWidth,
    pluginNameTextSize, ssPluginName.str());
  tabview->addWidget(tabInfo, pluginNameTextView);

  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInfoBottom - 5 * uiMargin, infoTextWidth, infoTextWidth,
      infoTextSize, "© 2020 Takamitsu Endo (ryukau@gmail.com)\n\nHave a nice day!"));

  tabview->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
