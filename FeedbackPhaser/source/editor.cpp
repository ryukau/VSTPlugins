// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "version.hpp"

#include <algorithm>
#include <random>

const char *outputCleanText = "Output is clean.";

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
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
    if (infoLabel.get()) {
      infoLabel->setText(outputCleanText);
      infoLabel->setDirty();
    }
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  using ID = Synth::ParameterID::ID;

  PlugEditor::updateUI(id, normalized);

  if (infoLabel.get() && id == ID::tooMuchFeedback) {
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

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;
  const auto left1 = left0 + labelWidth + 2 * margin;
  const auto left2 = left1 + labelWidth + 4 * margin;
  const auto left3 = left2 + labelWidth + 2 * margin;

  // Gain.
  const auto gainTop0 = top0;
  const auto gainTop1 = gainTop0 + labelY;
  const auto gainTop2 = gainTop1 + labelY;
  const auto gainTop3 = gainTop2 + labelY;
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
  const auto feedTop0 = gainTop3 + labelY;
  const auto feedTop1 = feedTop0 + labelY;
  const auto feedTop2 = feedTop1 + labelY;
  const auto feedTop3 = feedTop2 + labelY;
  const auto feedTop4 = feedTop3 + labelY;
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
  const auto noteTop0 = feedTop4 + labelY;
  const auto noteTop1 = noteTop0 + labelY;
  const auto noteTop2 = noteTop1 + labelY;
  const auto noteTop3 = noteTop2 + labelY;
  const auto noteTop4 = noteTop3 + labelY;
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
  const auto apTop0 = top0;
  const auto apTop1 = apTop0 + labelY;
  const auto apTop2 = apTop1 + labelY;
  const auto apTop3 = apTop2 + labelY;
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
  const auto modTop0 = apTop3 + labelY;
  const auto modTop1 = modTop0 + labelY;
  const auto modTop2 = modTop1 + labelY;
  const auto modTop3 = modTop2 + labelY;
  const auto modTop4 = modTop3 + labelY;
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
  const auto miscTop0 = modTop4 + labelY;
  const auto miscTop1 = miscTop0 + labelY;
  const auto miscTop2 = miscTop1 + labelY;
  const auto miscTop3 = miscTop2 + labelY;
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
  const auto splashMargin = uiMargin;
  const auto splashWidth = 2 * labelWidth - 2 * uiMargin;
  const auto splashHeight = labelY;
  const auto splashTop = defaultHeight - uiMargin - splashHeight;
  const auto splashLeft = left2 + uiMargin;
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
