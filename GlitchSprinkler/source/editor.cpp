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

constexpr const char *wireDidntCollidedText = "Wire didn't collide.";
constexpr const char *membraneDidntCollidedText = "Membrane didn't collide.";

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  using ID = Synth::ParameterID::ID;

  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  PlugEditor::updateUI(id, normalized);

  using ID = Synth::ParameterID::ID;

  if (id >= ID::polynomialPointX0 && id < ID::polynomialPointX0 + nPolyOscControl) {
    polynomialXYPad->setXAt(id - ID::polynomialPointX0, normalized);
    polynomialXYPad->setDirty();
  } else if (id >= ID::polynomialPointY0 && id < ID::polynomialPointY0 + nPolyOscControl)
  {
    polynomialXYPad->setYAt(id - ID::polynomialPointY0, normalized);
    polynomialXYPad->setDirty();
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
  constexpr auto mixLeft0 = left0;
  constexpr auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Mix & Options");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);

  addLabel(mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    mixLeft1, mixTop2, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed, false,
    0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }

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

  // Waveform.
  constexpr auto waveformTop0 = top0 + 0 * labelY;
  constexpr auto waveformTop1 = waveformTop0 + 1 * labelY;
  constexpr auto waveformLeft0 = left4;
  addGroupLabel(
    waveformLeft0, waveformTop0, groupLabelWidth, labelHeight, uiTextSize, "Waveform");

  polynomialXYPad = new PolynomialXYPad(
    CRect(
      waveformLeft0, waveformTop1, waveformLeft0 + barBoxWidth,
      waveformTop1 + barBoxWidth),
    this, 0, palette, this);
  for (size_t idx = 0; idx < nPolyOscControl; ++idx) {
    polynomialXYPad->setXAt(
      idx, controller->getParamNormalized(ID::polynomialPointX0 + idx));
    polynomialXYPad->setYAt(
      idx, controller->getParamNormalized(ID::polynomialPointY0 + idx));
  }
  frame->addView(polynomialXYPad);

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
