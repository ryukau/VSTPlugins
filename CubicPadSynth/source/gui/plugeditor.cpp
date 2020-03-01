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

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "../version.hpp"
#include "plugeditor.hpp"
#include "x11runloop.hpp"

#include "barbox.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "slider.hpp"
#include "tabview.hpp"
#include "textbutton.hpp"
#include "textview.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller) { setRect(viewRect); }

bool PlugEditor::open(void *parent, const PlatformType &platformType)
{
  if (frame) return false;

  setIdleRate(1000 / 60);

  frame = new CFrame(
    CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
  if (frame == nullptr) return false;
  frame->setBackgroundColor(colorWhite);
  frame->registerMouseObserver(this);

  IPlatformFrameConfig *config = nullptr;
#if LINUX
  X11::FrameConfig x11config;
  x11config.runLoop = VSTGUI::owned(new RunLoop(plugFrame));
  config = &x11config;
#endif
  frame->open(parent, platformType, config);

  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // TabView test.
  const auto tabTop0 = top0;
  const auto tabLeft0 = left0;

  std::vector<std::string> tabs = {"Main", "Wavetable", "Information"};
  const auto tabWidth = defaultWidth - 40.0f;
  const auto tabHeight = labelY + 4.0f * barboxY + 9.0f * margin;
  auto tabview = new TabView(
    tabs, new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace),
    labelY, CRect(tabLeft0, tabTop0, tabLeft0 + tabWidth, tabTop0 + tabHeight));
  tabview->setHighlightColor(colorBlue);
  frame->addView(tabview);

  const auto tabInsideTop0 = tabTop0 + labelY + 4.0f * margin;
  const auto tabInsideLeft0 = tabLeft0 + 4.0f * margin;
  const auto tabCenterX = tabInsideLeft0 + labelY + 7.0f * knobX + 4.0f * margin;

  // Tuning.
  const auto tuningTop = tabInsideTop0;
  const auto tuningLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(
    tabMain, addGroupLabel(tuningLeft, tuningTop, 2.0f * knobX - 4.0 * margin, "Tuning"));

  const auto tuningLabelWidth = knobX - 2.0f * margin;
  const auto tuningLeft1 = tuningLeft;
  const auto tuningLeft2 = tuningLeft1 + tuningLabelWidth;

  const auto tuningTop1 = tuningTop + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop1, tuningLabelWidth, "Octave"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop1, knobWidth, colorBlue, ID::oscOctave, Scales::oscOctave,
      false, 0, -12));

  const auto tuningTop2 = tuningTop1 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop2, tuningLabelWidth, "Semi"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop2, knobWidth, colorBlue, ID::oscSemi, Scales::oscSemi, false,
      0, -120));

  const auto tuningTop3 = tuningTop2 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop3, tuningLabelWidth, "Milli"));
  auto knobOscMilli = addTextKnob(
    tuningLeft2, tuningTop3, knobWidth, colorBlue, ID::oscMilli, Scales::oscMilli, false,
    0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;
  tabview->addWidget(tabMain, knobOscMilli);

  const auto tuningTop4 = tuningTop3 + 1.5f * labelY;
  tabview->addWidget(tabMain, addLabel(tuningLeft1, tuningTop4, tuningLabelWidth, "ET"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop4, knobWidth, colorBlue, ID::equalTemperament,
      Scales::equalTemperament, false, 0, 1));

  const auto tuningTop5 = tuningTop4 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop5, tuningLabelWidth, "A4 [Hz]"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop5, knobWidth, colorBlue, ID::pitchA4Hz, Scales::pitchA4Hz,
      false, 0, 100));

  const auto tuningOffsetX = 2.0f * knobX;

  // Gain.
  const auto gainTop = tabInsideTop0;
  const auto gainLeft = tabInsideLeft0 + labelY + tuningOffsetX;
  tabview->addWidget(tabMain, addGroupLabel(gainLeft, gainTop, 5.0f * knobX, "Gain"));
  const auto gainKnobTop = gainTop + labelY;

  tabview->addWidget(
    tabMain, addKnob(gainLeft, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 1.0f * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 2.0f * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 3.0f * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 4.0f * knobX, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain));

  // Lowpass.
  const auto filterTop = tabInsideTop0;
  const auto filterLeft = tabCenterX;
  tabview->addWidget(
    tabMain, addGroupLabel(filterLeft, filterTop, 7.0f * knobX, "Lowpass"));
  const auto filterKnobTop = filterTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(filterLeft, filterKnobTop, knobWidth, colorBlue, "Cutoff", ID::tableLowpass));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + knobX, filterKnobTop, knobWidth, colorBlue, "KeyFollow",
      ID::tableLowpassKeyFollow));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 2.0f * knobX, filterKnobTop, knobWidth, colorBlue, "A",
      ID::tableLowpassA));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 3.0f * knobX, filterKnobTop, knobWidth, colorBlue, "D",
      ID::tableLowpassD));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 4.0f * knobX, filterKnobTop, knobWidth, colorBlue, "S",
      ID::tableLowpassS));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 5.0f * knobX, filterKnobTop, knobWidth, colorBlue, "R",
      ID::tableLowpassR));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 6.0f * knobX, filterKnobTop, knobWidth, colorBlue, "Amount",
      ID::tableLowpassEnvelopeAmount));

  // Pitch.
  const auto pitchTop = filterTop + labelY + knobY;
  const auto pitchLeft = tabInsideLeft0 + labelY + tuningOffsetX;
  tabview->addWidget(tabMain, addGroupLabel(pitchLeft, pitchTop, 5.0f * knobX, "Pitch"));

  const auto pitchKnobTop = pitchTop + labelY;
  tabview->addWidget(
    tabMain, addKnob(pitchLeft, pitchKnobTop, knobWidth, colorBlue, "A", ID::pitchA));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 1.0f * knobX, pitchKnobTop, knobWidth, colorBlue, "D", ID::pitchD));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 2.0f * knobX, pitchKnobTop, knobWidth, colorBlue, "S", ID::pitchS));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 3.0f * knobX, pitchKnobTop, knobWidth, colorBlue, "R", ID::pitchR));
  tabview->addWidget(
    tabMain,
    addKnob(
      pitchLeft + 4.0f * knobX, pitchKnobTop, knobWidth, colorBlue, "Amount",
      ID::pitchEnvelopeAmount));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      pitchLeft + 4.0f * knobX - 1.5f * margin, pitchKnobTop + knobY, knobWidth,
      "Negative", ID::pitchEnvelopeAmountNegative));

  // Unison.
  const auto unisonTop = pitchTop;
  const auto unisonLeft = tabCenterX;
  tabview->addWidget(
    tabMain, addGroupLabel(unisonLeft, unisonTop, 7.0f * knobX, "Unison"));
  const auto unisonKnobTop = unisonTop + labelY;
  tabview->addWidget(tabMain, addLabel(unisonLeft, unisonKnobTop, knobWidth, "nUnison"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      unisonLeft, unisonKnobTop + labelHeight + margin, knobWidth, colorBlue, ID::nUnison,
      Scales::nUnison, false, 0, 1));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + knobX, unisonKnobTop, knobWidth, colorBlue, "Detune",
      ID::unisonDetune));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 2.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "GainRnd",
      ID::unisonGainRandom));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 3.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Phase",
      ID::unisonPhase));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 4.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Spread",
      ID::unisonPan));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      unisonLeft + knobX - 1.5f * margin, unisonKnobTop + knobY, knobWidth,
      "Random Detune", ID::unisonDetuneRandom));

  std::vector<UTF8String> unisonPanTypeOptions{
    "Alternate L-R", "Alternate M-S", "Ascend L -> R", "Ascend R -> L", "High on Mid",
    "High on Side",  "Random",        "Rotate L",      "Rotate R",      "Shuffle"};
  tabview->addWidget(
    tabMain,
    addLabel(unisonLeft + 5.0f * knobX, unisonKnobTop, 2.0f * knobWidth, "Spread Type"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      unisonLeft + 5.0f * knobX, unisonKnobTop + labelY, 2.0f * knobWidth,
      ID::unisonPanType, unisonPanTypeOptions));

  // LFO.
  const auto lfoTop = unisonTop + 2.0f * labelY + knobY;
  const auto lfoLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(tabMain, addGroupLabel(lfoLeft, lfoTop, 10.0f * knobX, "LFO"));
  const auto lfoKnobTop = lfoTop + labelY;

  tabview->addWidget(
    tabMain,
    addStateButton(
      lfoLeft, lfoKnobTop + 2.0f * margin, 2.0f * knobX, "Refresh LFO", "lfo", "N/A"));

  std::vector<UTF8String> lfoWavetableTypeOptions{"Step", "Linear", "Cubic"};
  tabview->addWidget(
    tabMain,
    addLabel(lfoLeft + 2.5f * knobX, lfoKnobTop, 1.55f * knobWidth, "Interpolation"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      lfoLeft + 2.5f * knobX, lfoKnobTop + labelY, 1.55f * knobWidth,
      ID::lfoWavetableType, lfoWavetableTypeOptions));

  const auto lfoLeft1 = lfoLeft + 4.0f * knobX;
  tabview->addWidget(
    tabMain,
    addLabel(
      lfoLeft1 + margin, lfoKnobTop + margin + 0.5f * labelHeight, knobWidth, "Tempo"));

  auto knobLfoTempoNumerator = addTextKnob(
    lfoLeft1 + knobX, lfoKnobTop + margin, knobWidth, colorBlue, ID::lfoTempoNumerator,
    Scales::lfoTempoNumerator, false, 0, 1);
  knobLfoTempoNumerator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoNumerator);

  auto knobLfoTempoDenominator = addTextKnob(
    lfoLeft1 + knobX, lfoKnobTop + labelHeight + 1.0f + margin, knobWidth, colorBlue,
    ID::lfoTempoDenominator, Scales::lfoTempoDenominator, false, 0, 1);
  knobLfoTempoDenominator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoDenominator);

  tabview->addWidget(
    tabMain,
    addCheckbox(
      lfoLeft1 + 0.5f * knobWidth, lfoKnobTop + labelHeight + labelY, knobX, "Retrigger",
      ID::lfoPhaseReset));

  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 2.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Multiply",
      ID::lfoFrequencyMultiplier));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 3.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Amount",
      ID::lfoPitchAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 4.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Lowpass",
      ID::lfoLowpass));

  // Phase.
  const auto phaseTop = lfoTop;
  const auto phaseLeft = lfoLeft + 10.0f * knobX + 2.0f * margin;
  tabview->addWidget(tabMain, addGroupLabel(phaseLeft, phaseTop, 2.0f * knobX, "Phase"));
  const auto phaseKnobTop = phaseTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(phaseLeft, phaseKnobTop, knobWidth, colorBlue, "Phase", ID::oscInitialPhase));

  const auto phaseLeft1 = phaseLeft + knobX;
  tabview->addWidget(
    tabMain,
    addCheckbox(phaseLeft1, phaseKnobTop, knobWidth, "Reset", ID::oscPhaseReset));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      phaseLeft1, phaseKnobTop + labelY, knobWidth, "Random", ID::oscPhaseRandom));

  // Misc.
  const auto miscTop = phaseTop;
  const auto miscLeft = phaseLeft + 2.0f * knobX + 2.0f * margin;
  tabview->addWidget(tabMain, addGroupLabel(miscLeft, miscTop, 2.0f * knobX, "Misc."));

  const auto miscTop0 = miscTop + labelY;
  tabview->addWidget(
    tabMain, addKnob(miscLeft, miscTop0, knobWidth, colorBlue, "Smooth", ID::smoothness));

  const auto miscLeft0 = miscLeft + knobX - (checkboxWidth - knobWidth) / 2.0f;
  std::vector<UTF8String> nVoiceOptions
    = {"16 Voices", "32 Voices", "48 Voices",  "64 Voices",
       "80 Voices", "96 Voices", "112 Voices", "128 Voices"};
  tabview->addWidget(
    tabMain,
    addOptionMenu(miscLeft0, miscTop0, checkboxWidth, ID::nVoice, nVoiceOptions));
  tabview->addWidget(
    tabMain, addCheckbox(miscLeft0, miscTop0 + labelY, knobWidth, "Pool", ID::voicePool));

  // LFO wavetable.
  const auto lfoWaveTop = lfoKnobTop + knobY + 0.5f * labelY;
  const auto lfoWaveLeft = tabInsideLeft0;
  const auto lfoBarboxHeight = barboxHeight + 3.5f * labelY + 3.0f * margin;
  tabview->addWidget(
    tabMain, addGroupVerticalLabel(lfoWaveLeft, lfoWaveTop, lfoBarboxHeight, "LFO Wave"));
  auto barboxLfoWavetable = addBarBox(
    lfoWaveLeft + labelY, lfoWaveTop, barboxWidth + 2.0f * knobX + 4.0f * margin,
    lfoBarboxHeight, ID::lfoWavetable0, nLFOWavetable, "LFO Wave");
  barboxLfoWavetable->drawCenterLine = true;
  tabview->addWidget(tabMain, barboxLfoWavetable);

  // Wavetable pitch.
  const auto tablePitchTop = tabInsideTop0;
  const auto tablePitchLeft0 = tabInsideLeft0;
  const auto tablePitchLeft1 = tablePitchLeft0 + knobX;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tablePitchLeft0, tablePitchTop, 2.0f * knobX, "Pitch"));

  tabview->addWidget(
    tabPadSynth, addLabel(tablePitchLeft0, tablePitchTop + labelY, knobX, "Base Freq."));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + labelY, knobX, colorBlue, ID::tableBaseFrequency,
      Scales::tableBaseFrequency, false, 2));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tablePitchLeft0, tablePitchTop + 2.0f * labelY, knobX, "Multiply"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 2.0f * labelY, knobX, colorBlue,
      ID::overtonePitchMultiply, Scales::overtonePitchMultiply, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tablePitchLeft0, tablePitchTop + 3.0f * labelY, knobX, "Modulo"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 3.0f * labelY, knobX, colorBlue,
      ID::overtonePitchModulo, Scales::overtonePitchModulo, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePitchLeft0, tablePitchTop + 4.0f * labelY, checkboxWidth, "Random",
      ID::overtonePitchRandom));

  // WaveTable spectrum.
  const auto tableSpectrumTop = tablePitchTop + 5.0f * labelY;
  const auto tableSpectrumLeft0 = tablePitchLeft0;
  const auto tableSpectrumLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(tableSpectrumLeft0, tableSpectrumTop, 2.0f * knobX, "Spectrum"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + labelY, knobX, "Expand"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + labelY, knobX, colorBlue, ID::spectrumExpand,
      Scales::spectrumExpand, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 2.0f * labelY, knobX, "Shift"));
  auto knobSpectrumShift = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 2.0f * labelY, knobX, colorBlue,
    ID::spectrumShift, Scales::spectrumShift, false, 0, -spectrumSize);
  knobSpectrumShift->lowSensitivity = 0.08f / spectrumSize;
  tabview->addWidget(tabPadSynth, knobSpectrumShift);

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 3.0 * labelY, knobX, "Comb"));
  auto knobProfileComb = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 3.0 * labelY, knobX, colorBlue,
    ID::profileComb, Scales::profileComb);
  knobProfileComb->sensitivity = 0.002;
  knobProfileComb->lowSensitivity = 0.0002;
  tabview->addWidget(tabPadSynth, knobProfileComb);

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 4.0 * labelY, knobX, "Shape"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 4.0 * labelY, knobX, colorBlue,
      ID::profileShape, Scales::profileShape, false, 4, 0));

  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tableSpectrumLeft0, tableSpectrumTop + 5.0f * labelY, checkboxWidth, "Invert",
      ID::spectrumInvert));

  // WaveTable phase.
  const auto tablePhaseTop = tableSpectrumTop + 6.0f * labelY;
  const auto tablePhaseLeft0 = tablePitchLeft0;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tablePhaseLeft0, tablePhaseTop, 2.0f * knobX, "Phase"));
  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePhaseLeft0, tablePhaseTop + labelY, checkboxWidth, "UniformPhase",
      ID::uniformPhaseProfile));

  // Wavetable random.
  const auto tableRandomTop = tablePhaseTop + 2.0f * labelY;
  const auto tableRandomLeft0 = tablePitchLeft0;
  const auto tableRandomLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tableRandomLeft0, tableRandomTop, 2.0f * knobX, "Random"));

  tabview->addWidget(
    tabPadSynth, addLabel(tableRandomLeft0, tableRandomTop + labelY, knobX, "Seed"));

  auto knobSeed = addTextKnob(
    tableRandomLeft1, tableRandomTop + labelY, knobX, colorBlue, ID::padSynthSeed,
    Scales::seed);
  knobSeed->lowSensitivity = 0.1f / Scales::seed.getMax();
  tabview->addWidget(tabPadSynth, knobSeed);

  // Wavetable modifier.
  const auto tableModifierTop = tableRandomTop + 2.0f * labelY;
  const auto tableModifierLeft0 = tablePitchLeft0;
  const auto tableModifierLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(tableModifierLeft0, tableModifierTop, 2.0f * knobX, "Modifier"));

  const auto tableModifierTop0 = tableModifierTop + labelY;
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft0, tableModifierTop0, knobWidth, colorBlue, "Gain^",
      ID::overtoneGainPower));
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft1, tableModifierTop0, knobWidth, colorBlue, "Width*",
      ID::overtoneWidthMultiply));

  // Refresh button.
  const auto refreshTop = tabTop0 + tabHeight - 2.0f * labelY;
  const auto refreshLeft = tabInsideLeft0;
  tabview->addWidget(
    tabPadSynth,
    addStateButton(
      refreshLeft, refreshTop, 2.0f * knobX, "Refresh Table", "padsynth", "N/A"));

  // Overtone Gain.
  const auto otGainTop = tabInsideTop0;
  const auto otGainLeft = tabInsideLeft0 + 2.0f * knobX + 4.0f * margin;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otGainLeft, otGainTop, barboxHeight, "Gain"));

  const auto otGainLeft0 = otGainLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otGainLeft0, otGainTop, barboxWidth, barboxHeight, ID::overtoneGain0, nOvertone,
      "Gain"));

  // Overtone Width.
  const auto otWidthTop = otGainTop + barboxY + margin;
  const auto otWidthLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otWidthLeft, otWidthTop, barboxHeight, "Width"));

  const auto otWidthLeft0 = otWidthLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otWidthLeft0, otWidthTop, barboxWidth, barboxHeight, ID::overtoneWidth0, nOvertone,
      "Width"));

  // Overtone Pitch.
  const auto otPitchTop = otWidthTop + barboxY + margin;
  const auto otPitchLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otPitchLeft, otPitchTop, barboxHeight, "Pitch"));

  const auto otPitchLeft0 = otPitchLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otPitchLeft0, otPitchTop, barboxWidth, barboxHeight, ID::overtonePitch0, nOvertone,
      "Pitch"));

  // Overtone Phase.
  const auto otPhaseTop = otPitchTop + barboxY + margin;
  const auto otPhaseLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otPhaseLeft, otPhaseTop, barboxHeight, "Phase"));

  const auto otPhaseLeft0 = otPhaseLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otPhaseLeft0, otPhaseTop, barboxWidth, barboxHeight, ID::overtonePhase0, nOvertone,
      "Phase"));

  auto textKnobControl = R"(- Knob -
