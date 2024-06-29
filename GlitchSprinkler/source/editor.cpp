// (c) 2021-2023 Takamitsu Endo
//
// This file is part of GlitchSprinkler.
//
// GlitchSprinkler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GlitchSprinkler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlitchSprinkler.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "gui/randomizebutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 16.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float knobWidth = 80.0f;
constexpr float knobX = knobWidth + 2 * margin;
constexpr float knobY = knobWidth + labelHeight + 2 * margin;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 2 * knobWidth;
constexpr float groupLabelWidth = 2 * labelWidth + 2 * margin;
constexpr float splashWidth = int(labelWidth * 3 / 2) + 2 * margin;
constexpr float splashHeight = int(2 * labelHeight + 2 * margin);

constexpr float barBoxWidth = groupLabelWidth;
constexpr float barBoxHeight = 5 * labelY - 2 * margin;
constexpr float smallKnobWidth = labelHeight;
constexpr float smallKnobX = smallKnobWidth + 2 * margin;

constexpr float tabViewWidth = 2 * groupLabelWidth + 4 * margin + 2 * uiMargin;
constexpr float tabViewHeight = 20 * labelY - 2 * margin + 2 * uiMargin;

constexpr int_least32_t defaultWidth = int_least32_t(4 * uiMargin + 3 * groupLabelWidth);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 20 * labelY - 2 * margin);

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

