// (c) 2023 Takamitsu Endo
//
// This file is part of FeedbackPhaser.
//
// FeedbackPhaser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FeedbackPhaser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FeedbackPhaser.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

constexpr float uiTextSize = 12.0f;
constexpr float pluginNameTextSize = 14.0f;
constexpr float margin = 5.0f;
constexpr float uiMargin = 20.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = labelHeight + 2 * margin;
constexpr float labelWidth = 100.0f;
constexpr float labelX = labelWidth + margin;
constexpr const char *outputCleanText = "Output is clean.";

constexpr int_least32_t defaultWidth
  = int_least32_t(2 * uiMargin + 4 * labelWidth + 8 * margin);
constexpr int_least32_t defaultHeight
  = int_least32_t(2 * uiMargin + 14 * labelY + 2 * margin);

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

  if (id != ID::tooMuchFeedback) {
    controller->setParamNormalized(ID::tooMuchFeedback, 0.0);
    controller->performEdit(ID::tooMuchFeedback, 0.0);
    infoLabel->setText(outputCleanText);
    infoLabel->setDirty();
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  using ID = Synth::ParameterID::ID;

  PlugEditor::updateUI(id, normalized);

  if (id == ID::tooMuchFeedback) {
    if (getPlainValue(ID::tooMuchFeedback)) {
      infoLabel->setText("Too much feedback.");
    } else {
      infoLabel->setText(outputCleanText);
    }
    infoLabel->setDirty();
  }
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  constexpr auto top0 = uiMargin;
  constexpr auto left0 = uiMargin;
  constexpr auto left1 = left0 + labelWidth + 2 * margin;
  constexpr auto left2 = left1 + labelWidth + 4 * margin;
  constexpr auto left3 = left2 + labelWidth + 2 * margin;

  // Gain.
  constexpr auto gainTop0 = top0;
  constexpr auto gainTop1 = gainTop0 + labelY;
  constexpr auto gainTop2 = gainTop1 + labelY;
  constexpr auto gainTop3 = gainTop2 + labelY;
  addGroupLabel(
    left0, gainTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Gain");
  addLabel(left0, gainTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    left1, gainTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain,
    Scales::outputGain, true, 5);
  addLabel(left0, gainTop2, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    left1, gainTop2, labelWidth, labelHeight, uiTextSize, ID::outputHighpassHz,
    Scales::cutoffHz, false, 5);
  addLabel(left0, gainTop3, labelWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob<Style::warning>(
    left1, gainTop3, labelWidth, labelHeight, uiTextSize, ID::mix, Scales::bipolarScale,
    false, 5);

  // Feedback.
  constexpr auto feedTop0 = gainTop3 + labelY;
  constexpr auto feedTop1 = feedTop0 + labelY;
  constexpr auto feedTop2 = feedTop1 + labelY;
  constexpr auto feedTop3 = feedTop2 + labelY;
  constexpr auto feedTop4 = feedTop3 + labelY;
  addGroupLabel(
    left0, feedTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Feedback");
  addLabel(left0, feedTop1, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob<Style::warning>(
    left1, feedTop1, labelWidth, labelHeight, uiTextSize, ID::feedback, Scales::feedback,
    false, 5);
  addLabel(left0, feedTop2, labelWidth, labelHeight, uiTextSize, "Safety Clip [dB]");
  addTextKnob<Style::warning>(
    left1, feedTop2, labelWidth, labelHeight, uiTextSize, ID::feedbackClip,
    Scales::feedbackClip, true, 5);
  addLabel(left0, feedTop3, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]");
  addTextKnob(
    left1, feedTop3, labelWidth, labelHeight, uiTextSize, ID::feedbackHighpassHz,
    Scales::cutoffHz, false, 5);
  infoLabel = addLabel(
    left0, feedTop4, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize,
    outputCleanText);

  // Note.
  constexpr auto noteTop0 = feedTop4 + labelY;
  constexpr auto noteTop1 = noteTop0 + labelY;
  constexpr auto noteTop2 = noteTop1 + labelY;
  constexpr auto noteTop3 = noteTop2 + labelY;
  constexpr auto noteTop4 = noteTop3 + labelY;
  addGroupLabel(
    left0, noteTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Note");
  addLabel(left0, noteTop1, labelWidth, labelHeight, uiTextSize, "Transpose [st.]");
  addTextKnob(
    left1, noteTop1, labelWidth, labelHeight, uiTextSize, ID::notePitchCenter,
    Scales::notePitchCenter, false, 5);
  addLabel(left0, noteTop2, labelWidth, labelHeight, uiTextSize, "To Allpass");
  addTextKnob(
    left1, noteTop2, labelWidth, labelHeight, uiTextSize, ID::notePitchToAllpassCutoff,
    Scales::notePitchToAllpassCutoff, false, 5);
  addLabel(left0, noteTop3, labelWidth, labelHeight, uiTextSize, "Slide [s]");
  addTextKnob(
    left1, noteTop3, labelWidth, labelHeight, uiTextSize, ID::notePitchSlideSecond,
    Scales::notePitchEnvelopeSecond, false, 5);
  addLabel(left0, noteTop4, labelWidth, labelHeight, uiTextSize, "Release [s]");
  addTextKnob(
    left1, noteTop4, labelWidth, labelHeight, uiTextSize, ID::notePitchReleaseSecond,
    Scales::notePitchEnvelopeSecond, false, 5);

  // Allpass.
  constexpr auto apTop0 = top0;
  constexpr auto apTop1 = apTop0 + labelY;
  constexpr auto apTop2 = apTop1 + labelY;
  constexpr auto apTop3 = apTop2 + labelY;
  addGroupLabel(
    left2, apTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Allpass");
  addLabel(left2, apTop1, labelWidth, labelHeight, uiTextSize, "Stage");
  addTextKnob(
    left3, apTop1, labelWidth, labelHeight, uiTextSize, ID::stage, Scales::stage, false,
    0, 1);
  addLabel(left2, apTop2, labelWidth, labelHeight, uiTextSize, "Spread");
  addTextKnob(
    left3, apTop2, labelWidth, labelHeight, uiTextSize, ID::allpassSpread,
    Scales::defaultScale, false, 5);
  addLabel(left2, apTop3, labelWidth, labelHeight, uiTextSize, "Center [Hz]");
  addTextKnob(
    left3, apTop3, labelWidth, labelHeight, uiTextSize, ID::allpassCenterHz,
    Scales::cutoffHz, false, 5);

  // Modulation.
  constexpr auto modTop0 = apTop3 + labelY;
  constexpr auto modTop1 = modTop0 + labelY;
  constexpr auto modTop2 = modTop1 + labelY;
  constexpr auto modTop3 = modTop2 + labelY;
  constexpr auto modTop4 = modTop3 + labelY;
  addGroupLabel(
    left2, modTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Modulation");
  addLabel(left2, modTop1, labelWidth, labelHeight, uiTextSize, "Type");
  std::vector<std::string> modTypeItems{"Exp. Mul.", "Lin. Mul.", "Add"};
  addOptionMenu(
    left3, modTop1, labelWidth, labelHeight, uiTextSize, ID::modType, modTypeItems);
  addLabel(left2, modTop2, labelWidth, labelHeight, uiTextSize, "Amount");
  addTextKnob<Style::warning>(
    left3, modTop2, labelWidth, labelHeight, uiTextSize, ID::modAmount,
    Scales::modulation, false, 5);
  addLabel(left2, modTop3, labelWidth, labelHeight, uiTextSize, "Mod. Asym.");
  addTextKnob(
    left3, modTop3, labelWidth, labelHeight, uiTextSize, ID::modAsymmetry,
    Scales::modAsymmetry, false, 5);
  addLabel(left2, modTop4, labelWidth, labelHeight, uiTextSize, "Post LP [Hz]");
  addTextKnob(
    left3, modTop4, labelWidth, labelHeight, uiTextSize, ID::modLowpassHz,
    Scales::cutoffHz, false, 5);

  // Misc.
  constexpr auto miscTop0 = modTop4 + labelY;
  constexpr auto miscTop1 = miscTop0 + labelY;
  constexpr auto miscTop2 = miscTop1 + labelY;
  constexpr auto miscTop3 = miscTop2 + labelY;
  addGroupLabel(
    left2, miscTop0, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Misc.");
  addToggleButton(
    left2, miscTop1, 2 * labelWidth + 2 * margin, labelHeight, uiTextSize, "Side Chain",
    ID::modSideChain);
  addLabel(left2, miscTop2, labelWidth, labelHeight, uiTextSize, "Smoothing [s]");
  addTextKnob(
    left3, miscTop2, labelWidth, labelHeight, uiTextSize, ID::parameterSmoothingSecond,
    Scales::parameterSmoothingSecond, false, 5);
  addLabel(left2, miscTop3, labelWidth, labelHeight, uiTextSize, "Oversampling");
  std::vector<std::string> oversamplingItems{"1x", "2x", "8x"};
  addOptionMenu(
    left3, miscTop3, labelWidth, labelHeight, uiTextSize, ID::oversampling,
    oversamplingItems);

  // Plugin name.
  constexpr auto splashMargin = uiMargin;
  constexpr auto splashWidth = 2 * labelWidth - 2 * uiMargin;
  constexpr auto splashHeight = labelY;
  constexpr auto splashTop = defaultHeight - uiMargin - splashHeight;
  constexpr auto splashLeft = left2 + uiMargin;
  addSplashScreen(
    splashLeft, splashTop, splashWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "FeedbackPhaser");

  // Probably this restartComponent() is redundant, but to make sure.
  controller->getComponentHandler()->restartComponent(kLatencyChanged);

  return true;
}

} // namespace Vst
} // namespace Steinberg