Shift + Left Drag|Fine Tuning
Ctrl + Left Click|Reset to Default)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0, 400.0f, 400.0f, textKnobControl, 150.0f, fontSize));

  auto textNumberControl = R"(- Number -
Shares same controls with knob, and:
Right Click|Flip Minimum and Maximum)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0 + 80.0f, 400.0f, 400.0f, textNumberControl, 150.0f,
      fontSize));

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
      tabInsideLeft0, tabInsideTop0 + 160.0f, 400.0f, 600.0f, textOvertoneControl, 150.0f,
      fontSize));

  const auto tabInfoLeft1 = tabInsideLeft0 + tabWidth / 2.0f;

  auto textRefreshNotice = R"(Wavetables won't refresh automatically.
Press following button to apply changes.
- `Refresh LFO` at center-left in Main tab.
- `Refresh Table` at bottom-left in WaveTable tab.)";
  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInsideTop0, 400.0f, 200.0f, textRefreshNotice, fontSize));

  const auto tabInfoBottom = tabInsideTop0 + tabHeight - labelY;
  std::stringstream ssPluginName;
  ssPluginName << "\nCubicPadSynth " << VERSION_STR;
  auto pluginNameTextView = addTextView(
    tabInfoLeft1, tabInfoBottom - 160.0f, 400.0f, 400.0f, ssPluginName.str(), 36.0f);
  tabview->addWidget(tabInfo, pluginNameTextView);

  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInfoBottom - 100.0f, 400.0f, 400.0f,
      "© 2020 Takamitsu Endo (ryukau@gmail.com)\n\nHave a nice day!", fontSize));

  tabview->refreshTab();

  return true;
}

