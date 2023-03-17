// (c) 2021 Takamitsu Endo
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
constexpr float splashHeight = 40.0f;
constexpr float labelWidth = 100.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float knobWidth = 50.0f;
constexpr float smallKnobWidth = int(knobWidth / 2);
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelY;
constexpr float barBoxWidth = 512.0f;
constexpr float barBoxHeight = 200.0f;
constexpr float innerWidth
  = 3 * labelWidth + 12 * smallKnobWidth + 6 * margin + barBoxWidth + 4 * uiMargin;
constexpr float innerHeight = 3 * labelY + 2 * knobY + 2 * barBoxHeight + 3 * uiMargin;
constexpr uint32_t defaultWidth = uint32_t(innerWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(innerHeight + 2 * uiMargin);

enum tabIndex { tabWavetable, tabEnvelope, tabLfo };

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

  constexpr auto leftmostSectionWidth = 2 * labelWidth + 2 * margin;

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
    gainLeft0, gainTop0, leftmostSectionWidth, labelHeight, midTextSize, "Gain");
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
    tuningLeft0, tuningTop0, leftmostSectionWidth, labelHeight, midTextSize, "Tuning");

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
    miscLeft0, miscTop0, leftmostSectionWidth, labelHeight, midTextSize, "Misc.");

  /// TODO: Use or remove.
  // addLabel(miscLeft0, miscTop1, labelWidth, labelHeight, uiTextSize, "Oversampling");
  // std::vector<std::string> oversamplingItems{"1x", "16x"};
  // addOptionMenu<Style::warning>(
  //   miscLeft1, miscTop1, labelWidth, labelHeight, uiTextSize, ID::oversampling,
  //   oversamplingItems);

  addToggleButton(
    miscLeft0, miscTop1, leftmostSectionWidth, labelHeight, uiTextSize, "16x Sampling",
    ID::oversampling);
  addLabel(miscLeft0, miscTop2, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    miscLeft1, miscTop2, labelWidth, labelHeight, uiTextSize,
    ID::parameterSmoothingSecond, Scales::parameterSmoothingSecond, false, 3);
  addLabel(miscLeft0, miscTop3, labelWidth, labelHeight, uiTextSize, "Voice");
  addTextKnob(
    miscLeft1, miscTop3, labelWidth, labelHeight, uiTextSize, ID::nVoice, Scales::nVoice,
    false, 0, 1);

  // Oscillator.
  constexpr auto xyPadWidth = 4 * smallKnobWidth;
  constexpr auto xyPadX = xyPadWidth + smallKnobWidth + 4 * margin;
  constexpr auto oscLeft0 = gainLeft0 + leftmostSectionWidth + 4 * margin;
  constexpr auto oscLeft1 = oscLeft0 + xyPadX;
  constexpr auto oscLeft2 = oscLeft1 + xyPadX;
  constexpr auto oscTop0 = top0;
  constexpr auto oscTop1 = oscTop0 + labelY;
  constexpr auto oscTop2 = oscTop1 + labelY;
  constexpr auto oscTop3 = oscTop2 + xyPadWidth + 2 * margin;
  constexpr auto oscTop4 = oscTop3 + labelY;
  addGroupLabel(
    oscLeft0, oscTop0, labelWidth + 12 * smallKnobWidth + 4 * margin, labelHeight,
    midTextSize, "Oscillator");

  addLabel(oscLeft0, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Pitch");
  addXYControls(
    oscLeft0, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::oscPitchSemitone0, ID::oscPitchSemitone0 + 1);
  addLabel(oscLeft1, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Feed Mix");
  addXYControls(
    oscLeft1, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::sumMix0, ID::sumMix0 + 1);
  addLabel(oscLeft2, oscTop1, 4 * smallKnobWidth, labelHeight, uiTextSize, "Feed LP");
  addXYControls(
    oscLeft2, oscTop2, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::feedbackLowpassHz0, ID::feedbackLowpassHz0 + 1);

  addLabel(oscLeft0, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "Im. PM");
  addXYControls(
    oscLeft0, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::sumToImmediatePm0, ID::sumToImmediatePm0 + 1);
  addLabel(oscLeft1, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "Acc. PM");
  addXYControls(
    oscLeft1, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::sumToAccumulatePm0, ID::sumToAccumulatePm0 + 1);
  addLabel(oscLeft2, oscTop3, 4 * smallKnobWidth, labelHeight, uiTextSize, "FM");
  addXYControls(
    oscLeft2, oscTop4, 4 * smallKnobWidth, smallKnobWidth, margin, uiTextSize,
    ID::sumToFm0, ID::sumToFm0 + 1);

  // Modulation.
  constexpr auto modLeft0 = oscLeft0 + 2 * margin;
  constexpr auto modLeft1 = modLeft0 + labelWidth + 2 * margin;
  constexpr auto modTop0 = oscTop4 + xyPadWidth + 2 * margin;
  constexpr auto modTop1 = modTop0 + labelY;
  constexpr auto modTop2 = modTop1 + labelY;
  constexpr auto modTop3 = modTop2 + labelY;

  addGroupLabel(
    modLeft0, modTop0, labelWidth + 4 * margin + 12 * smallKnobWidth, labelHeight,
    midTextSize, "Modulation");

  // Modulation matrix source label.
  constexpr std::array<const char *, ModID::MODID_ENUM_LENGTH> modSourceLabelText{
    "Env. 0", "Env. 1", "LFO 0", "LFO 1", "Ext. 0", "Ext. 1"};
  for (size_t idx = 0; idx < modSourceLabelText.size(); ++idx) {
    addLabel(
      modLeft1 + idx * knobWidth, modTop1, knobWidth, labelHeight, uiTextSize,
      modSourceLabelText[idx]);
  }

  // Oscillator modulation matrix.
  constexpr std::array<const char *, nOscillator> oscLabelText{"0", "1"};
  for (size_t idx = 0; idx < oscLabelText.size(); ++idx) {
    addLabel(
      modLeft1 + idx * smallKnobWidth, modTop2, smallKnobWidth, labelHeight, uiTextSize,
      oscLabelText[idx]);
  }
  for (size_t idx = 0; idx < nModulation; ++idx) {
    addLabel(
      modLeft1 + idx * smallKnobWidth, modTop2, smallKnobWidth, labelHeight, uiTextSize,
      oscLabelText[idx % oscLabelText.size()]);
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
  addMatrixKnob(
    modLeft1, modTop3, oscModMatrixCol * smallKnobWidth, oscModMatrixRow * smallKnobWidth,
    oscModMatrixRow, oscModMatrixCol, modMatrixId);

  constexpr std::array<const char *, oscModMatrixRow> modDestinationLabelText{
    "Pitch",      "Feedback Mix", "Immediate PM", "Accumulate PM", "FM",
    "Hard Sync.", "Spc. Spread",  "Phase Slope",  "Lowpass",       "Highpass",
  };
  for (size_t idx = 0; idx < modDestinationLabelText.size(); ++idx) {
    addLabel(
      modLeft0, modTop3 + idx * smallKnobWidth, labelWidth, smallKnobWidth, uiTextSize,
      modDestinationLabelText[idx]);
  }

  // TabView.
  constexpr auto tabViewLeft
    = modLeft0 + labelWidth + 12 * smallKnobWidth + 4 * margin + uiMargin;
  constexpr auto tabViewWidth = barBoxWidth + 2 * uiMargin;

  std::vector<std::string> tabs{"Wavetable", "Envelope", "LFO"};
  auto tabView
    = addTabView(tabViewLeft, top0, tabViewWidth, innerHeight, uiTextSize, labelY, tabs);

  constexpr auto tabInsideLeft = tabViewLeft + uiMargin;
  constexpr auto tabInsideTop = top0 + labelY + uiMargin;
  constexpr auto tabInsideWidth = tabViewWidth - 2 * uiMargin;

  // Wavetable.
  constexpr auto waveLeft0 = tabInsideLeft;
  constexpr auto waveLeft1 = waveLeft0 + knobX;
  constexpr auto waveLeft2 = waveLeft1 + knobX;
  constexpr auto waveLeft3 = waveLeft2 + knobX;
  constexpr auto waveLeft4 = waveLeft3 + knobX;
  constexpr auto waveTop0 = tabInsideTop;
  constexpr auto waveTop1 = waveTop0 + labelY;
  constexpr auto waveTop2 = waveTop1 + knobY;
  constexpr auto waveTop3 = waveTop2 + barBoxHeight + uiMargin;
  constexpr auto waveTop4 = waveTop3 + labelY;
  constexpr auto waveTop5 = waveTop4 + knobY;

  tabView->addWidget(
    tabWavetable,
    addGroupLabel(
      waveLeft0, waveTop0, tabInsideWidth, labelHeight, midTextSize, "Oscillator 0"));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft0, waveTop1, knobWidth, margin, uiTextSize, "Hard Sync.",
      ID::hardSync0 + 0));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft1, waveTop1, knobWidth, margin, uiTextSize, "Spc. Spread",
      ID::spectralSpread0 + 0));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft2, waveTop1, knobWidth, margin, uiTextSize, "Phase Slope",
      ID::phaseSlope0 + 0));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft3, waveTop1, knobWidth, margin, uiTextSize, "Lowpass",
      ID::spectralLowpass0 + 0));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft4, waveTop1, knobWidth, margin, uiTextSize, "Highpass",
      ID::spectralHighpass0 + 0));
  auto barBoxOsc0Waveform = addBarBox(
    waveLeft0, waveTop2, barBoxWidth, barBoxHeight, ID::osc0Waveform0, nOscWavetable,
    Scales::bipolarScale, "Osc. 0 Wave");
  if (barBoxOsc0Waveform) {
    barBoxOsc0Waveform->sliderZero = 0.5f;
  }
  tabView->addWidget(tabWavetable, barBoxOsc0Waveform);

  tabView->addWidget(
    tabWavetable,
    addGroupLabel(
      waveLeft0, waveTop3, tabInsideWidth, labelHeight, midTextSize, "Oscillator 1"));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft0, waveTop4, knobWidth, margin, uiTextSize, "Hard Sync.",
      ID::hardSync0 + 1));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft1, waveTop4, knobWidth, margin, uiTextSize, "Spc. Spread",
      ID::spectralSpread0 + 1));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft2, waveTop4, knobWidth, margin, uiTextSize, "Phase Slope",
      ID::phaseSlope0 + 1));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft3, waveTop4, knobWidth, margin, uiTextSize, "Lowpass",
      ID::spectralLowpass0 + 1));
  tabView->addWidget(
    tabWavetable,
    addKnob(
      waveLeft4, waveTop4, knobWidth, margin, uiTextSize, "Highpass",
      ID::spectralHighpass0 + 1));
  auto barBoxOsc1Waveform = addBarBox(
    waveLeft0, waveTop5, barBoxWidth, barBoxHeight, ID::osc1Waveform0, nOscWavetable,
    Scales::bipolarScale, "Osc. 1 Wave");
  if (barBoxOsc1Waveform) {
    barBoxOsc1Waveform->sliderZero = 0.5f;
  }
  tabView->addWidget(tabWavetable, barBoxOsc1Waveform);

  // Envelope.
  constexpr auto envLeft0 = tabInsideLeft;
  constexpr auto envLeft1 = envLeft0 + knobX;
  constexpr auto envLeft2 = envLeft1 + knobX;
  constexpr auto envLeft3 = envLeft2 + knobX;
  constexpr auto envLeft4 = envLeft3 + knobX;
  constexpr auto env0Top0 = tabInsideTop;
  constexpr auto env0Top1 = env0Top0 + labelY;
  constexpr auto env1Top0 = tabInsideTop + labelY + knobY + barBoxHeight + uiMargin;
  constexpr auto env1Top1 = env1Top0 + labelY;

  tabView->addWidget(
    tabEnvelope,
    addGroupLabel(
      envLeft0, env0Top0, tabInsideWidth, labelHeight, midTextSize, "Envelope 0"));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft0, env0Top1, knobWidth, margin, uiTextSize, "A", ID::envelopeAttackSecond0));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft1, env0Top1, knobWidth, margin, uiTextSize, "D", ID::envelopeDecaySecond0));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft2, env0Top1, knobWidth, margin, uiTextSize, "S",
      ID::envelopeSustainAmplitude0));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft3, env0Top1, knobWidth, margin, uiTextSize, "R",
      ID::envelopeReleaseSecond0));

  tabView->addWidget(
    tabEnvelope,
    addGroupLabel(
      envLeft0, env1Top0, tabInsideWidth, labelHeight, midTextSize, "Envelope 1"));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft0, env1Top1, knobWidth, margin, uiTextSize, "A",
      ID::envelopeAttackSecond0 + 1));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft1, env1Top1, knobWidth, margin, uiTextSize, "D",
      ID::envelopeDecaySecond0 + 1));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft2, env1Top1, knobWidth, margin, uiTextSize, "S",
      ID::envelopeSustainAmplitude0 + 1));
  tabView->addWidget(
    tabEnvelope,
    addKnob(
      envLeft3, env1Top1, knobWidth, margin, uiTextSize, "R",
      ID::envelopeReleaseSecond0 + 1));

  // LFO.
  constexpr auto lfoLeft0 = tabInsideLeft;
  constexpr auto lfoTop0 = tabInsideTop;
  constexpr auto lfoTop1 = lfoTop0 + labelY;
  constexpr auto lfoTop2 = lfoTop1 + knobY;
  constexpr auto lfoTop3 = lfoTop2 + barBoxHeight + uiMargin;
  constexpr auto lfoTop4 = lfoTop3 + labelY;
  constexpr auto lfoTop5 = lfoTop4 + knobY;

  tabView->addWidget(
    tabLfo,
    addGroupLabel(lfoLeft0, lfoTop0, tabInsideWidth, labelHeight, midTextSize, "LFO 0"));
  tabView->addWidget(
    tabLfo,
    addKnob(lfoLeft0, lfoTop1, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate0));

  auto barBoxLfo0Waveform = addBarBox(
    lfoLeft0, lfoTop2, barBoxWidth, barBoxHeight, ID::lfo0Waveform0, nLfoWavetable,
    Scales::bipolarScale, "LFO 0 Wave");
  if (barBoxLfo0Waveform) {
    barBoxLfo0Waveform->sliderZero = 0.5f;
  }
  tabView->addWidget(tabLfo, barBoxLfo0Waveform);

  tabView->addWidget(
    tabLfo,
    addGroupLabel(lfoLeft0, lfoTop3, tabInsideWidth, labelHeight, midTextSize, "LFO 1"));
  tabView->addWidget(
    tabLfo,
    addKnob(lfoLeft0, lfoTop4, knobWidth, margin, uiTextSize, "Rate", ID::lfoRate0 + 1));

  auto barBoxLfo1Waveform = addBarBox(
    lfoLeft0, lfoTop5, barBoxWidth, barBoxHeight, ID::lfo1Waveform0, nLfoWavetable,
    Scales::bipolarScale, "LFO 1 Wave");
  if (barBoxLfo1Waveform) {
    barBoxLfo1Waveform->sliderZero = 0.5f;
  }
  tabView->addWidget(tabLfo, barBoxLfo1Waveform);

  // // Plugin name.
  // const auto splashTop = innerHeight - splashHeight + uiMargin;
  // const auto splashLeft = uiMargin;
  // addSplashScreen(
  //   splashLeft, splashTop, 4 * knobX, splashHeight, 20.0f, 20.0f,
  //   defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
  //   "TestBedSynth");

  tabView->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg
