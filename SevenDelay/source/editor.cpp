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

  fontSize = 14.0;

  viewRect = ViewRect{0, 0, 960, 330};
  setRect(viewRect);
}

Editor::~Editor()
{
  if (waveView) waveView->forget();
}

void Editor::addWaveView(const CRect &size)
{
  auto view = new WaveView(size);
  view->shape = getPlainValue(Synth::ParameterID::lfoShape);
  view->phase = getPlainValue(Synth::ParameterID::lfoInitialPhase);
  frame->addView(view);

  if (waveView) waveView->forget();
  waveView = view;
  waveView->remember();
}

ParamValue Editor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
}

void Editor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);

  refreshWaveView(tag);
}

void Editor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter == controlMap.end()) return;
  iter->second->setValueNormalized(normalized);
  iter->second->invalid();

  refreshWaveView(id);
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

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto normalWidth = 90.0;
  const auto normalHeight = normalWidth + 40.0;
  const auto smallWidth = 50.0;
  const auto smallHeight = 50.0;
  const auto interval = 100.0;

  // Delay.
  const auto delayTop1 = 50.0;
  const auto delayLeft = 20.0;
  addGroupLabel(delayLeft, 10.0, 480.0, "Delay");
  addKnob(delayLeft, delayTop1, normalWidth, colorBlue, "Time", ID::time);
  addKnob(
    1.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Feedback",
    ID::feedback);
  addKnob(
    2.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Stereo", ID::offset,
    LabelPosition::bottom);
  addKnob(
    3.0 * interval + delayLeft, delayTop1, normalWidth, colorBlue, "Wet", ID::wetMix);
  addKnob(
    4.0 * interval + delayLeft, delayTop1, normalWidth, colorGreen, "Dry", ID::dryMix);

  const auto delayTop2 = delayTop1 + normalHeight - 10.0;
  const auto delayTop3 = delayTop2 + smallHeight;
  const auto delayTop4 = delayTop3 + smallHeight;
  addCheckbox(delayLeft + 10.0, delayTop2 + 10.0, 100.0, "Sync", ID::tempoSync);
  addCheckbox(
    delayLeft + 10.0, delayTop3 + 10.0, 100.0, "Negative", ID::negativeFeedback);

  addKnob(
    1.0 * interval + delayLeft, delayTop2, smallWidth, colorBlue, " In Spread",
    ID::inSpread, LabelPosition::right);
  addKnob(
    1.0 * interval + delayLeft, delayTop3, smallWidth, colorBlue, " Out Spread",
    ID::outSpread, LabelPosition::right);
  addKnob(
    2.3 * interval + delayLeft, delayTop2, smallWidth, colorBlue, " In Pan", ID::inPan,
    LabelPosition::right);
  addKnob(
    2.3 * interval + delayLeft, delayTop3, smallWidth, colorBlue, " Out Pan", ID::outPan,
    LabelPosition::right);
  addKnob(
    2.3 * interval + delayLeft, delayTop4, smallWidth, colorBlue, " DC Kill", ID::dckill,
    LabelPosition::right);

  addKnob(
    3.6 * interval + delayLeft, delayTop2, smallWidth, colorBlue, " Allpass Cut",
    ID::toneCutoff, LabelPosition::right);
  addKnob(
    3.6 * interval + delayLeft, delayTop3, smallWidth, colorBlue, " Allpass Q", ID::toneQ,
    LabelPosition::right);
  addKnob(
    3.6 * interval + delayLeft, delayTop4, smallWidth, colorBlue, " Smooth",
    ID::smoothness, LabelPosition::right);

  // LFO.
  // 750 - 520 = 230 / 3 = 66 + 10
  const auto lfoLeft1 = 520.0;
  addGroupLabel(520.0, 10.0, 420.0, "LFO");
  addVSlider(lfoLeft1, 50.0, colorBlue, "To Time", ID::lfoTimeAmount);
  addVSlider(lfoLeft1 + 75.0, 50.0, colorBlue, "To Allpass", ID::lfoToneAmount);
  addVSlider(lfoLeft1 + 150.0, 50.0, colorGreen, "Frequency", ID::lfoFrequency);
  const auto lfoLeft2 = lfoLeft1 + 230.0;
  addKnob(lfoLeft2, 50.0, normalWidth, colorBlue, "Shape", ID::lfoShape);
  addKnob(
    interval + lfoLeft2, 50.0, normalWidth, colorBlue, "Phase", ID::lfoInitialPhase);

  addButton(
    WaveViewSize.left, WaveViewSize.bottom + 10.0, WaveViewSize.right - WaveViewSize.left,
    "LFO Hold", ID::lfoHold, CTextButton::kOnOffStyle);
  addWaveView(WaveViewSize);

  // Plugin name.
  const auto nameLeft = delayLeft;
  const auto nameTop = viewRect.bottom - 60.0;
  const auto nameWidth = 180.0;
  addSplashScreen(
    nameLeft, nameTop, nameWidth, 40.0, 200.0, 20.0, viewRect.right - 400.0,
    viewRect.bottom - 40.0, "SevenDelay", 24.0f);

  return true;
}

} // namespace Vst
} // namespace Steinberg
