// (c) 2023 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float uiMargin = 20.0f;
constexpr float margin = 5.0f;
constexpr float labelWidth = 100.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float knobWidth = 50.0f;
constexpr float smallKnobWidth = int(knobWidth / 2);
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelY;
constexpr float barBoxWidth = 512.0f;
constexpr float barBoxHeight = 200.0f;
constexpr float innerWidth = 2 * labelWidth + 2 * margin + 2 * barBoxWidth + 6 * uiMargin;
constexpr float innerHeight = 3 * labelY + 2 * knobY + 2 * barBoxHeight + 3 * uiMargin;
constexpr uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

enum tabIndexMod { tabOscMod, tabLfo };
enum tabIndexWave { tabWavetable, tabWaveMod, tabModulation };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::syncUI(ParamID id, float normalized)
{
  auto syncer = xyControlMap.find(id);
  if (syncer == xyControlMap.end()) return;
  syncer->second->sync(normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  PlugEditor::valueChanged(pControl);
  syncUI(pControl->getTag(), pControl->getValueNormalized());
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);
  syncUI(id, normalized);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = 20.0f;
  constexpr auto left0 = 20.0f;

  constexpr auto textKnobSectionWidth = 2 * labelWidth + 2 * margin;

  // Gain.
  constexpr auto gainLeft0 = left0;
  constexpr auto gainLeft1 = gainLeft0 + labelWidth + 2 * margin;
  constexpr auto gainTop0 = top0;
  constexpr auto gainTop1 = gainTop0 + labelY;
  constexpr auto gainTop2 = gainTop1 + labelY;
  constexpr auto gainTop3 = gainTop2 + labelY;
  constexpr auto gainTop4 = gainTop3 + labelY;
  constexpr auto gainTop5 = gainTop4 + labelY;
  constexpr auto gainTop6 = gainTop5 + labelY;
  constexpr auto gainTop7 = gainTop6 + knobY;
  addGroupLabel(
    gainLeft0, gainTop0, textKnobSectionWidth, labelHeight, midTextSize, "Gain");
  addLabel(gainLeft0, gainTop1, labelWidth, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    gainLeft1, gainTop1, labelWidth, labelHeight, uiTextSize, ID::gain, Scales::gain,
    true, 5);
  addLabel(gainLeft0, gainTop2, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    gainLeft1, gainTop2, labelWidth, labelHeight, uiTextSize, ID::gainAttackSecond,
    Scales::envelopeSecond, false, 5);
  addLabel(gainLeft0, gainTop3, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    gainLeft1, gainTop3, labelWidth, labelHeight, uiTextSize, ID::gainDecaySecond,
    Scales::envelopeSecond, false, 5);
  addLabel(gainLeft0, gainTop4, labelWidth, labelHeight, uiTextSize, "Sustain [dB]");
  addTextKnob(
    gainLeft1, gainTop4, labelWidth, labelHeight, uiTextSize, ID::gainSustainAmplitude,
    Scales::envelopeSustainAmplitude, true, 5);
  addLabel(gainLeft0, gainTop5, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    gainLeft1, gainTop5, labelWidth, labelHeight, uiTextSize, ID::gainReleaseSecond,
    Scales::envelopeSecond, false, 5);

  addKnob(
    gainLeft0 + labelWidth + margin - int(knobWidth / 2), gainTop6, knobWidth, margin,
    uiTextSize, "Osc. Mix", ID::oscMix);

  addToggleButton(
    gainLeft0, gainTop7, labelWidth, labelHeight, uiTextSize, "DC Highpass [Hz]",
    ID::dcHighpassEnable);
  addTextKnob(
    gainLeft1, gainTop7, labelWidth, labelHeight, uiTextSize, ID::dcHighpassCutoffHz,
    Scales::cutoffHz, false, 3);

