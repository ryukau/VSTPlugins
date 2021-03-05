// (c) 2020 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 24.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float checkboxWidth = 60.0f;
constexpr uint32_t defaultWidth = uint32_t(10.0 * knobX + 50.0);
constexpr uint32_t defaultHeight = uint32_t(20.0 + 3.0 * labelY + 6.0 * knobY);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::addOscillatorSection(
  std::string label,
  double left,
  double top,
  ParamID tagGain,
  ParamID tagSemi,
  ParamID tagCent,
  ParamID tagSync,
  ParamID tagSyncType,
  ParamID tagPTROrder,
  ParamID tagPhase,
  ParamID tagPhaseLock)
{
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  addGroupLabel(left, top, 2.0 * knobX, labelHeight, midTextSize, label);

  top += labelHeight + 10.0;
  auto knobCenterX = margin + left + knobX / 2.0;
  addKnob(knobCenterX, top, knobWidth, margin, uiTextSize, "Gain", tagGain);

  auto oscTop2 = top + knobY;
  auto knobLeft = margin + left;
  addNumberKnob(
    knobLeft, oscTop2, knobWidth, margin, uiTextSize, "Semi", tagSemi, Scales::semi);
  addKnob(knobLeft + knobX, oscTop2, knobWidth, margin, uiTextSize, "Cent", tagCent);

  auto oscTop3 = oscTop2 + knobY;
  auto syncKnobSize = 2.0 * knobHeight;
  auto oscMenuWidth = 2.0 * knobX;
  addKnob(
    left + (oscMenuWidth - syncKnobSize) / 2.0, oscTop3, syncKnobSize, margin, uiTextSize,
    "Sync", tagSync);

  auto oscTop4 = oscTop3 + syncKnobSize + labelY - 10.0;
  std::vector<std::string> syncOptions = {"Off", "Ratio", "Fixed-Master", "Fixed-Slave"};
  addOptionMenu(
    left, oscTop4, oscMenuWidth, labelHeight, uiTextSize, tagSyncType, syncOptions);

  auto oscTop5 = oscTop4 + labelY - margin;
  std::vector<std::string> ptrOrderOptions
    = {"Order 0",        "Order 1",        "Order 2",        "Order 3",
       "Order 4",        "Order 5",        "Order 6",        "Order 7",
       "Order 8",        "Order 9",        "Order 10",       "Sin",
       "Order 6 double", "Order 7 double", "Order 8 double", "Order 9 double",
       "Order 10 double"};
  addOptionMenu(
    left, oscTop5, oscMenuWidth, labelHeight, uiTextSize, tagPTROrder, ptrOrderOptions);

  auto oscTop6 = oscTop5 + labelY;
  addKnob(knobLeft, oscTop6, knobWidth, margin, uiTextSize, "Phase", tagPhase);
  addCheckbox(
    knobLeft + knobX, oscTop6 + (knobHeight - labelHeight) / 2.0, checkboxWidth,
    labelHeight, uiTextSize, "Lock", tagPhaseLock);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  // Oscillators.
  const auto oscWidth = 2.0 * knobWidth + 4.0 * margin;
  const auto oscTop = 10.0;
  const auto oscLeft1 = 10.0;
  addOscillatorSection(
    "Osc1", oscLeft1, oscTop, ID::osc1Gain, ID::osc1Semi, ID::osc1Cent, ID::osc1Sync,
    ID::osc1SyncType, ID::osc1PTROrder, ID::osc1Phase, ID::osc1PhaseLock);

  const auto oscLeft2 = oscLeft1 + oscWidth + 10.0;
  addOscillatorSection(
    "Osc2", oscLeft2, oscTop, ID::osc2Gain, ID::osc2Semi, ID::osc2Cent, ID::osc2Sync,
    ID::osc2SyncType, ID::osc2PTROrder, ID::osc2Phase, ID::osc2PhaseLock);

  std::vector<std::string> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(
    oscLeft2 - knobX / 2.0, oscTop + labelY, checkboxWidth, labelHeight, uiTextSize,
    ID::nVoice, nVoiceOptions);
  addCheckbox(
    oscLeft2 - knobX / 2.0, oscTop + 2.0 * labelY, checkboxWidth, labelHeight, uiTextSize,
    "Unison", ID::unison);

  const auto oscTop2 = 4.0 * labelY + 2.0 * knobY + 2.0 * knobHeight - margin;
  addCheckbox(
    oscLeft2 + margin + knobX, oscTop2 + labelY + (knobHeight - labelHeight) / 2.0 - 10.0,
    checkboxWidth, labelHeight, uiTextSize, "Invert", ID::osc2Invert);

  // Cross modulation.
  const auto crossTop = oscTop2 + knobY;
  const auto crossLeft = oscLeft1;
  addGroupLabel(
    crossLeft, crossTop, 2.0 * oscWidth + 10.0, labelHeight, midTextSize, "Modulation");

  const auto crossTop2 = crossTop + labelY;
  const auto crossKnobLeft = margin + crossLeft;
  addKnob(
    crossKnobLeft, crossTop2, 1.5 * knobWidth, margin, uiTextSize, "Osc1->Sync1",
    ID::fmOsc1ToSync1);
  addKnob(
    crossKnobLeft + 1.5 * knobWidth + 10.0, crossTop2, 1.5 * knobWidth, margin,
    uiTextSize, "Osc1->Freq2", ID::fmOsc1ToFreq2);
  addKnob(
    crossKnobLeft + 3.0 * knobWidth + 20.0, crossTop2, 1.5 * knobWidth, margin,
    uiTextSize, "Osc2->Sync1", ID::fmOsc2ToSync1);

  // Modulation envelope and LFO.
  const auto modTop = oscTop;
  const auto modLeft = oscLeft2 + oscWidth + 20.0;
  addGroupLabel(modLeft, modTop, 6.0 * knobX, labelHeight, midTextSize, "Modulation");

  const auto modTop1 = modTop + labelY;
  addKnob(modLeft, modTop1, knobWidth, margin, uiTextSize, "Attack", ID::modEnvelopeA);
  addKnob(
    modLeft + 1.0 * knobX, modTop1, knobWidth, margin, uiTextSize, "Curve",
    ID::modEnvelopeCurve);
  addKnob(
    modLeft + 2.0 * knobX, modTop1, knobWidth, margin, uiTextSize, "To Freq1",
    ID::modEnvelopeToFreq1);
  addKnob(
    modLeft + 3.0 * knobX, modTop1, knobWidth, margin, uiTextSize, "To Sync1",
    ID::modEnvelopeToSync1);
  addKnob(
    modLeft + 4.0 * knobX, modTop1, knobWidth, margin, uiTextSize, "To Freq2",
    ID::modEnvelopeToFreq2);
  addKnob(
    modLeft + 5.0 * knobX, modTop1, knobWidth, margin, uiTextSize, "To Sync2",
    ID::modEnvelopeToSync2);

  const auto modTop2 = modTop1 + knobY;
  addKnob(modLeft, modTop2, knobWidth, margin, uiTextSize, "LFO", ID::modLFOFrequency);
  addKnob(
    modLeft + 1.0 * knobX, modTop2, knobWidth, margin, uiTextSize, "NoiseMix",
    ID::modLFONoiseMix);
  addKnob(
    modLeft + 2.0 * knobX, modTop2, knobWidth, margin, uiTextSize, "To Freq1",
    ID::modLFOToFreq1);
  addKnob(
    modLeft + 3.0 * knobX, modTop2, knobWidth, margin, uiTextSize, "To Sync1",
    ID::modLFOToSync1);
  addKnob(
    modLeft + 4.0 * knobX, modTop2, knobWidth, margin, uiTextSize, "To Freq2",
    ID::modLFOToFreq2);
  addKnob(
    modLeft + 5.0 * knobX, modTop2, knobWidth, margin, uiTextSize, "To Sync2",
    ID::modLFOToSync2);

  // Gain.
  const auto gainTop = modTop2 + knobY + margin;
  const auto gainLeft = modLeft;
  addGroupLabel(gainLeft, gainTop, 6.0 * knobX, labelHeight, midTextSize, "Gain");

  const auto gainKnobTop = gainTop + labelY;
  addKnob(gainLeft, gainKnobTop, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(
    gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "A", ID::gainA);
  addKnob(
    gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "D", ID::gainD);
  addKnob(
    gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "S", ID::gainS);
  addKnob(
    gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "R", ID::gainR);
  addKnob(
    gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Curve",
    ID::gainEnvelopeCurve);

  // Filter.
  const auto filterTop = gainKnobTop + knobY + margin;
  const auto filterLeft = modLeft;
  addGroupLabel(
    filterLeft, filterTop, 4.0 * knobX - 10.0, labelHeight, midTextSize, "Filter");
  addCheckbox(
    filterLeft + 4.0 * knobX, filterTop, 1.5 * checkboxWidth, labelHeight, uiTextSize,
    "Dirty Buffer", ID::filterDirty);

  const auto filterTop1 = filterTop + labelY;
  addKnob(filterLeft, filterTop1, knobWidth, margin, uiTextSize, "Cut", ID::filterCutoff);
  addKnob(
    filterLeft + 1.0 * knobX, filterTop1, knobWidth, margin, uiTextSize, "Res",
    ID::filterResonance);
  addKnob(
    filterLeft + 2.0 * knobX, filterTop1, knobWidth, margin, uiTextSize, "Feed",
    ID::filterFeedback);
  addKnob(
    filterLeft + 3.0 * knobX, filterTop1, knobWidth, margin, uiTextSize, "Sat",
    ID::filterSaturation);

  const auto filterMenuWidth = 100.0;
  std::vector<std::string> filterTypeOptions = {"LP", "HP", "BP", "Notch", "Bypass"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1, filterMenuWidth, labelHeight, uiTextSize,
    ID::filterType, filterTypeOptions);
  std::vector<std::string> filterShaperOptions
    = {"HardClip", "Tanh", "ShaperA", "ShaperB"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1 + labelY, filterMenuWidth, labelHeight,
    uiTextSize, ID::filterShaper, filterShaperOptions);

  const auto filterTop2 = filterTop1 + knobY;
  addKnob(filterLeft, filterTop2, knobWidth, margin, uiTextSize, "A", ID::filterA);
  addKnob(
    filterLeft + 1.0 * knobX, filterTop2, knobWidth, margin, uiTextSize, "D",
    ID::filterD);
  addKnob(
    filterLeft + 2.0 * knobX, filterTop2, knobWidth, margin, uiTextSize, "S",
    ID::filterS);
  addKnob(
    filterLeft + 3.0 * knobX, filterTop2, knobWidth, margin, uiTextSize, "R",
    ID::filterR);
  addKnob(
    filterLeft + 4.0 * knobX, filterTop2, knobWidth, margin, uiTextSize, "To Cut",
    ID::filterCutoffAmount);
  addKnob(
    filterLeft + 5.0 * knobX, filterTop2, knobWidth, margin, uiTextSize, "To Res",
    ID::filterResonanceAmount);

  const auto filterTop3 = filterTop2 + knobY;
  addKnob(
    filterLeft, filterTop3, knobWidth, margin, uiTextSize, "Key->Cut",
    ID::filterKeyToCutoff);
  addKnob(
    filterLeft + 1.0 * knobX, filterTop3, knobWidth, margin, uiTextSize, "Key->Feed",
    ID::filterKeyToFeedback);

  // Plugin name.
  const auto splashTop = filterTop2 + knobY + 2.0 * margin;
  const auto splashLeft = modLeft + 2.0 * knobX;
  addSplashScreen(
    splashLeft + 0.25f * knobX, splashTop, 3.5f * knobX, 40.0f, 20.0f, 20.0f,
    defaultWidth - 40.0f, defaultHeight - 40.0f, pluginNameTextSize, "SyncSawSynth");

  return true;
}

} // namespace Vst
} // namespace Steinberg
