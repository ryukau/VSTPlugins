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

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  margin = 5.0;
  labelHeight = 20.0;
  labelY = 30.0;
  knobWidth = 50.0;
  knobHeight = 40.0;
  knobX = 60.0; // With margin.
  knobY = knobHeight + labelY;

  viewRect = ViewRect{0, 0, int32(10.0 * knobX + 50.0),
                      int32(20.0 + 3.0 * labelY + 6.0 * knobY)};
  setRect(viewRect);
}

void Editor::addOscillatorSection(
  UTF8String label,
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
  addGroupLabel(left, top, 2.0 * knobX, label);

  top += labelHeight + 10.0;
  auto knobCenterX = margin + left + knobX / 2.0;
  addKnob(knobCenterX, top, knobWidth, colorBlue, "Gain", tagGain);

  auto oscTop2 = top + knobY;
  auto knobLeft = margin + left;
  addNumberKnob(
    knobLeft, oscTop2, knobWidth, colorBlue, "Semi", tagSemi, Synth::Scales::semi, 0);
  addKnob(knobLeft + knobX, oscTop2, knobWidth, colorBlue, "Cent", tagCent);

  auto oscTop3 = oscTop2 + knobY;
  auto syncKnobSize = 2.0 * knobHeight;
  auto oscMenuWidth = 2.0 * knobX;
  addKnob(
    left + (oscMenuWidth - syncKnobSize) / 2.0, oscTop3, syncKnobSize, colorBlue, "Sync",
    tagSync);

  auto oscTop4 = oscTop3 + syncKnobSize + labelY - 10.0;
  std::vector<UTF8String> syncOptions = {"Off", "Ratio", "Fixed-Master", "Fixed-Slave"};
  addOptionMenu(left, oscTop4, oscMenuWidth, tagSyncType, syncOptions);

  auto oscTop5 = oscTop4 + labelY - margin;
  std::vector<UTF8String> ptrOrderOptions
    = {"Order 0",        "Order 1",        "Order 2",        "Order 3",
       "Order 4",        "Order 5",        "Order 6",        "Order 7",
       "Order 8",        "Order 9",        "Order 10",       "Sin",
       "Order 6 double", "Order 7 double", "Order 8 double", "Order 9 double",
       "Order 10 double"};
  addOptionMenu(left, oscTop5, oscMenuWidth, tagPTROrder, ptrOrderOptions);

  auto oscTop6 = oscTop5 + labelY;
  addKnob(knobLeft, oscTop6, knobWidth, colorBlue, "Phase", tagPhase);
  addCheckbox(
    knobLeft + knobX, oscTop6 + (knobHeight - labelHeight) / 2.0, 100.0, "Lock",
    tagPhaseLock);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

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

  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(oscLeft2 - knobX / 2.0, oscTop + labelY, 60.0, ID::nVoice, nVoiceOptions);
  addCheckbox(oscLeft2 - knobX / 2.0, oscTop + 2.0 * labelY, 100.0, "Unison", ID::unison);

  const auto oscTop2 = 4.0 * labelY + 2.0 * knobY + 2.0 * knobHeight - margin;
  addCheckbox(
    oscLeft2 + margin + knobX, oscTop2 + labelY + (knobHeight - labelHeight) / 2.0 - 10.0,
    100.0, "Invert", ID::osc2Invert);

  // Cross modulation.
  const auto crossTop = oscTop2 + knobY;
  const auto crossLeft = oscLeft1;
  addGroupLabel(crossLeft, crossTop, 2.0 * oscWidth + 10.0, "Modulation");

  const auto crossTop2 = crossTop + labelY;
  const auto crossKnobLeft = margin + crossLeft;
  addKnob(
    crossKnobLeft, crossTop2, 1.5 * knobWidth, colorBlue, "Osc1->Sync1",
    ID::fmOsc1ToSync1);
  addKnob(
    crossKnobLeft + 1.5 * knobWidth + 10.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc1->Freq2", ID::fmOsc1ToFreq2);
  addKnob(
    crossKnobLeft + 3.0 * knobWidth + 20.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc2->Sync1", ID::fmOsc2ToSync1);

  // Modulation envelope and LFO.
  const auto modTop = oscTop;
  const auto modLeft = oscLeft2 + oscWidth + 20.0;
  addGroupLabel(modLeft, modTop, 6.0 * knobX, "Modulation");

  const auto modTop1 = modTop + labelY;
  addKnob(modLeft, modTop1, knobWidth, colorBlue, "Attack", ID::modEnvelopeA);
  addKnob(
    modLeft + 1.0 * knobX, modTop1, knobWidth, colorBlue, "Curve", ID::modEnvelopeCurve);
  addKnob(
    modLeft + 2.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq1",
    ID::modEnvelopeToFreq1);
  addKnob(
    modLeft + 3.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync1",
    ID::modEnvelopeToSync1);
  addKnob(
    modLeft + 4.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq2",
    ID::modEnvelopeToFreq2);
  addKnob(
    modLeft + 5.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync2",
    ID::modEnvelopeToSync2);

  const auto modTop2 = modTop1 + knobY;
  addKnob(modLeft, modTop2, knobWidth, colorBlue, "LFO", ID::modLFOFrequency);
  addKnob(
    modLeft + 1.0 * knobX, modTop2, knobWidth, colorBlue, "NoiseMix", ID::modLFONoiseMix);
  addKnob(
    modLeft + 2.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq1", ID::modLFOToFreq1);
  addKnob(
    modLeft + 3.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync1", ID::modLFOToSync1);
  addKnob(
    modLeft + 4.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq2", ID::modLFOToFreq2);
  addKnob(
    modLeft + 5.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync2", ID::modLFOToSync2);

  // Gain.
  const auto gainTop = modTop2 + knobY + margin;
  const auto gainLeft = modLeft;
  addGroupLabel(gainLeft, gainTop, 6.0 * knobX, "Gain");

  const auto gainKnobTop = gainTop + labelY;
  addKnob(gainLeft, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA);
  addKnob(gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD);
  addKnob(gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS);
  addKnob(gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR);
  addKnob(
    gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
    ID::gainEnvelopeCurve);

  // Filter.
  const auto filterTop = gainKnobTop + knobY + margin;
  const auto filterLeft = modLeft;
  addGroupLabel(filterLeft, filterTop, 4.0 * knobX - 10.0, "Filter");
  addCheckbox(
    filterLeft + 4.0 * knobX, filterTop, 100.0, "Dirty Buffer", ID::filterDirty);

  const auto filterTop1 = filterTop + labelY;
  addKnob(filterLeft, filterTop1, knobWidth, colorBlue, "Cut", ID::filterCutoff);
  addKnob(
    filterLeft + 1.0 * knobX, filterTop1, knobWidth, colorBlue, "Res",
    ID::filterResonance);
  addKnob(
    filterLeft + 2.0 * knobX, filterTop1, knobWidth, colorBlue, "Feed",
    ID::filterFeedback);
  addKnob(
    filterLeft + 3.0 * knobX, filterTop1, knobWidth, colorBlue, "Sat",
    ID::filterSaturation);

  const auto filterMenuWidth = 100.0;
  std::vector<UTF8String> filterTypeOptions = {"LP", "HP", "BP", "Notch", "Bypass"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1, filterMenuWidth, ID::filterType,
    filterTypeOptions);
  std::vector<UTF8String> filterShaperOptions
    = {"HardClip", "Tanh", "ShaperA", "ShaperB"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1 + labelY, filterMenuWidth, ID::filterShaper,
    filterShaperOptions);

  const auto filterTop2 = filterTop1 + knobY;
  addKnob(filterLeft, filterTop2, knobWidth, colorBlue, "A", ID::filterA);
  addKnob(filterLeft + 1.0 * knobX, filterTop2, knobWidth, colorBlue, "D", ID::filterD);
  addKnob(filterLeft + 2.0 * knobX, filterTop2, knobWidth, colorBlue, "S", ID::filterS);
  addKnob(filterLeft + 3.0 * knobX, filterTop2, knobWidth, colorBlue, "R", ID::filterR);
  addKnob(
    filterLeft + 4.0 * knobX, filterTop2, knobWidth, colorBlue, "To Cut",
    ID::filterCutoffAmount);
  addKnob(
    filterLeft + 5.0 * knobX, filterTop2, knobWidth, colorBlue, "To Res",
    ID::filterResonanceAmount);

  const auto filterTop3 = filterTop2 + knobY;
  addKnob(
    filterLeft, filterTop3, knobWidth, colorBlue, "Key->Cut", ID::filterKeyToCutoff);
  addKnob(
    filterLeft + 1.0 * knobX, filterTop3, knobWidth, colorBlue, "Key->Feed",
    ID::filterKeyToFeedback);

  // Plugin name.
  const auto splashTop = filterTop2 + knobY + 2.0 * margin;
  const auto splashLeft = modLeft + 2.0 * knobX;
  addSplashScreen(
    splashLeft + 0.25 * knobX, splashTop, 3.75 * knobX, 40.0, 20.0, 20.0,
    viewRect.right - 40.0, viewRect.bottom - 40.0, "SyncSawSynth", 24.0);

  return true;
}

} // namespace Vst
} // namespace Steinberg
