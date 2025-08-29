// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
}

Editor::~Editor()
{
  if (waveView) waveView->forget();
  if (timeTextView) timeTextView->forget();
}

ParamValue Editor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
}

void Editor::addWaveView(const CRect &size)
{
  auto view = new WaveView(size, palette);
  view->shape = getPlainValue(Synth::ParameterID::lfoShape);
  view->phase = getPlainValue(Synth::ParameterID::lfoInitialPhase);
  frame->addView(view);

  if (waveView) waveView->forget();
  waveView = view;
  waveView->remember();
}

void Editor::refreshWaveView(ParamID id)
{
  if (id == Synth::ParameterID::lfoShape) {
    if (waveView == nullptr) return;
    waveView->shape = getPlainValue(id);
    waveView->setDirty(true);
  } else if (id == Synth::ParameterID::lfoInitialPhase) {
    if (waveView == nullptr) return;
    waveView->phase = getPlainValue(id);
    waveView->setDirty(true);
  }
}

void Editor::refreshTimeTextView(ParamID id)
{
  using ID = Synth::ParameterID::ID;

  if (timeTextView == nullptr) return;
  if (id != ID::time && id != ID::offset && id != ID::tempoSync) return;

  auto timeC = getPlainValue(ID::time);
  auto offset = getPlainValue(ID::offset);

  std::stringstream ss;
  if (0.0f != controller->getParamNormalized(ID::tempoSync)) { // is tempo syncing.
    if (timeC > 1.0) timeC = std::floor(2.0 * timeC) / 2.0;
    auto timeL = offset < 0.0 ? timeC * (1.0 + offset) : timeC;
    auto timeR = offset > 0.0 ? timeC * (1.0 - offset) : timeC;
    ss << "L: " << std::setprecision(4) << timeL << "/16\nR: " << std::setprecision(4)
       << timeR << "/16";
  } else {
    timeC *= 1e3;
    auto timeL = offset < 0.0 ? timeC * (1.0 + offset) : timeC;
    auto timeR = offset > 0.0 ? timeC * (1.0 - offset) : timeC;
    ss << "L: " << std::setw(7) << std::right << timeL << "ms\nR: " << std::setw(7)
       << std::right << timeR << "ms";
  }
  std::string text = ss.str();
  timeTextView->setText(text);
  timeTextView->setDirty(true);
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);

  refreshWaveView(id);
  refreshTimeTextView(id);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter == controlMap.end()) return;
  iter->second->setValueNormalized(normalized);
  iter->second->invalid();

  refreshWaveView(id);
  refreshTimeTextView(id);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto sc = palette.guiScale();

  const auto normalWidth = int(sc * 80);
  const auto normalHeight = normalWidth + int(sc * 40);
  const auto smallWidth = int(sc * 40);
  const auto smallHeight = int(sc * 50);
  const auto interval = int(sc * 100);

  // Delay.
  const auto delayTop1 = int(sc * 50);
  const auto delayLeft = int(sc * 20);
  addGroupLabel(
    delayLeft, int(sc * 10), int(sc * 480), labelHeight, midTextSize, "Delay");
  addKnob(delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Time", ID::time);
  addKnob(
    1 * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Feedback",
    ID::feedback);
  addKnob(
    2 * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Stereo",
    ID::offset);
  addKnob(
    3 * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Wet",
    ID::wetMix);
  addKnob<Style::accent>(
    4 * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Dry",
    ID::dryMix);

  const auto delayTop2 = delayTop1 + normalHeight;
  const auto delayTop3 = delayTop2 + smallHeight;
  const auto delayTop4 = delayTop3 + smallHeight;

  if (timeTextView) timeTextView->forget();
  timeTextView = addTextView(
    delayLeft, delayTop2 - int(sc * 15), checkboxWidth + int(sc * 15), int(sc * 40),
    infoTextSize, "");
  timeTextView->remember();

  addCheckbox(
    delayLeft + int(sc * 10), delayTop3 - int(sc * 15), checkboxWidth, labelHeight,
    uiTextSize, "Sync", ID::tempoSync);
  addCheckbox(
    delayLeft + int(sc * 10), delayTop3 + int(sc * 15), checkboxWidth, labelHeight,
    uiTextSize, "Negative", ID::negativeFeedback);

  addKnob(
    1.0f * interval + delayLeft, delayTop2, smallWidth, margin, uiTextSize, "In Spread",
    ID::inSpread, LabelPosition::right);
  addKnob(
    1.0f * interval + delayLeft, delayTop3, smallWidth, margin, uiTextSize, "Out Spread",
    ID::outSpread, LabelPosition::right);

  addKnob(
    std::floor(2.3f * interval) + delayLeft, delayTop2, smallWidth, margin, uiTextSize,
    "In Pan", ID::inPan, LabelPosition::right);
  addKnob(
    std::floor(2.3f * interval) + delayLeft, delayTop3, smallWidth, margin, uiTextSize,
    "Out Pan", ID::outPan, LabelPosition::right);
  addKnob(
    std::floor(2.3f * interval) + delayLeft, delayTop4, smallWidth, margin, uiTextSize,
    "DC Kill", ID::dckill, LabelPosition::right);

  addKnob(
    std::floor(3.6f * interval) + delayLeft, delayTop2, smallWidth, margin, uiTextSize,
    "Allpass Cut", ID::toneCutoff, LabelPosition::right);
  addKnob(
    std::floor(3.6f * interval) + delayLeft, delayTop3, smallWidth, margin, uiTextSize,
    "Allpass Q", ID::toneQ, LabelPosition::right);
  addKnob(
    std::floor(3.6f * interval) + delayLeft, delayTop4, smallWidth, margin, uiTextSize,
    "Smooth", ID::smoothness, LabelPosition::right);

  // LFO.
  // 750 - 520 = 230 / 3 = 66 + 10
  const auto lfoLeft1 = int(sc * 520);
  addGroupLabel(lfoLeft1, int(sc * 10), int(sc * 420), labelHeight, midTextSize, "LFO");
  addVSlider(
    lfoLeft1, int(sc * 50), sliderWidth, sliderHeight, margin, labelHeight, uiTextSize,
    "To Time", ID::lfoTimeAmount);
  addVSlider(
    lfoLeft1 + int(sc * 75), int(sc * 50), sliderWidth, sliderHeight, margin, labelHeight,
    uiTextSize, "To Allpass", ID::lfoToneAmount);
  addVSlider<Style::accent>(
    lfoLeft1 + int(sc * 150), int(sc * 50), sliderWidth, sliderHeight, margin,
    labelHeight, uiTextSize, "Frequency", ID::lfoFrequency);
  const auto lfoLeft2 = lfoLeft1 + int(sc * 230);
  addKnob(lfoLeft2, int(sc * 50), normalWidth, margin, uiTextSize, "Shape", ID::lfoShape);
  addKnob(
    interval + lfoLeft2, int(sc * 50), normalWidth, margin, uiTextSize, "Phase",
    ID::lfoInitialPhase);

  const auto waveViewLeft = int(sc * 760);
  const auto waveViewTop = int(sc * 170);
  const auto waveViewWidth = int(sc * 180);
  const auto waveViewHeight = int(sc * 110);
  addWaveView(CRect(
    waveViewLeft, waveViewTop, waveViewLeft + waveViewWidth,
    waveViewTop + waveViewHeight));
  addToggleButton(
    waveViewLeft, waveViewTop + waveViewHeight + int(sc * 10), waveViewWidth, labelHeight,
    midTextSize, "LFO Hold", ID::lfoHold);

  // Plugin name.
  const auto nameLeft = delayLeft;
  const auto nameTop = defaultHeight - int(sc * 50);
  const auto nameWidth = int(sc * 180);
  addSplashScreen(
    nameLeft, nameTop, nameWidth, int(sc * 40), int(sc * 200), int(sc * 20),
    defaultWidth - int(sc * 400), defaultHeight - int(sc * 40), pluginNameTextSize,
    "SevenDelay");

  refreshTimeTextView(ID::time);

  return true;
}

} // namespace Vst
} // namespace Steinberg