  // Tuning.
  constexpr auto tuningLeft0 = left0;
  constexpr auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  constexpr auto tuningTop0 = gainTop7 + labelY;
  constexpr auto tuningTop1 = tuningTop0 + labelY;
  constexpr auto tuningTop2 = tuningTop1 + labelY;
  constexpr auto tuningTop3 = tuningTop2 + labelY;
  constexpr auto tuningTop4 = tuningTop3 + labelY;
  constexpr auto tuningTop5 = tuningTop4 + labelY;
  constexpr auto tuningTop6 = tuningTop5 + labelY;
  addGroupLabel(
    tuningLeft0, tuningTop0, textKnobSectionWidth, labelHeight, midTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::octave,
    Scales::octave, false, 0, -12);

  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::semitone,
    Scales::semitone, false, 0, -120);

  addLabel(
    tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Milli [cent/10]");
  auto knobOscMilli = addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::milli,
    Scales::milli, false, 0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;

  addLabel(tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize, "Equal Temp.");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::equalTemperament,
    Scales::equalTemperament, false, 0, 1);

  addLabel(tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "A4 [Hz]");
  addTextKnob(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::pitchA4Hz,
    Scales::pitchA4Hz, false, 0, 100);

  addLabel(
    tuningLeft0, tuningTop6, labelWidth, labelHeight, uiTextSize, "Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop6, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 3);

  // Misc.
  constexpr auto miscLeft0 = tuningLeft0;
  constexpr auto miscLeft1 = miscLeft0 + labelWidth + 2 * margin;
  constexpr auto miscTop0 = tuningTop6 + labelY;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscTop2 = miscTop1 + labelY;
  constexpr auto miscTop3 = miscTop2 + labelY;
  addGroupLabel(
    miscLeft0, miscTop0, textKnobSectionWidth, labelHeight, midTextSize, "Misc.");

  addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "Oversampling");
  std::vector<std::string> oversamplingItems{"1x", "2x", "16x"};
  addOptionMenu<Style::warning>(
    miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize, ID::oversampling,
    oversamplingItems);
  addLabel(miscLeft0, miscTop2, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop2, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 3);
  addLabel(miscLeft0, miscTop3, labelWidth, labelHeight, uiTextSize, "Voice");
  addTextKnob(
    miscLeft1, miscTop3, labelWidth, labelHeight, uiTextSize, ID::nVoice, Scales::nVoice,
    false, 0, 1);

  // TabView modulation.
  constexpr auto tabViewModLeft = gainLeft0 + textKnobSectionWidth + 4 * margin;
  constexpr auto tabViewModWidth = barBoxWidth + 2 * uiMargin;

  std::vector<std::string> modTabs{"Oscillator", "LFO"};
  auto tabViewMod = addTabView(
    tabViewModLeft, top0, tabViewModWidth, innerHeight, uiTextSize, labelY, modTabs);

  constexpr auto modTabInsideLeft = tabViewModLeft + uiMargin;
  constexpr auto modTabInsideTop = top0 + labelY + uiMargin;
  constexpr auto modTabInsideWidth = tabViewModWidth - 2 * uiMargin;

  // Oscillator.
  constexpr auto xyPadWidth = 4 * smallKnobWidth;
  constexpr auto xyPadX = xyPadWidth + smallKnobWidth + 4 * margin;
  constexpr auto oscLeft0 = modTabInsideLeft;
  constexpr auto oscLeft1 = oscLeft0 + xyPadX;
  constexpr auto oscLeft2 = oscLeft1 + xyPadX;
  constexpr auto oscTop1 = modTabInsideTop;
  constexpr auto oscTop2 = oscTop1 + labelY;
  constexpr auto oscTop3 = oscTop2 + xyPadWidth + 2 * margin;
  constexpr auto oscTop4 = oscTop3 + labelY;
  constexpr auto oscTop5 = oscTop4 + xyPadWidth + 2 * margin;
  constexpr auto oscTop6 = oscTop5 + labelY;
  constexpr auto oscTop7 = oscTop6 + xyPadWidth + 2 * margin;
  constexpr auto oscTop8 = oscTop7 + labelY;

  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft0, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Pitch"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft0, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::oscPitchSemitone0, ID::oscPitchSemitone0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft1, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Feed Mix"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft1, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::sumMix0, ID::sumMix0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft2, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Feed LP"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft2, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::feedbackLowpassHz0, ID::feedbackLowpassHz0 + 1);

  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft0, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "Im. PM"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft0, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::sumToImmediatePm0, ID::sumToImmediatePm0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft1, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "Acc. PM"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft1, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::sumToAccumulatePm0, ID::sumToAccumulatePm0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft2, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "FM"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft2, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::sumToFm0, ID::sumToFm0 + 1);

  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft0, oscTop5, 4 * smallKnobWidth, labelHeight, uiTextSize, "AM"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft0, oscTop6, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::sumToAm0, ID::sumToAm0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(
      oscLeft1, oscTop5, 4 * smallKnobWidth, labelHeight, uiTextSize, "Hard Sync."));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft1, oscTop6, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::hardSync0, ID::hardSync0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(oscLeft2, oscTop5, 4 * smallKnobWidth, labelHeight, uiTextSize, "Skew"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft2, oscTop6, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::phaseSkew0, ID::phaseSkew0 + 1);

  tabViewMod->addWidget(
    tabOscMod,
    addLabel(
      oscLeft0, oscTop7, 4 * smallKnobWidth, labelHeight, uiTextSize, "Distortion"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft0, oscTop8, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::distortion0, ID::distortion0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(
      oscLeft1, oscTop7, 4 * smallKnobWidth, labelHeight, uiTextSize, "Spc. Spread"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft1, oscTop8, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::spectralSpread0, ID::spectralSpread0 + 1);
  tabViewMod->addWidget(
    tabOscMod,
    addLabel(
      oscLeft2, oscTop7, 4 * smallKnobWidth, labelHeight, uiTextSize, "Phase Slope"));
  addXYControls(
    tabViewMod, tabOscMod, oscLeft2, oscTop8, 4 * smallKnobWidth, smallKnobWidth, margin,
    uiTextSize, ID::phaseSlope0, ID::phaseSlope0 + 1);

  // LFO.
  constexpr auto lfoLeft0 = modTabInsideLeft;
  constexpr auto lfoLeft1 = lfoLeft0 + knobX;
  constexpr auto lfoLeft2 = lfoLeft1 + knobX;
  constexpr auto lfoTop0 = modTabInsideTop;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + knobY;
  constexpr auto lfoTop3 = lfoTop2 + barBoxHeight + uiMargin;
  constexpr auto lfoTop4 = lfoTop3 + labelY;
  constexpr auto lfoTop5 = lfoTop4 + knobY;

  tabViewMod->addWidget(
    tabLfo,
    addGroupLabel(
      lfoLeft0, lfoTop0, modTabInsideWidth, labelHeight, midTextSize, "LFO 0"));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate0 + 0));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(
      lfoLeft1, lfoTop1, knobWidth, margin, uiTextSize, "Key", ID::lfoKeyFollow0 + 0));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(
      lfoLeft2, lfoTop1, knobWidth, margin, uiTextSize, "Lowpass",
      ID::lfoLowpassHz0 + 0));

  auto barBoxLfo0Waveform = addBarBox(
    lfoLeft0, lfoTop2, barBoxWidth, barBoxHeight, ID::lfo0Waveform0, nLfoWavetable,
    Scales::bipolarScale, "LFO 0 Wave");
  if (barBoxLfo0Waveform) {
    barBoxLfo0Waveform->sliderZero = 0.5f;
  }
  tabViewMod->addWidget(tabLfo, barBoxLfo0Waveform);

  tabViewMod->addWidget(
    tabLfo,
    addGroupLabel(
      lfoLeft0, lfoTop3, modTabInsideWidth, labelHeight, midTextSize, "LFO 1"));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(lfoLeft0, lfoTop4, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate0 + 1));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(
      lfoLeft1, lfoTop4, knobWidth, margin, uiTextSize, "Key", ID::lfoKeyFollow0 + 1));
  tabViewMod->addWidget(
    tabLfo,
    addKnob(
      lfoLeft2, lfoTop4, knobWidth, margin, uiTextSize, "Lowpass",
      ID::lfoLowpassHz0 + 1));

  auto barBoxLfo1Waveform = addBarBox(
    lfoLeft0, lfoTop5, barBoxWidth, barBoxHeight, ID::lfo1Waveform0, nLfoWavetable,
    Scales::bipolarScale, "LFO 1 Wave");
  if (barBoxLfo1Waveform) {
    barBoxLfo1Waveform->sliderZero = 0.5f;
  }
  tabViewMod->addWidget(tabLfo, barBoxLfo1Waveform);

  // TabView wave.
  constexpr auto tabViewWaveLeft = tabViewModLeft + tabViewModWidth + uiMargin;
  constexpr auto tabViewWaveWidth = barBoxWidth + 2 * uiMargin;

  std::vector<std::string> waveTabs{"Wavetable", "Wave Mod.", "Env. / Ext. / Matrix"};
  auto tabViewWave = addTabView(
    tabViewWaveLeft, top0, tabViewWaveWidth, innerHeight, uiTextSize, labelY, waveTabs);

  constexpr auto waveTabInsideLeft = tabViewWaveLeft + uiMargin;
  constexpr auto waveTabInsideTop = top0 + labelY + uiMargin;
  constexpr auto waveTabInsideWidth = tabViewWaveWidth - 2 * uiMargin;

  // Wavetable.
  constexpr auto waveLeft0 = waveTabInsideLeft;
  constexpr auto waveLeft1 = waveLeft0 + knobX;
  constexpr auto waveLeft2 = waveLeft1 + knobX;
  constexpr auto waveLeft3 = waveLeft2 + knobX;
  constexpr auto waveLeft4 = waveLeft3 + knobX;
  constexpr auto waveLeft5 = waveLeft4 + knobX;
  constexpr auto waveLeft6 = waveLeft5 + knobX;
  constexpr auto waveLeft7 = waveLeft6 + knobX;
  constexpr auto waveTop0 = waveTabInsideTop;
  constexpr auto waveTop1 = waveTop0 + labelY;
  constexpr auto waveTop2 = waveTop1 + knobY;
  constexpr auto waveTop3 = waveTop2 + barBoxHeight + uiMargin;
  constexpr auto waveTop4 = waveTop3 + labelY;
  constexpr auto waveTop5 = waveTop4 + knobY;

  std::vector<std::string> oscInterpolationTypeItems{
    "Step Interp.", "Linear Interp.", "Cubic Interp."};

  tabViewWave->addWidget(
    tabWavetable,
    addGroupLabel(
      waveLeft0, waveTop0, waveTabInsideWidth, labelHeight, midTextSize, "Oscillator 0"));
  tabViewWave->addWidget(
    tabWavetable,
    addKnob(
      waveLeft5, waveTop1, knobWidth, margin, uiTextSize, "Lowpass",
      ID::spectralLowpass0 + 0));
  tabViewWave->addWidget(
    tabWavetable,
    addKnob(
      waveLeft6, waveTop1, knobWidth, margin, uiTextSize, "Highpass",
      ID::spectralHighpass0 + 0));
  tabViewWave->addWidget(
    tabWavetable,
    addOptionMenu(
      waveLeft7, waveTop1, labelWidth, labelHeight, uiTextSize,
      ID::oscInterpolationType0 + 0, oscInterpolationTypeItems));
  auto barBoxOsc0Waveform = addBarBox(
    waveLeft0, waveTop2, barBoxWidth, barBoxHeight, ID::osc0Waveform0, nOscWavetable,
    Scales::bipolarScale, "Osc. 0 Wave");
  if (barBoxOsc0Waveform) {
    barBoxOsc0Waveform->sliderZero = 0.5f;
  }
  tabViewWave->addWidget(tabWavetable, barBoxOsc0Waveform);

  tabViewWave->addWidget(
    tabWavetable,
    addGroupLabel(
      waveLeft0, waveTop3, waveTabInsideWidth, labelHeight, midTextSize, "Oscillator 1"));
  tabViewWave->addWidget(
    tabWavetable,
    addKnob(
      waveLeft5, waveTop4, knobWidth, margin, uiTextSize, "Lowpass",
      ID::spectralLowpass0 + 1));
  tabViewWave->addWidget(
    tabWavetable,
    addKnob(
      waveLeft6, waveTop4, knobWidth, margin, uiTextSize, "Highpass",
      ID::spectralHighpass0 + 1));
  tabViewWave->addWidget(
    tabWavetable,
    addOptionMenu(
      waveLeft7, waveTop4, labelWidth, labelHeight, uiTextSize,
      ID::oscInterpolationType0 + 1, oscInterpolationTypeItems));
  auto barBoxOsc1Waveform = addBarBox(
    waveLeft0, waveTop5, barBoxWidth, barBoxHeight, ID::osc1Waveform0, nOscWavetable,
    Scales::bipolarScale, "Osc. 1 Wave");
  if (barBoxOsc1Waveform) {
    barBoxOsc1Waveform->sliderZero = 0.5f;
  }
  tabViewWave->addWidget(tabWavetable, barBoxOsc1Waveform);

  // Wave Mod.
  constexpr auto wmLeft0 = waveTabInsideLeft;
  constexpr auto wmLeft1 = wmLeft0 + knobX;
  constexpr auto wmLeft2 = wmLeft1 + knobX;
  constexpr auto wmLeft3 = wmLeft2 + knobX;
  constexpr auto wmLeft4 = wmLeft3 + knobX;
  constexpr auto wmTop0 = waveTabInsideTop;
  constexpr auto wmTop1 = wmTop0 + labelY;
  constexpr auto wmTop2 = wmTop1 + knobY;
  constexpr auto wmTop3 = wmTop2 + int((barBoxHeight + uiMargin) / 2);
  constexpr auto wmTop4 = wmTop3 + int((barBoxHeight + uiMargin) / 2);
  constexpr auto wmTop5 = wmTop4 + labelY;
  constexpr auto wmTop6 = wmTop5 + knobY;
  constexpr auto wmTop7 = wmTop6 + int((barBoxHeight + uiMargin) / 2);

  tabViewWave->addWidget(
    tabWaveMod,
    addGroupLabel(
      wmLeft0, wmTop0, waveTabInsideWidth, labelHeight, midTextSize,
      "Wavetable Modulation 0"));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft0, wmTop1, knobWidth, margin, uiTextSize, "Input",
      ID::waveMod0Input0 + nWaveModInput - 1));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft1, wmTop1, knobWidth, margin, uiTextSize, "Env. 0",
      ID::waveMod0Input0 + ModID::env0));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft2, wmTop1, knobWidth, margin, uiTextSize, "Env. 1",
      ID::waveMod0Input0 + ModID::env1));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft3, wmTop1, knobWidth, margin, uiTextSize, "LFO 0",
      ID::waveMod0Input0 + ModID::lfo0));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft4, wmTop1, knobWidth, margin, uiTextSize, "LFO 1",
      ID::waveMod0Input0 + ModID::lfo1));

  auto barBoxWm0Gain = addBarBox(
    wmLeft0, wmTop2, barBoxWidth, int(barBoxHeight / 2), ID::waveMod0Gain0, nOscWavetable,
    Scales::bipolarScale, "Wave Mod. 0 Gain");
  if (barBoxWm0Gain) {
    barBoxWm0Gain->sliderZero = 0.5f;
  }
  tabViewWave->addWidget(tabWaveMod, barBoxWm0Gain);
  auto barBoxWm0Lp = addBarBox(
    wmLeft0, wmTop3, barBoxWidth, int(barBoxHeight / 2), ID::waveMod0Delay0,
    nOscWavetable, Scales::waveModDelay, "Wave Mod. 0 Delay");
  tabViewWave->addWidget(tabWaveMod, barBoxWm0Lp);

  tabViewWave->addWidget(
    tabWaveMod,
    addGroupLabel(
      wmLeft0, wmTop4, waveTabInsideWidth, labelHeight, midTextSize,
      "Wavetable Modulation 1"));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft0, wmTop5, knobWidth, margin, uiTextSize, "Input",
      ID::waveMod1Input0 + nWaveModInput - 1));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft1, wmTop5, knobWidth, margin, uiTextSize, "Env. 0",
      ID::waveMod1Input0 + ModID::env0));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft2, wmTop5, knobWidth, margin, uiTextSize, "Env. 1",
      ID::waveMod1Input0 + ModID::env1));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft3, wmTop5, knobWidth, margin, uiTextSize, "LFO 0",
      ID::waveMod1Input0 + ModID::lfo0));
  tabViewWave->addWidget(
    tabWaveMod,
    addKnob(
      wmLeft4, wmTop5, knobWidth, margin, uiTextSize, "LFO 1",
      ID::waveMod1Input0 + ModID::lfo1));

  auto barBoxWm1Gain = addBarBox(
    wmLeft0, wmTop6, barBoxWidth, int(barBoxHeight / 2), ID::waveMod1Gain0, nOscWavetable,
    Scales::bipolarScale, "Wave Mod. 1 Gain");
  if (barBoxWm1Gain) {
    barBoxWm1Gain->sliderZero = 0.5f;
  }
  tabViewWave->addWidget(tabWaveMod, barBoxWm1Gain);
  auto barBoxWm1Lp = addBarBox(
    wmLeft0, wmTop7, barBoxWidth, int(barBoxHeight / 2), ID::waveMod1Delay0,
    nOscWavetable, Scales::waveModDelay, "Wave Mod. 1 Delay");
  tabViewWave->addWidget(tabWaveMod, barBoxWm1Lp);

  // Modulation.
  constexpr auto envLeft0 = waveTabInsideLeft;
  constexpr auto envLeft1 = envLeft0 + labelWidth + 2 * margin;
  constexpr auto envLeft2 = waveTabInsideLeft + int(waveTabInsideWidth / 2);
  constexpr auto envLeft3 = envLeft2 + labelWidth + 2 * margin;
  constexpr auto env0Top0 = waveTabInsideTop;
  constexpr auto env0Top1 = env0Top0 + labelY;
  constexpr auto env0Top2 = env0Top1 + labelY;
  constexpr auto env0Top3 = env0Top2 + labelY;
  constexpr auto env0Top4 = env0Top3 + labelY;
  constexpr auto env1Top0 = env0Top0;
  constexpr auto env1Top1 = env1Top0 + labelY;
  constexpr auto env1Top2 = env1Top1 + labelY;
  constexpr auto env1Top3 = env1Top2 + labelY;
  constexpr auto env1Top4 = env1Top3 + labelY;
  constexpr auto extTop0 = env1Top4 + labelY;
  constexpr auto extTop1 = extTop0 + labelY;
  constexpr auto modLeft0 = waveTabInsideLeft;
  constexpr auto modLeft1 = modLeft0 + labelWidth + 2 * margin;
  constexpr auto modTop0 = extTop1 + labelY;
  constexpr auto modTop1 = modTop0 + labelY;
  constexpr auto modTop2 = modTop1 + labelY;
  constexpr auto modTop3 = modTop2 + labelY;

  tabViewWave->addWidget(
    tabModulation,
    addGroupLabel(
      envLeft0, env0Top0, textKnobSectionWidth, labelHeight, midTextSize, "Envelope 0"));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft0, env0Top1, labelWidth, labelHeight, uiTextSize, "Attack [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft1, env0Top1, labelWidth, labelHeight, uiTextSize,
      ID::envelopeAttackSecond0 + 0, Scales::envelopeSecond, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft0, env0Top2, labelWidth, labelHeight, uiTextSize, "Decay [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft1, env0Top2, labelWidth, labelHeight, uiTextSize,
      ID::envelopeDecaySecond0 + 0, Scales::envelopeSecond, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft0, env0Top3, labelWidth, labelHeight, uiTextSize, "Sustain"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft1, env0Top3, labelWidth, labelHeight, uiTextSize,
      ID::envelopeSustainAmplitude0 + 0, Scales::envelopeSustainAmplitude, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft0, env0Top4, labelWidth, labelHeight, uiTextSize, "Release [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft1, env0Top4, labelWidth, labelHeight, uiTextSize,
      ID::envelopeReleaseSecond0 + 0, Scales::envelopeSecond, false, 5));

  tabViewWave->addWidget(
    tabModulation,
    addGroupLabel(
      envLeft2, env1Top0, textKnobSectionWidth, labelHeight, midTextSize, "Envelope 1"));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft2, env1Top1, labelWidth, labelHeight, uiTextSize, "Attack [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft3, env1Top1, labelWidth, labelHeight, uiTextSize,
      ID::envelopeAttackSecond0 + 1, Scales::envelopeSecond, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft2, env1Top2, labelWidth, labelHeight, uiTextSize, "Decay [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft3, env1Top2, labelWidth, labelHeight, uiTextSize,
      ID::envelopeDecaySecond0 + 1, Scales::envelopeSecond, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft2, env1Top3, labelWidth, labelHeight, uiTextSize, "Sustain"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft3, env1Top3, labelWidth, labelHeight, uiTextSize,
      ID::envelopeSustainAmplitude0 + 1, Scales::envelopeSustainAmplitude, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft2, env1Top4, labelWidth, labelHeight, uiTextSize, "Release [s]"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft3, env1Top4, labelWidth, labelHeight, uiTextSize,
      ID::envelopeReleaseSecond0 + 1, Scales::envelopeSecond, false, 5));

  tabViewWave->addWidget(
    tabModulation,
    addGroupLabel(
      envLeft0, extTop0, waveTabInsideWidth, labelHeight, midTextSize, "External Input"));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft0, extTop1, labelWidth, labelHeight, uiTextSize, "Ext. 0"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft1, extTop1, labelWidth, labelHeight, uiTextSize, ID::externalInput0 + 0,
      Scales::bipolarScale, false, 5));
  tabViewWave->addWidget(
    tabModulation,
    addLabel(envLeft2, extTop1, labelWidth, labelHeight, uiTextSize, "Ext. 1"));
  tabViewWave->addWidget(
    tabModulation,
    addTextKnob(
      envLeft3, extTop1, labelWidth, labelHeight, uiTextSize, ID::externalInput0 + 1,
      Scales::bipolarScale, false, 5));

  tabViewWave->addWidget(
    tabModulation,
    addGroupLabel(
      modLeft0, modTop0, waveTabInsideWidth, labelHeight, midTextSize, "Matrix"));

  // Modulation matrix source label.
  constexpr std::array<const char *, ModID::MODID_ENUM_LENGTH> modSourceLabelText{
    "Env. 0", "Env. 1", "LFO 0", "LFO 1", "Ext. 0", "Ext. 1"};
  for (size_t idx = 0; idx < modSourceLabelText.size(); ++idx) {
    tabViewWave->addWidget(
      tabModulation,
      addLabel(
        modLeft1 + idx * knobWidth, modTop1, knobWidth, labelHeight, uiTextSize,
        modSourceLabelText[idx]));
  }

  // Oscillator modulation matrix.
  constexpr std::array<const char *, nOscillator> oscLabelText{"0", "1"};
  for (size_t idx = 0; idx < oscLabelText.size(); ++idx) {
    tabViewWave->addWidget(
      tabModulation,
      addLabel(
        modLeft1 + idx * smallKnobWidth, modTop2, smallKnobWidth, labelHeight, uiTextSize,
        oscLabelText[idx]));
  }
  for (size_t idx = 0; idx < nModulation; ++idx) {
    tabViewWave->addWidget(
      tabModulation,
      addLabel(
        modLeft1 + idx * smallKnobWidth, modTop2, smallKnobWidth, labelHeight, uiTextSize,
        oscLabelText[idx % oscLabelText.size()]));
  }

  constexpr auto oscModIdEnd = ID::modSpectralHighpass0 + nModulation;
  constexpr auto oscModMatrixRow = (oscModIdEnd - ID::modPitch0) / nModulation;
  constexpr auto oscModMatrixCol = 2 * ModID::MODID_ENUM_LENGTH;
  std::vector<ParamID> modMatrixId;
  for (ParamID idCol = ID::modPitch0; idCol < oscModIdEnd; idCol += oscModMatrixCol) {
    for (ParamID idOffset = 0; idOffset < ModID::MODID_ENUM_LENGTH; ++idOffset) {
      for (ParamID osc = 0; osc < nOscillator; ++osc) {
        modMatrixId.push_back(idCol + idOffset + osc * ModID::MODID_ENUM_LENGTH);
      }
    }
  }
  tabViewWave->addWidget(
    tabModulation,
    addMatrixKnob(
      modLeft1, modTop3, oscModMatrixCol * smallKnobWidth,
      oscModMatrixRow * smallKnobWidth, oscModMatrixRow, oscModMatrixCol, modMatrixId));

  constexpr std::array<const char *, oscModMatrixRow> modDestinationLabelText{
    "Pitch",       "Feedback Mix", "Immediate PM", "Accumulate PM",
    "FM",          "Hard Sync.",   "Phase Skew",   "Distortion",
    "Spc. Spread", "Phase Slope",  "Lowpass",      "Highpass",
  };
  for (size_t idx = 0; idx < modDestinationLabelText.size(); ++idx) {
    tabViewWave->addWidget(
      tabModulation,
      addLabel(
        modLeft0, modTop3 + idx * smallKnobWidth, labelWidth, smallKnobWidth, uiTextSize,
        modDestinationLabelText[idx]));
  }

  // Plugin name.
  constexpr auto splashWidth = 2 * labelWidth + 2 * margin;
  constexpr float splashHeight = 2 * labelHeight;
  constexpr auto splashTop = innerHeight - splashHeight + uiMargin;
  constexpr auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "TestBedSynth", true);

  tabViewMod->refreshTab();
  tabViewWave->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