void PlugEditor::close()
{
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

void PlugEditor::valueChanged(ParamID id, ParamValue normalized)
{
  controller->setParamNormalized(id, normalized);
  controller->performEdit(id, normalized);
}

void PlugEditor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter != controlMap.end()) {
    iter->second->setValueNormalized(normalized);
    iter->second->invalid();
    return;
  }

  using ID = Synth::ParameterID::ID;

  for (auto &ctrl : arrayControls) {
    if (id < ctrl->id.front() && id > ctrl->id.back()) continue;
    ctrl->setValueAt(id - ctrl->id.front(), normalized);
    ctrl->invalid();
  }
}

CMouseEventResult
PlugEditor::onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isRightButton()) return kMouseEventNotHandled;

  auto componentHandler = controller->getComponentHandler();
  if (componentHandler == nullptr) return kMouseEventNotHandled;

  FUnknownPtr<IComponentHandler3> handler(componentHandler);
  if (handler == nullptr) return kMouseEventNotHandled;

  auto control = dynamic_cast<CControl *>(frame->getViewAt(where));
  if (control == nullptr) return kMouseEventNotHandled;

  // Context menu will not popup when the control has negative tag.
  ParamID id = control->getTag();
  if (id < 1 || id >= LONG_MAX) return kMouseEventNotHandled;

  IContextMenu *menu = handler->createContextMenu(this, &id);
  if (menu == nullptr) return kMouseEventNotHandled;

  menu->popup(where.x, where.y);
  menu->release();
  return kMouseEventHandled;
}

