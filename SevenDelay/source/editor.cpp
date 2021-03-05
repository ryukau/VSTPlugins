// (c) 2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

constexpr uint32_t defaultWidth = 960;
constexpr uint32_t defaultHeight = 330;
constexpr float pluginNameTextSize = 24.0f;
constexpr float labelHeight = 30.0f;
constexpr float midTextSize = 14.0f;
constexpr float uiTextSize = 14.0f;
constexpr float infoTextSize = 12.0f;
constexpr float checkboxWidth = 80.0f;
constexpr float sliderWidth = 70.0f;
constexpr float sliderHeight = 230.0f;
constexpr float margin = 0.0f;

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

template<> Editor<Synth::PlugParameter>::Editor(void *controller) : PlugEditor(controller)
{
  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

template<> Editor<Synth::PlugParameter>::~Editor()
{
  if (waveView) waveView->forget();
  if (timeTextView) timeTextView->forget();
}

template<> void Editor<Synth::PlugParameter>::addWaveView(const CRect &size)
{
  auto view = new WaveView(size, palette);
  view->shape = getPlainValue(Synth::ParameterID::lfoShape);
  view->phase = getPlainValue(Synth::ParameterID::lfoInitialPhase);
  frame->addView(view);

  if (waveView) waveView->forget();
  waveView = view;
  waveView->remember();
}

template<> void Editor<Synth::PlugParameter>::refreshWaveView(ParamID id)
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

template<> void Editor<Synth::PlugParameter>::refreshTimeTextView(ParamID id)
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

template<> ParamValue Editor<Synth::PlugParameter>::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
}

template<> void Editor<Synth::PlugParameter>::valueChanged(CControl *pControl)
{
  ParamID id = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);

  refreshWaveView(id);
  refreshTimeTextView(id);
}

template<> void Editor<Synth::PlugParameter>::updateUI(ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter == controlMap.end()) return;
  iter->second->setValueNormalized(normalized);
  iter->second->invalid();

  refreshWaveView(id);
  refreshTimeTextView(id);
}

template<> bool Editor<Synth::PlugParameter>::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto normalWidth = 80.0f;
  const auto normalHeight = normalWidth + 40.0f;
  const auto smallWidth = 40.0f;
  const auto smallHeight = 50.0f;
  const auto interval = 100.0f;

  // Delay.
  const auto delayTop1 = 50.0f;
  const auto delayLeft = 20.0f;
  addGroupLabel(delayLeft, 10.0f, 480.0f, labelHeight, midTextSize, "Delay");
  addKnob(delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Time", ID::time);
  addKnob(
    1.0f * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Feedback",
    ID::feedback);
  addKnob(
    2.0f * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Stereo",
    ID::offset);
  addKnob(
    3.0f * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Wet",
    ID::wetMix);
  addKnob<Style::accent>(
    4.0f * interval + delayLeft, delayTop1, normalWidth, margin, uiTextSize, "Dry",
    ID::dryMix);

  const auto delayTop2 = delayTop1 + normalHeight;
  const auto delayTop3 = delayTop2 + smallHeight;
  const auto delayTop4 = delayTop3 + smallHeight;

  if (timeTextView) timeTextView->forget();
  timeTextView = addTextView(
    delayLeft, delayTop2 - 15.0f, checkboxWidth + 15.0f, 40.0f, infoTextSize, "");
  timeTextView->remember();

  addCheckbox(
    delayLeft + 10.0f, delayTop3 - 15.0f, checkboxWidth, labelHeight, uiTextSize, "Sync",
    ID::tempoSync);
  addCheckbox(
    delayLeft + 10.0f, delayTop3 + 15.0f, checkboxWidth, labelHeight, uiTextSize,
    "Negative", ID::negativeFeedback);

  addKnob(
    1.0f * interval + delayLeft, delayTop2, smallWidth, margin, uiTextSize, "In Spread",
    ID::inSpread, LabelPosition::right);
  addKnob(
    1.0f * interval + delayLeft, delayTop3, smallWidth, margin, uiTextSize, "Out Spread",
    ID::outSpread, LabelPosition::right);
  addKnob(
    2.3f * interval + delayLeft, delayTop2, smallWidth, margin, uiTextSize, "In Pan",
    ID::inPan, LabelPosition::right);
  addKnob(
    2.3f * interval + delayLeft, delayTop3, smallWidth, margin, uiTextSize, "Out Pan",
    ID::outPan, LabelPosition::right);
  addKnob(
    2.3f * interval + delayLeft, delayTop4, smallWidth, margin, uiTextSize, "DC Kill",
    ID::dckill, LabelPosition::right);

  addKnob(
    3.6f * interval + delayLeft, delayTop2, smallWidth, margin, uiTextSize, "Allpass Cut",
    ID::toneCutoff, LabelPosition::right);
  addKnob(
    3.6f * interval + delayLeft, delayTop3, smallWidth, margin, uiTextSize, "Allpass Q",
    ID::toneQ, LabelPosition::right);
  addKnob(
    3.6f * interval + delayLeft, delayTop4, smallWidth, margin, uiTextSize, "Smooth",
    ID::smoothness, LabelPosition::right);

  // LFO.
  // 750 - 520 = 230 / 3 = 66 + 10
  const auto lfoLeft1 = 520.0f;
  addGroupLabel(520.0f, 10.0f, 420.0f, labelHeight, midTextSize, "LFO");
  addVSlider(
    lfoLeft1, 50.0f, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize,
    "To Time", ID::lfoTimeAmount);
  addVSlider(
    lfoLeft1 + 75.0f, 50.0f, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize,
    "To Allpass", ID::lfoToneAmount);
  addVSlider<Style::accent>(
    lfoLeft1 + 150.0f, 50.0f, sliderWidth, sliderHeight, margin, labelHeight, uiTextSize,
    "Frequency", ID::lfoFrequency);
  const auto lfoLeft2 = lfoLeft1 + 230.0f;
  addKnob(lfoLeft2, 50.0f, normalWidth, margin, uiTextSize, "Shape", ID::lfoShape);
  addKnob(
    interval + lfoLeft2, 50.0f, normalWidth, margin, uiTextSize, "Phase",
    ID::lfoInitialPhase);

  const auto waveViewLeft = 760.0f;
  const auto waveViewTop = 170.0f;
  const auto waveViewWidth = 180.0f;
  const auto waveViewHeight = 110.0f;
  addWaveView(CRect(
    waveViewLeft, waveViewTop, waveViewLeft + waveViewWidth,
    waveViewTop + waveViewHeight));
  addToggleButton(
    waveViewLeft, waveViewTop + waveViewHeight + 10.0f, waveViewWidth, labelHeight,
    midTextSize, "LFO Hold", ID::lfoHold);

  // Plugin name.
  const auto nameLeft = delayLeft;
  const auto nameTop = defaultHeight - 50.0f;
  const auto nameWidth = 180.0f;
  addSplashScreen(
    nameLeft, nameTop, nameWidth, 40.0f, 200.0f, 20.0f, defaultWidth - 400.0f,
    defaultHeight - 40.0f, pluginNameTextSize, "SevenDelay");

  refreshTimeTextView(ID::time);

  return true;
}

} // namespace Vst
} // namespace Steinberg
