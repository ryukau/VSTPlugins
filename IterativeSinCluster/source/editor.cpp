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

  const float top0 = uiMargin;
  const float left0 = uiMargin;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0 + knobX;
  addGroupLabel(gainLeft, gainTop, 7 * knobX, labelHeight, midTextSize, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Boost",
    ID::gainBoost);
  addKnob(
    gainLeft + 1 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Gain", ID::gain);
  addKnob(
    gainLeft + 2 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "A", ID::gainA);
  addKnob(
    gainLeft + 3 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "D", ID::gainD);
  addKnob(
    gainLeft + 4 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "S", ID::gainS);
  addKnob(
    gainLeft + 5 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "R", ID::gainR);
  addKnob(
    gainLeft + 6 * knobX, gainKnobTop, knobWidth, margin, uiTextSize, "Curve",
    ID::gainEnvelopeCurve);

  // Random.
  const float randomTop0 = top0;
  const float randomLeft0 = left0 + 8 * knobX + 4 * margin;
  const float randomLeft1 = randomLeft0 + knobX;
  const float randomLeft2 = randomLeft1 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 4 * knobX, labelHeight, midTextSize, "Random");
  addCheckbox(
    randomLeft0 + 8, randomTop0 + labelY - 2 * margin, int(1.5 * knobX), labelHeight,
    uiTextSize, "Retrigger", ID::randomRetrigger);

  const float randomTop1 = randomTop0 + 2 * labelY - 3 * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2 * margin, labelHeight, uiTextSize, "Seed");
  addTextKnob(
    randomLeft1 - 2 * margin, randomTop1, knobX, labelHeight, uiTextSize, ID::seed,
    Scales::seed);
  addKnob(
    randomLeft2, randomTop0 + labelY, knobWidth, margin, uiTextSize, "To Gain",
    ID::randomGainAmount);
  addKnob(
    randomLeft2 + knobX, randomTop0 + labelY, knobWidth, margin, uiTextSize, "To Pitch",
    ID::randomFrequencyAmount);

  // Shelving.
  const auto filterTop0 = gainTop + knobY + labelY;
  const auto filterLeft0 = left0;
  const auto filterLeft1 = filterLeft0 + knobX;
  const auto filterLeft2 = filterLeft1 + knobX;
  const auto filterTop1 = filterTop0 + labelY - 2 * margin;
  const auto filterTop2 = filterTop1 + labelY - margin;
  const auto filterTop3 = filterTop2 + labelY;
  addGroupLabel(filterLeft1, filterTop0, 2 * knobX, labelHeight, midTextSize, "Shelving");
  addLabel(filterLeft0, filterTop2, knobX, labelHeight, uiTextSize, "Gain [dB]");
  addLabel(filterLeft0, filterTop3, knobX, labelHeight, uiTextSize, "Semi");

  addLabel(filterLeft1, filterTop1, knobX, labelHeight, uiTextSize, "Low");
  addTextKnob(
    filterLeft1, filterTop2, knobX, labelHeight, uiTextSize, ID::lowShelfGain,
    Scales::shelvingGain, true, 2);
  addTextKnob(
    filterLeft1, filterTop3, knobX, labelHeight, uiTextSize, ID::lowShelfPitch,
    Scales::shelvingPitch, false, 3);

  addLabel(filterLeft2, filterTop1, knobX, labelHeight, uiTextSize, "High");
  addTextKnob(
    filterLeft2, filterTop2, knobX, labelHeight, uiTextSize, ID::highShelfGain,
    Scales::shelvingGain, true, 2);
  addTextKnob(
    filterLeft2, filterTop3, knobX, labelHeight, uiTextSize, ID::highShelfPitch,
    Scales::shelvingPitch, false, 3);

  // Pitch.
  const auto pitchTop0 = top0 + knobY + labelY;
  const auto pitchLeft0 = left0 + 3 * knobX + 4 * margin;
  addGroupLabel(pitchLeft0, pitchTop0, 4 * knobX, labelHeight, midTextSize, "Pitch");
  addCheckbox(
    pitchLeft0 + 0 * knobX, pitchTop0 + labelY - 2 * margin, int(1.5 * knobX),
    labelHeight, uiTextSize, "Add Aliasing", ID::aliasing);
  addCheckbox(
    pitchLeft0 + int(1.8 * knobX), pitchTop0 + labelY - 2 * margin, int(1.75 * knobX),
    labelHeight, uiTextSize, "Reverse Semi", ID::negativeSemi);

  const auto pitchTop1 = pitchTop0 + 2 * labelY - 3 * margin;
  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(
    pitchLeft0 + margin, pitchTop1, knobX - 2 * margin, labelHeight, uiTextSize,
    "Octave");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop1, knobX, labelHeight, uiTextSize, ID::masterOctave,
    Scales::masterOctave);
  addLabel(
    pitchLeft0 + margin, pitchTop2, knobX - 2 * margin, labelHeight, uiTextSize, "ET");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop2, knobX, labelHeight, uiTextSize, ID::equalTemperament,
    Scales::equalTemperament);

  const auto pitchLeft1 = pitchLeft0 + int(2.1f * knobX);
  addLabel(
    pitchLeft1 + margin, pitchTop1, knobX - 2 * margin, labelHeight, uiTextSize,
    "Multiply");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop1, knobX, labelHeight, uiTextSize, ID::pitchMultiply,
    Scales::pitchMultiply, false, 3);

  addLabel(
    pitchLeft1 + margin, pitchTop2, knobX - 2 * margin, labelHeight, uiTextSize,
    "Modulo");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop2, knobX, labelHeight, uiTextSize, ID::pitchModulo,
    Scales::pitchModulo, false, 3);

  // Misc.
  const auto miscLeft = pitchLeft0 + int(4.5f * knobX) + 2 * margin;
  addKnob(miscLeft, pitchTop0, knobWidth, margin, uiTextSize, "Smooth", ID::smoothness);
  std::vector<std::string> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(
    int(miscLeft - (checkboxWidth - knobWidth) / 2.0), pitchTop0 + knobY, checkboxWidth,
    labelHeight, uiTextSize, ID::nVoice, nVoiceOptions);

  // Chorus.
  const float chorusTop0 = filterTop0;
  const float chorusLeft0 = randomLeft0 + knobX;
  const float chorusLeft1 = chorusLeft0 + knobX;
  const float chorusLeft2 = chorusLeft1 + knobX;
  addGroupLabel(chorusLeft0, chorusTop0, 3 * knobX, labelHeight, midTextSize, "Chorus");

  const float chorusTop1 = chorusTop0 + labelY;
  addKnob(chorusLeft0, chorusTop1, knobWidth, margin, uiTextSize, "Mix", ID::chorusMix);
  addKnob(
    chorusLeft1, chorusTop1, knobWidth, margin, uiTextSize, "Freq", ID::chorusFrequency);
  addKnob(
    chorusLeft2, chorusTop1, knobWidth, margin, uiTextSize, "Depth", ID::chorusDepth);

  const float chorusTop2 = chorusTop1 + knobY;
  addKnob(
    chorusLeft0, chorusTop2, knobWidth, margin, uiTextSize, "Range0",
    ID::chorusDelayTimeRange0);
  addKnob(
    chorusLeft1, chorusTop2, knobWidth, margin, uiTextSize, "Range1",
    ID::chorusDelayTimeRange1);
  addKnob(
    chorusLeft2, chorusTop2, knobWidth, margin, uiTextSize, "Range2",
    ID::chorusDelayTimeRange2);

  const float chorusTop3 = chorusTop2 + knobY;
  addKnob(
    chorusLeft0, chorusTop3, knobWidth, margin, uiTextSize, "Time0",
    ID::chorusMinDelayTime0);
  addKnob(
    chorusLeft1, chorusTop3, knobWidth, margin, uiTextSize, "Time1",
    ID::chorusMinDelayTime1);
  addKnob(
    chorusLeft2, chorusTop3, knobWidth, margin, uiTextSize, "Time2",
    ID::chorusMinDelayTime2);

  const float chorusTop4 = chorusTop3 + knobY;
  addRotaryKnob(
    chorusLeft0, chorusTop4, knobWidth, margin, uiTextSize, "Phase", ID::chorusPhase);
  addKnob(
    chorusLeft1, chorusTop4, knobWidth, margin, uiTextSize, "Offset", ID::chorusOffset);
  addKnob(
    chorusLeft2, chorusTop4, knobWidth, margin, uiTextSize, "Feedback",
    ID::chorusFeedback);

  const float chorusTop5 = chorusTop4 + knobY;
  addCheckbox(
    chorusLeft0 + int(0.8 * knobX), chorusTop5 + margin, int(1.5 * checkboxWidth),
    labelHeight, uiTextSize, "Key Follow", ID::chorusKeyFollow);

  // Note.
  const float noteTop0 = filterTop0 + knobY + labelY;
  addGroupLabel(left0 + knobX, noteTop0, 8 * knobX, labelHeight, midTextSize, "Note");

  const float noteTop1 = noteTop0 + labelY;
  addLabel(left0, noteTop1, knobX, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    left0 + 1 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain0,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 2 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain1,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 3 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain2,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 4 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain3,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 5 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain4,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 6 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain5,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 7 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain6,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 8 * knobX, noteTop1, knobX, labelHeight, uiTextSize, ID::gain7,
    Scales::gainDecibel, true, 2);

  const float noteTop2 = noteTop1 + labelY;
  addLabel(left0, noteTop2, knobX, labelHeight, uiTextSize, "Semi");
  addTextKnob(
    left0 + 1 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi0,
    Scales::oscSemi);
  addTextKnob(
    left0 + 2 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi1,
    Scales::oscSemi);
  addTextKnob(
    left0 + 3 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi2,
    Scales::oscSemi);
  addTextKnob(
    left0 + 4 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi3,
    Scales::oscSemi);
  addTextKnob(
    left0 + 5 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi4,
    Scales::oscSemi);
  addTextKnob(
    left0 + 6 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi5,
    Scales::oscSemi);
  addTextKnob(
    left0 + 7 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi6,
    Scales::oscSemi);
  addTextKnob(
    left0 + 8 * knobX, noteTop2, knobX, labelHeight, uiTextSize, ID::semi7,
    Scales::oscSemi);

  const float noteTop3 = noteTop2 + labelY;
  addLabel(left0, noteTop3, knobX, labelHeight, uiTextSize, "Milli");
  addTextKnob(
    left0 + 1 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli0,
    Scales::oscMilli);
  addTextKnob(
    left0 + 2 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli1,
    Scales::oscMilli);
  addTextKnob(
    left0 + 3 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli2,
    Scales::oscMilli);
  addTextKnob(
    left0 + 4 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli3,
    Scales::oscMilli);
  addTextKnob(
    left0 + 5 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli4,
    Scales::oscMilli);
  addTextKnob(
    left0 + 6 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli5,
    Scales::oscMilli);
  addTextKnob(
    left0 + 7 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli6,
    Scales::oscMilli);
  addTextKnob(
    left0 + 8 * knobX, noteTop3, knobX, labelHeight, uiTextSize, ID::milli7,
    Scales::oscMilli);

  // Chord.
  const float topChord0 = noteTop0 + 4 * labelY;
  addGroupLabel(left0 + knobX, topChord0, 4 * knobX, labelHeight, midTextSize, "Chord");

  const float topChord1 = topChord0 + labelY;
  addLabel(left0, topChord1, knobX, labelHeight, uiTextSize, "Gain [dB]");
  addTextKnob(
    left0 + 1 * knobX, topChord1, knobX, labelHeight, uiTextSize, ID::chordGain0,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 2 * knobX, topChord1, knobX, labelHeight, uiTextSize, ID::chordGain1,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 3 * knobX, topChord1, knobX, labelHeight, uiTextSize, ID::chordGain2,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 4 * knobX, topChord1, knobX, labelHeight, uiTextSize, ID::chordGain3,
    Scales::gainDecibel, true, 2);

  const float topChord2 = topChord1 + labelY;
  addLabel(left0, topChord2, knobX, labelHeight, uiTextSize, "Semi");
  addTextKnob(
    left0 + 1 * knobX, topChord2, knobX, labelHeight, uiTextSize, ID::chordSemi0,
    Scales::oscSemi);
  addTextKnob(
    left0 + 2 * knobX, topChord2, knobX, labelHeight, uiTextSize, ID::chordSemi1,
    Scales::oscSemi);
  addTextKnob(
    left0 + 3 * knobX, topChord2, knobX, labelHeight, uiTextSize, ID::chordSemi2,
    Scales::oscSemi);
  addTextKnob(
    left0 + 4 * knobX, topChord2, knobX, labelHeight, uiTextSize, ID::chordSemi3,
    Scales::oscSemi);

  const float topChord3 = topChord2 + labelY;
  addLabel(left0, topChord3, knobX, labelHeight, uiTextSize, "Milli");
  addTextKnob(
    left0 + 1 * knobX, topChord3, knobX, labelHeight, uiTextSize, ID::chordMilli0,
    Scales::oscMilli);
  addTextKnob(
    left0 + 2 * knobX, topChord3, knobX, labelHeight, uiTextSize, ID::chordMilli1,
    Scales::oscMilli);
  addTextKnob(
    left0 + 3 * knobX, topChord3, knobX, labelHeight, uiTextSize, ID::chordMilli2,
    Scales::oscMilli);
  addTextKnob(
    left0 + 4 * knobX, topChord3, knobX, labelHeight, uiTextSize, ID::chordMilli3,
    Scales::oscMilli);

  const float topChord4 = topChord3 + labelY;
  addLabel(
    left0, int(topChord4 + (knobX - labelY) / 2.0), knobX, labelHeight, uiTextSize,
    "Pan");
  addKnob(
    left0 + 1 * knobX, topChord4 - margin, knobX, margin, uiTextSize, "", ID::chordPan0);
  addKnob(
    left0 + 2 * knobX, topChord4 - margin, knobX, margin, uiTextSize, "", ID::chordPan1);
  addKnob(
    left0 + 3 * knobX, topChord4 - margin, knobX, margin, uiTextSize, "", ID::chordPan2);
  addKnob(
    left0 + 4 * knobX, topChord4 - margin, knobX, margin, uiTextSize, "", ID::chordPan3);

  // Overtone.
  const float topOvertone0 = topChord0;
  const float leftOvertone = left0 + 5 * knobX + 2 * margin;
  addGroupLabel(
    leftOvertone, topOvertone0, 4 * knobX, labelHeight, midTextSize, "Overtone");
  addBarBox(
    leftOvertone, topOvertone0 + labelY, 4 * knobX, 2 * knobY, ID::overtone1, 16,
    Scales::gainDecibel, "Overtone");

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashLeft = defaultWidth - 3 * knobX - 3 * margin;
  addSplashScreen(
    splashLeft, splashTop, 3 * knobX, splashHeight, uiMargin, uiMargin,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "IterativeSinCluster");

  return true;
}

} // namespace Vst
} // namespace Steinberg