CMouseEventResult
PlugEditor::onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  return kMouseEventNotHandled;
}

BarBox *PlugEditor::addBarBox(
  CCoord left,
  CCoord top,
  CCoord width,
  CCoord height,
  ParamID id0,
  size_t nBar,
  std::string name)
{
  std::vector<ParamID> id(nBar);
  for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);
  std::vector<double> value(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    value[i] = controller->getParamNormalized(id[i]);
  std::vector<double> defaultValue(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    defaultValue[i] = param.value[id[i]]->getDefaultNormalized();

  auto barBox = new BarBox(
    CRect(left, top, left + width, top + height), this, id, value, defaultValue);
  barBox->setIndexFont(
    new CFontDesc(PlugEditorStyle::fontName(), 10.0, CTxtFace::kBoldFace));
  barBox->setNameFont(
    new CFontDesc(PlugEditorStyle::fontName(), 24.0, CTxtFace::kNormalFace));
  barBox->setBorderColor(colorBlack);
  barBox->setValueColor(colorBlue);
  barBox->setName(name);
  frame->addView(barBox);

  auto iter = std::find_if(
    arrayControls.begin(), arrayControls.end(),
    [&](const ArrayControl *elem) { return elem->id[0] == id0; });
  if (iter != arrayControls.end()) {
    (*iter)->forget();
    arrayControls.erase(iter);
  }
  barBox->remember();
  arrayControls.push_back(barBox);
  return barBox;
}

CTextLabel *PlugEditor::addLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font)
{
  auto bottom = top + 20.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  if (font == nullptr)
    font = new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace);
  label->setFont(font);
  label->setHoriAlign(CHoriTxtAlign::kCenterText);
  label->setStyle(CTextLabel::kNoFrame);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
  return label;
}

