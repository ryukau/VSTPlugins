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

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 22.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = 40.0f;
  knobX = 60.0f; // With margin.
  knobY = knobHeight + labelY;
  checkboxWidth = 60.0f;
  splashHeight = 40.0f;
  defaultWidth = int32(12 * knobX + 4 * margin + 40);
  defaultHeight = int32(40 + 10 * labelY + 2 * knobY + 1 * knobHeight + 2 * margin);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const float top0 = 20.0f;
  const float left0 = 20.0f;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0 + knobX;
  addGroupLabel(gainLeft, gainTop, 7.0 * knobX, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Boost", ID::gainBoost);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain);
  addKnob(gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA);
  addKnob(gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD);
  addKnob(gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS);
  addKnob(gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR);
  addKnob(
    gainLeft + 6.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
    ID::gainEnvelopeCurve);

  // Random.
  const float randomTop0 = top0;
  const float randomLeft0 = left0 + 8.0f * knobX + 4.0f * margin;
  const float randomLeft1 = randomLeft0 + knobX;
  const float randomLeft2 = randomLeft1 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 4.0f * knobX, "Random");
  addCheckbox(
    randomLeft0 + 8.0f, randomTop0 + labelY - 2.0f * margin, knobX, "Retrigger",
    ID::randomRetrigger);

  const float randomTop1 = randomTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2.0f * margin, "Seed");
  addTextKnob(
    randomLeft1 - 2.0f * margin, randomTop1, knobX, colorBlue, ID::seed, Scales::seed);
  addKnob(
    randomLeft2, randomTop0 + labelY, knobWidth, colorBlue, "To Gain",
    ID::randomGainAmount);
  addKnob(
    randomLeft2 + knobX, randomTop0 + labelY, knobWidth, colorBlue, "To Pitch",
    ID::randomFrequencyAmount);

  // Shelving.
  const auto filterTop0 = gainTop + knobY + labelY;
  const auto filterLeft0 = left0;
  const auto filterLeft1 = filterLeft0 + knobX;
  const auto filterLeft2 = filterLeft1 + knobX;
  const auto filterTop1 = filterTop0 + labelY - 2.0f * margin;
  const auto filterTop2 = filterTop1 + labelY - margin;
  const auto filterTop3 = filterTop2 + labelY;
  addGroupLabel(filterLeft1, filterTop0, 2.0f * knobX, "Shelving");
  addLabel(filterLeft0, filterTop2, knobX, "Gain [dB]");
  addLabel(filterLeft0, filterTop3, knobX, "Semi");

  addLabel(filterLeft1, filterTop1, knobX, "Low");
  addTextKnob(
    filterLeft1, filterTop2, knobX, colorBlue, ID::lowShelfGain, Scales::shelvingGain,
    true, 2);
  addTextKnob(
    filterLeft1, filterTop3, knobX, colorBlue, ID::lowShelfPitch, Scales::shelvingPitch,
    false, 3);

  addLabel(filterLeft2, filterTop1, knobX, "High");
  addTextKnob(
    filterLeft2, filterTop2, knobX, colorBlue, ID::highShelfGain, Scales::shelvingGain,
    true, 2);
  addTextKnob(
    filterLeft2, filterTop3, knobX, colorBlue, ID::highShelfPitch, Scales::shelvingPitch,
    false, 3);

  // Pitch.
  const auto pitchTop0 = top0 + knobY + labelY;
  const auto pitchLeft0 = left0 + 3.0f * knobX + 4.0f * margin;
  addGroupLabel(pitchLeft0, pitchTop0, 4.0f * knobX, "Pitch");
  addCheckbox(
    pitchLeft0 + 0.0f * knobX, pitchTop0 + labelY - 2.0f * margin, knobX, "Add Aliasing",
    ID::aliasing);
  addCheckbox(
    pitchLeft0 + 1.8f * knobX, pitchTop0 + labelY - 2.0f * margin, knobX, "Reverse Semi",
    ID::negativeSemi);

  const auto pitchTop1 = pitchTop0 + 2.0f * labelY - 3.0f * margin;
  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(pitchLeft0 + margin, pitchTop1, knobX - 2.0f * margin, "Octave");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop1, knobX, colorBlue, ID::masterOctave,
    Scales::masterOctave);
  addLabel(pitchLeft0 + margin, pitchTop2, knobX - 2.0f * margin, "ET");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop2, knobX, colorBlue, ID::equalTemperament,
    Scales::equalTemperament);

  const auto pitchLeft1 = pitchLeft0 + 2.1f * knobX;
  addLabel(pitchLeft1 + margin, pitchTop1, knobX - 2.0f * margin, "Multiply");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop1, knobX, colorBlue, ID::pitchMultiply,
    Scales::pitchMultiply, false, 3);

  addLabel(pitchLeft1 + margin, pitchTop2, knobX - 2.0f * margin, "Modulo");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop2, knobX, colorBlue, ID::pitchModulo, Scales::pitchModulo,
    false, 3);

  // Misc.
  const auto miscLeft = pitchLeft0 + 4.5f * knobX + 2.0f * margin;
  addKnob(miscLeft, pitchTop0, knobWidth, colorBlue, "Smooth", ID::smoothness);
  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(
    miscLeft - (checkboxWidth - knobWidth) / 2.0f, pitchTop0 + knobY, checkboxWidth,
    ID::nVoice, nVoiceOptions);

  // Chorus.
  const float chorusTop0 = filterTop0;
  const float chorusLeft0 = randomLeft0 + knobX;
  const float chorusLeft1 = chorusLeft0 + knobX;
  const float chorusLeft2 = chorusLeft1 + knobX;
  addGroupLabel(chorusLeft0, chorusTop0, 3.0f * knobX, "Chorus");

  const float chorusTop1 = chorusTop0 + labelY;
  addKnob(chorusLeft0, chorusTop1, knobWidth, colorBlue, "Mix", ID::chorusMix);
  addKnob(chorusLeft1, chorusTop1, knobWidth, colorBlue, "Freq", ID::chorusFrequency);
  addKnob(chorusLeft2, chorusTop1, knobWidth, colorBlue, "Depth", ID::chorusDepth);

  const float chorusTop2 = chorusTop1 + knobY;
  addKnob(
    chorusLeft0, chorusTop2, knobWidth, colorBlue, "Range0", ID::chorusDelayTimeRange0);
  addKnob(
    chorusLeft1, chorusTop2, knobWidth, colorBlue, "Range1", ID::chorusDelayTimeRange1);
  addKnob(
    chorusLeft2, chorusTop2, knobWidth, colorBlue, "Range2", ID::chorusDelayTimeRange2);

  const float chorusTop3 = chorusTop2 + knobY;
  addKnob(
    chorusLeft0, chorusTop3, knobWidth, colorBlue, "Time0", ID::chorusMinDelayTime0);
  addKnob(
    chorusLeft1, chorusTop3, knobWidth, colorBlue, "Time1", ID::chorusMinDelayTime1);
  addKnob(
    chorusLeft2, chorusTop3, knobWidth, colorBlue, "Time2", ID::chorusMinDelayTime2);

  const float chorusTop4 = chorusTop3 + knobY;
  addRotaryKnob(chorusLeft0, chorusTop4, knobWidth, colorBlue, "Phase", ID::chorusPhase);
  addKnob(chorusLeft1, chorusTop4, knobWidth, colorBlue, "Offset", ID::chorusOffset);
  addKnob(chorusLeft2, chorusTop4, knobWidth, colorBlue, "Feedback", ID::chorusFeedback);

  const float chorusTop5 = chorusTop4 + knobY;
  addCheckbox(
    chorusLeft0 + 0.8f * knobX, chorusTop5 + margin, checkboxWidth, "Key Follow",
    ID::chorusKeyFollow);

  // Note.
  const float noteTop0 = filterTop0 + knobY + labelY;
  addGroupLabel(left0 + knobX, noteTop0, 8.0f * knobX, "Note");

  const float noteTop1 = noteTop0 + labelY;
  addLabel(left0, noteTop1, knobX, "Gain [dB]");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop1, knobX, colorBlue, ID::gain0, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop1, knobX, colorBlue, ID::gain1, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop1, knobX, colorBlue, ID::gain2, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop1, knobX, colorBlue, ID::gain3, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop1, knobX, colorBlue, ID::gain4, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop1, knobX, colorBlue, ID::gain5, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop1, knobX, colorBlue, ID::gain6, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop1, knobX, colorBlue, ID::gain7, Scales::gainDecibel,
    true, 2);

  const float noteTop2 = noteTop1 + labelY;
  addLabel(left0, noteTop2, knobX, "Semi");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop2, knobX, colorBlue, ID::semi0, Scales::oscSemi);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop2, knobX, colorBlue, ID::semi1, Scales::oscSemi);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop2, knobX, colorBlue, ID::semi2, Scales::oscSemi);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop2, knobX, colorBlue, ID::semi3, Scales::oscSemi);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop2, knobX, colorBlue, ID::semi4, Scales::oscSemi);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop2, knobX, colorBlue, ID::semi5, Scales::oscSemi);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop2, knobX, colorBlue, ID::semi6, Scales::oscSemi);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop2, knobX, colorBlue, ID::semi7, Scales::oscSemi);

  const float noteTop3 = noteTop2 + labelY;
  addLabel(left0, noteTop3, knobX, "Milli");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop3, knobX, colorBlue, ID::milli0, Scales::oscMilli);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop3, knobX, colorBlue, ID::milli1, Scales::oscMilli);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop3, knobX, colorBlue, ID::milli2, Scales::oscMilli);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop3, knobX, colorBlue, ID::milli3, Scales::oscMilli);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop3, knobX, colorBlue, ID::milli4, Scales::oscMilli);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop3, knobX, colorBlue, ID::milli5, Scales::oscMilli);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop3, knobX, colorBlue, ID::milli6, Scales::oscMilli);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop3, knobX, colorBlue, ID::milli7, Scales::oscMilli);

  // Chord.
  const float topChord0 = noteTop0 + 4.0f * labelY;
  addGroupLabel(left0 + knobX, topChord0, 4.0f * knobX, "Chord");

  const float topChord1 = topChord0 + labelY;
  addLabel(left0, topChord1, knobX, "Gain [dB]");
  addTextKnob(
    left0 + 1.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain0,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 2.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain1,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 3.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain2,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 4.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain3,
    Scales::gainDecibel, true, 2);

  const float topChord2 = topChord1 + labelY;
  addLabel(left0, topChord2, knobX, "Semi");
  addTextKnob(
    left0 + 1.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi0, Scales::oscSemi);
  addTextKnob(
    left0 + 2.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi1, Scales::oscSemi);
  addTextKnob(
    left0 + 3.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi2, Scales::oscSemi);
  addTextKnob(
    left0 + 4.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi3, Scales::oscSemi);

  const float topChord3 = topChord2 + labelY;
  addLabel(left0, topChord3, knobX, "Milli");
  addTextKnob(
    left0 + 1.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli0, Scales::oscMilli);
  addTextKnob(
    left0 + 2.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli1, Scales::oscMilli);
  addTextKnob(
    left0 + 3.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli2, Scales::oscMilli);
  addTextKnob(
    left0 + 4.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli3, Scales::oscMilli);

  const float topChord4 = topChord3 + labelY;
  addLabel(left0, topChord4 + (knobX - labelY) / 2, knobX, "Pan");
  addKnob(
    left0 + 1.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan0);
  addKnob(
    left0 + 2.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan1);
  addKnob(
    left0 + 3.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan2);
  addKnob(
    left0 + 4.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan3);

  // Overtone.
  const float topOvertone0 = topChord0;
  const float leftOvertone = left0 + 5.0f * knobX + 2.0f * margin;
  addGroupLabel(leftOvertone, topOvertone0, 4.0f * knobX, "Overtone");

  addBarBox(
    leftOvertone, topOvertone0 + labelY, 4.0f * knobX, 2.0f * knobY, ID::overtone1, 16,
    "");

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 20.0f;
  const auto splashLeft = defaultWidth - 3.0f * knobX - 15.0f;
  addSplashScreen(
    splashLeft, splashTop, 3.0f * knobX, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, "IterativeSinCluster");
  return true;
}

} // namespace Vst
} // namespace Steinberg