void Editor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);

  using ID = Synth::ParameterID::ID;
  if (id >= ID::polynomialPointX0 && id < ID::polynomialPointY0 + nPolyOscControl) {
    polyXYPad->linkControlFromId(id);
  }
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;
  constexpr auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  constexpr auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  // Mix.
  constexpr auto mixTop0 = top0;
  constexpr auto mixTop1 = mixTop0 + 1 * labelY;
  constexpr auto mixTop2 = mixTop0 + 2 * labelY;
  constexpr auto mixTop3 = mixTop0 + 3 * labelY;
  constexpr auto mixTop4 = mixTop0 + 4 * labelY;
  constexpr auto mixTop5 = mixTop0 + 5 * labelY;
  constexpr auto mixTop6 = mixTop0 + 6 * labelY;
  constexpr auto mixTop7 = mixTop0 + 7 * labelY;
  constexpr auto mixTop8 = mixTop0 + 8 * labelY;
  constexpr auto mixTop9 = mixTop0 + 9 * labelY;
  constexpr auto mixTop10 = mixTop0 + 10 * labelY;
  constexpr auto mixTop11 = mixTop0 + 11 * labelY;
  constexpr auto mixTop12 = mixTop0 + 12 * labelY;
  constexpr auto mixLeft0 = left0;
  constexpr auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Mix & Options");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);

  addLabel(mixLeft0, mixTop3, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, ID::decaySeconds,
    Scales::decaySeconds, false, 5);
  addLabel(mixLeft0, mixTop4, labelWidth, labelHeight, uiTextSize, "Osc. Sync.");
  addTextKnob(
    mixLeft1, mixTop4, labelWidth, labelHeight, uiTextSize, ID::oscSync,
    Scales::defaultScale, false, 5);
  addLabel(mixLeft0, mixTop5, labelWidth, labelHeight, uiTextSize, "FM Index");
  addTextKnob(
    mixLeft1, mixTop5, labelWidth, labelHeight, uiTextSize, ID::fmIndex, Scales::fmIndex,
    false, 5);
  addLabel(mixLeft0, mixTop6, labelWidth, labelHeight, uiTextSize, "Saturation [dB]");
  addTextKnob(
    mixLeft1, mixTop6, labelWidth, labelHeight, uiTextSize, ID::saturationGain,
    Scales::gain, true, 5);

  addLabel(mixLeft0, mixTop7, labelWidth, labelHeight, uiTextSize, "Random FM Index");
  addTextKnob(
    mixLeft1, mixTop7, labelWidth, labelHeight, uiTextSize, ID::randomizeFmIndex,
    Scales::randomizeFmIndex, false, 5);

  addLabel(mixLeft0, mixTop8, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    mixLeft1, mixTop8, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed, false,
    0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }
  addLabel(mixLeft0, mixTop9, labelWidth, labelHeight, uiTextSize, "Octave");
  addTextKnob(
    mixLeft1, mixTop9, labelWidth, labelHeight, uiTextSize, ID::transposeOctave,
    Scales::transposeOctave, false, 0, -transposeOctaveOffset);
  addLabel(mixLeft0, mixTop10, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    mixLeft1, mixTop10, labelWidth, labelHeight, uiTextSize, ID::transposeSemitone,
    Scales::transposeSemitone, false, 0, -transposeSemitoneOffset);
  addLabel(mixLeft0, mixTop11, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    mixLeft1, mixTop11, labelWidth, labelHeight, uiTextSize, ID::transposeCent,
    Scales::transposeCent, false, 5);
  addLabel(mixLeft0, mixTop12, labelWidth, labelHeight, uiTextSize, "Tuning");
  addOptionMenu(
    mixLeft1, mixTop12, labelWidth, labelHeight, uiTextSize, ID::tuning,
    {
      "Equal Temperament 12", "Equal Temperament 5", "Just Intonation 5-limit Major",
      "- Reserved 03 -",      "- Reserved 04 -",     "- Reserved 05 -",
      "- Reserved 06 -",      "- Reserved 07 -",     "- Reserved 08 -",
      "- Reserved 09 -",      "- Reserved 10 -",     "- Reserved 11 -",
      "- Reserved 12 -",      "- Reserved 13 -",     "- Reserved 14 -",
      "- Reserved 15 -",      "- Reserved 16 -",     "- Reserved 17 -",
      "- Reserved 18 -",      "- Reserved 19 -",     "- Reserved 20 -",
      "- Reserved 21 -",      "- Reserved 22 -",     "- Reserved 23 -",
      "- Reserved 24 -",      "- Reserved 25 -",     "- Reserved 26 -",
      "- Reserved 27 -",      "- Reserved 28 -",     "- Reserved 29 -",
      "- Reserved 30 -",      "- Reserved 31 -",     "- Reserved 32 -",
    });

  // Waveform.
  constexpr auto waveformTop0 = top0 + 0 * labelY;
  constexpr auto waveformTop1 = waveformTop0 + 1 * labelY;
  constexpr auto waveformTop2 = waveformTop1 + barBoxWidth;
  constexpr auto waveformLeft0 = left4;
  constexpr auto waveformLeft1 = left4 + labelWidth + 2 * margin;
  addGroupLabel(
    waveformLeft0, waveformTop0, groupLabelWidth, labelHeight, uiTextSize, "Waveform");

  {
    constexpr auto halfLabelWidth = int(labelWidth / 2);
    polyXControl = addTextKnob(
      waveformLeft0 + halfLabelWidth, waveformTop2, halfLabelWidth, labelHeight,
      uiTextSize, ID::polynomialPointX0, Scales::defaultScale, false, 5);
    polyYControl = addTextKnob(
      waveformLeft1 + halfLabelWidth, waveformTop2, halfLabelWidth, labelHeight,
      uiTextSize, ID::polynomialPointY0, Scales::polynomialPointY, false, 5);

    // A wasteful hack to avoid shadowing. Using `polyXYPad->linkControlFromId()` in
    // `updateUI` for linking parameters.
    controlMap.erase(ID::polynomialPointX0);
    controlMap.erase(ID::polynomialPointY0);

    polyXLabel = addLabel(
      waveformLeft0, waveformTop2, halfLabelWidth, labelHeight, uiTextSize, "X0");
    polyYLabel = addLabel(
      waveformLeft1, waveformTop2, halfLabelWidth, labelHeight, uiTextSize, "Y0");

    std::vector<ParamID> id(2 * nPolyOscControl);
    for (size_t i = 0; i < id.size(); ++i) id[i] = ID::polynomialPointX0 + ParamID(i);
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i) {
      value[i] = controller->getParamNormalized(id[i]);
    }
    std::vector<double> defaultValue(id.size());
    for (size_t i = 0; i < defaultValue.size(); ++i) {
      defaultValue[i] = param->getDefaultNormalized(id[i]);
    }
    polyXYPad = new PolynomialXYPad(
      this,
      CRect(
        waveformLeft0, waveformTop1, waveformLeft0 + barBoxWidth,
        waveformTop1 + barBoxWidth),
      id, value, defaultValue, palette, polyXLabel, polyYLabel, polyXControl,
      polyYControl);

    for (ParamID i = 0; i < id.size(); ++i) {
      arrayControlMap.emplace(std::make_pair(id[i], polyXYPad));
    }
    frame->addView(polyXYPad);
  }

  // Arpeggio.
  constexpr auto arpTop0 = top0;
  constexpr auto arpTop1 = arpTop0 + 1 * labelY;
  constexpr auto arpTop2 = arpTop0 + 2 * labelY;
  constexpr auto arpTop3 = arpTop0 + 3 * labelY;
  constexpr auto arpTop4 = arpTop0 + 4 * labelY;
  constexpr auto arpTop5 = arpTop0 + 5 * labelY;
  constexpr auto arpTop6 = arpTop0 + 6 * labelY;
  constexpr auto arpTop7 = arpTop0 + 7 * labelY;
  constexpr auto arpTop8 = arpTop0 + 8 * labelY;
  constexpr auto arpTop9 = arpTop0 + 9 * labelY;
  constexpr auto arpTop10 = arpTop0 + 10 * labelY;
  constexpr auto arpTop11 = arpTop0 + 11 * labelY;
  constexpr auto arpTop12 = arpTop0 + 12 * labelY;
  constexpr auto arpLeft0 = left8;
  constexpr auto arpLeft1 = arpLeft0 + labelWidth + 2 * margin;
  addToggleButton(
    arpLeft0, arpTop0, groupLabelWidth, labelHeight, uiTextSize, "Arpeggio",
    ID::arpeggioSwitch);

  addLabel(arpLeft0, arpTop1, labelWidth, labelHeight, uiTextSize, "Note / Beat");
  addTextKnob(
    arpLeft1, arpTop1, labelWidth, labelHeight, uiTextSize, ID::arpeggioNotesPerBeat,
    Scales::arpeggioNotesPerBeat, false, 0, 1);
  addLabel(arpLeft0, arpTop2, labelWidth, labelHeight, uiTextSize, "Loop Length [beat]");
  addTextKnob(
    arpLeft1, arpTop2, labelWidth, labelHeight, uiTextSize, ID::arpeggioLoopLengthInBeat,
    Scales::arpeggioLoopLengthInBeat, false, 0, 0);
  addLabel(arpLeft0, arpTop3, labelWidth, labelHeight, uiTextSize, "Duration Variation");
  addTextKnob(
    arpLeft1, arpTop3, labelWidth, labelHeight, uiTextSize, ID::arpeggioDurationVariation,
    Scales::arpeggioDurationVariation, false, 0, 1);
  addLabel(arpLeft0, arpTop4, labelWidth, labelHeight, uiTextSize, "Rest Chance");
  addTextKnob(
    arpLeft1, arpTop4, labelWidth, labelHeight, uiTextSize, ID::arpeggioRestChance,
    Scales::defaultScale, false, 5);

  addLabel(arpLeft0, arpTop7, labelWidth, labelHeight, uiTextSize, "Scale");
  addOptionMenu(
    arpLeft1, arpTop7, labelWidth, labelHeight, uiTextSize, ID::arpeggioScale,
    {
      "Octave",          "ET 5 Chromatic",  "ET 12 Major",     "ET 12 Minor",
      "Overtone 32",     "- Reserved 05 -", "- Reserved 06 -", "- Reserved 07 -",
      "- Reserved 08 -", "- Reserved 09 -", "- Reserved 10 -", "- Reserved 11 -",
      "- Reserved 12 -", "- Reserved 13 -", "- Reserved 14 -", "- Reserved 15 -",
      "- Reserved 16 -", "- Reserved 17 -", "- Reserved 18 -", "- Reserved 19 -",
      "- Reserved 20 -", "- Reserved 21 -", "- Reserved 22 -", "- Reserved 23 -",
      "- Reserved 24 -", "- Reserved 25 -", "- Reserved 26 -", "- Reserved 27 -",
      "- Reserved 28 -", "- Reserved 29 -", "- Reserved 30 -", "- Reserved 31 -",
      "- Reserved 32 -",
    });
  addLabel(arpLeft0, arpTop8, labelWidth, labelHeight, uiTextSize, "Pitch Drift [cent]");
  addTextKnob(
    arpLeft1, arpTop8, labelWidth, labelHeight, uiTextSize, ID::arpeggioPicthDriftCent,
    Scales::arpeggioPicthDriftCent, false, 5);
  addLabel(arpLeft0, arpTop9, labelWidth, labelHeight, uiTextSize, "Octave Range");
  addTextKnob(
    arpLeft1, arpTop9, labelWidth, labelHeight, uiTextSize, ID::arpeggioOctave,
    Scales::arpeggioOctave, false, 0, 1);

  // Randomize button.
  const auto randomButtonTop = top0 + 18 * labelY;
  const auto randomButtonLeft = left0 + labelWidth + 2 * margin;
  auto panicButton = new RandomizeButton(
    CRect(
      randomButtonLeft, randomButtonTop, randomButtonLeft + labelWidth,
      randomButtonTop + splashHeight),
    this, 0, "Random", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashTop = top0 + 18 * labelY;
  constexpr auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, labelWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "GlitchSprinkler", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