GroupLabel *
PlugEditor::addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + labelHeight;

  auto label
    = new GroupLabel(CRect(left, top, left + width, bottom), this, UTF8String(name));
  label->setFont(new CFontDesc(PlugEditorStyle::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
  return label;
}

VGroupLabel *
PlugEditor::addGroupVerticalLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  return nullptr;

  // VSTGUI 4.9 can't draw roteted text.
  /*
  auto label = new VGroupLabel(
    CRect(left, top, left + labelHeight, top + width), this, UTF8String(name));
  label->setFont(new CFontDesc(PlugEditorStyle::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
  return label;
  */
};

std::tuple<Slider *, CTextLabel *> PlugEditor::addVSlider(
  CCoord left,
  CCoord top,
  CColor valueColor,
  UTF8String name,
  ParamID tag,
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  auto right = left + 70.0f;
  auto bottom = top + 230.0f;

  auto slider = new Slider(
    CRect(left, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);
  slider->setSliderMode(CSliderMode::FreeClick);
  slider->setStyle(CSlider::kBottom | CSlider::kVertical);
  slider->setDrawStyle(
    CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
    | (drawFromCenter ? CSlider::kDrawValueFromCenter | CSlider::kDrawInverted : 0));
  slider->setBackColor(colorWhite);
  slider->setValueColor(valueColor);
  slider->setHighlightColor(valueColor);
  slider->setDefaultFrameColor(colorBlack);
  slider->setHighlightWidth(3.0);
  slider->setValueNormalized(controller->getParamNormalized(tag));
  slider->setDefaultValue(param.value[tag]->getDefaultNormalized());
  slider->setTooltipText(tooltip);
  frame->addView(slider);
  addToControlMap(tag, slider);

  top = bottom + margin;
  bottom = top + labelHeight;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  frame->addView(label);

  return std::make_tuple(slider, label);
}

TextButton *PlugEditor::addButton(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + labelHeight;

  auto button = new TextButton(
    CRect(left, top, right, bottom), this, tag, title, (CTextButton::Style)style);
  button->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setTextColor(colorBlack);
  button->setTextColorHighlighted(colorBlack);
  button->setGradient(CGradient::create(0.0, 1.0, colorWhite, colorWhite));
  button->setGradientHighlighted(CGradient::create(0.0, 1.0, colorOrange, colorOrange));
  button->setHighlightColor(colorOrange);
  button->setFrameColorHighlighted(colorBlack);
  button->setFrameWidth(1.0);
  button->setRoundRadius(0.0);
  button->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(button);
  addToControlMap(tag, button);
  return button;
}

MessageButton *PlugEditor::addStateButton(
  CCoord left,
  CCoord top,
  CCoord width,
  std::string label,
  std::string messageID,
  std::string)
{
  auto right = left + width;
  auto bottom = top + 2.0 * labelHeight;

  auto button = new MessageButton(
    controller, CRect(left, top, right, bottom), label, messageID,
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setHighlightColor(colorRed);
  frame->addView(button);
  return button;
}

CheckBox *PlugEditor::addCheckbox(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + 20.0;

  auto checkbox
    = new CheckBox(CRect(left, top, right, bottom), this, tag, title, nullptr, style);
  checkbox->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  checkbox->setFontColor(colorBlack);
  checkbox->setBoxFrameColor(colorBlack);
  checkbox->setBoxFillColor(colorWhite);
  checkbox->setCheckMarkColor(colorBlue);
  checkbox->sizeToFit();
  checkbox->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(checkbox);
  addToControlMap(tag, checkbox);
  return checkbox;
}

OptionMenu *PlugEditor::addOptionMenu(
  CCoord left,
  CCoord top,
  CCoord width,
  ParamID tag,
  const std::vector<UTF8String> &items)
{
  auto right = left + width;
  auto bottom = top + 20.0;

  auto menu = new OptionMenu(
    CRect(left, top, right, bottom), this, tag, nullptr, nullptr,
    COptionMenu::kCheckStyle);
  for (const auto &item : items) menu->addEntry(item);
  menu->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  menu->setFontColor(colorBlack);
  menu->setBackColor(colorWhite);
  menu->setDefaultFrameColor(colorBlack);
  menu->setHighlightColor(colorBlue);
  menu->sizeToFit();
  menu->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(menu);
  addToControlMap(tag, menu);
  return menu;
}

std::tuple<Knob *, CTextLabel *> PlugEditor::addKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
  return std::make_tuple(knob, label);
}

std::tuple<RotaryKnob *, CTextLabel *> PlugEditor::addRotaryKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new RotaryKnob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
  return std::make_tuple(knob, label);
}

template<typename Scale>
std::tuple<NumberKnob<Scale> *, CTextLabel *> PlugEditor::addNumberKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  Scale &scale,
  int32_t offset,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new NumberKnob<Scale>(
    CRect(left + 5.0, top, right - 5.0, bottom), this, tag, scale, offset);
  knob->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  auto label = addKnobLabel(left, top, right, bottom, name, labelPosition);
  return std::make_tuple(knob, label);
}

CTextLabel *PlugEditor::addKnobLabel(
  CCoord left,
  CCoord top,
  CCoord right,
  CCoord bottom,
  UTF8String name,
  LabelPosition labelPosition)
{
  switch (labelPosition) {
    default:
    case LabelPosition::bottom:
      top = bottom;
      bottom = top + 30.0;
      left -= 10.0;
      right += 10.0;
      break;

    case LabelPosition::right:
      left = right + margin;
      right = left + 100.0;
      break;
  }

  auto label
    = new CTextLabel(CRect(left - 10.0, top, right + 10.0, bottom), UTF8String(name));
  label->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(CColor{0, 0, 0, 0});
  if (labelPosition == LabelPosition::right) label->sizeToFit();
  frame->addView(label);
  return label;
}

template<typename Scale>
TextKnob<Scale> *PlugEditor::addTextKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  ParamID tag,
  Scale &scale,
  bool isDecibel,
  uint32_t precision,
  int32_t offset)
{
  auto bottom = top + labelHeight;
  auto right = left + width;

  auto knob
    = new TextKnob<Scale>(CRect(left, top, right, bottom), this, tag, scale, isDecibel);
  knob->setFont(
    new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  knob->setPrecision(precision);
  knob->offset = offset;
  frame->addView(knob);
  addToControlMap(tag, knob);
  return knob;
}

TextView *PlugEditor::addTextView(
  CCoord left, CCoord top, CCoord width, CCoord height, std::string text, CCoord textSize)
{
  auto bottom = top + height;
  auto right = left + width;

  auto view = new TextView(
    CRect(left, top, right, bottom), text,
    new CFontDesc(PlugEditorStyle::fontName(), textSize, CTxtFace::kNormalFace));
  frame->addView(view);
  return view;
}

TextTableView *PlugEditor::addTextTableView(
  CCoord left,
  CCoord top,
  CCoord width,
  CCoord height,
  std::string text,
  float cellWidth,
  CCoord textSize)
{
  auto bottom = top + height;
  auto right = left + width;

  auto view = new TextTableView(
    CRect(left, top, right, bottom), text, cellWidth,
    new CFontDesc(PlugEditorStyle::fontName(), textSize, CTxtFace::kNormalFace));
  frame->addView(view);
  return view;
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
