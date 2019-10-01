// (c) 2019 Takamitsu Endo
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

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "../parameter.hpp"
#include "checkbox.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "plugeditor.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "textbutton.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

PlugEditor::PlugEditor(void *controller) : VSTGUIEditor(controller) { setRect(viewRect); }

bool PlugEditor::open(void *parent, const PlatformType &platformType)
{
  if (frame) return false;

  setIdleRate(1000 / 60);

  frame = new CFrame(
    CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
  if (frame == nullptr) return false;
  frame->setBackgroundColor(colorWhite);
  frame->registerMouseObserver(this);
  frame->open(parent);

  using ID = Synth::ParameterID;
  Synth::GlobalParameter param;

  // Oscillators.
  const auto oscWidth = 2.0 * knobWidth + 4.0 * margin;
  const auto oscTop = 10.0;
  const auto oscLeft1 = 10.0;
  addOscillatorSection("Osc1", oscLeft1, oscTop, ID::osc1Gain, ID::osc1Semi, ID::osc1Cent,
    ID::osc1Sync, ID::osc1SyncType, ID::osc1PTROrder, ID::osc1Phase, ID::osc1PhaseLock,
    param.osc1Gain, param.osc1Semi, param.osc1Cent, param.osc1Sync, param.osc1Phase);

  const auto oscLeft2 = oscLeft1 + oscWidth + 10.0;
  addOscillatorSection("Osc2", oscLeft2, oscTop, ID::osc2Gain, ID::osc2Semi, ID::osc2Cent,
    ID::osc2Sync, ID::osc2SyncType, ID::osc2PTROrder, ID::osc2Phase, ID::osc2PhaseLock,
    param.osc2Gain, param.osc2Semi, param.osc2Cent, param.osc2Sync, param.osc2Phase);

  const auto oscTop2 = 4.0 * labelY + 2.0 * knobY + 2.0 * knobHeight - margin;
  addCheckbox(oscLeft2 + margin + knobX,
    oscTop2 + labelY + (knobHeight - labelHeight) / 2.0 - 10.0, "Invert", ID::osc2Invert);

  // Cross modulation.
  const auto crossTop = oscTop2 + knobY;
  const auto crossLeft = oscLeft1;
  addGroupLabel(crossLeft, crossTop, 2.0 * oscWidth + 10.0, "Modulation");

  const auto crossTop2 = crossTop + labelY;
  const auto crossKnobLeft = margin + crossLeft;
  addKnob(crossKnobLeft, crossTop2, 1.5 * knobWidth, colorBlue, "Osc1->Sync1",
    ID::fmOsc1ToSync1, param.fmOsc1ToSync1);
  addKnob(crossKnobLeft + 1.5 * knobWidth + 10.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc1->Freq2", ID::fmOsc1ToFreq2, param.fmOsc1ToFreq2);
  addKnob(crossKnobLeft + 3.0 * knobWidth + 20.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc2->Sync1", ID::fmOsc2ToSync1, param.fmOsc2ToSync1);

  // Modulation envelope and LFO.
  const auto modTop = oscTop;
  const auto modLeft = oscLeft2 + oscWidth + 20.0;
  addGroupLabel(modLeft, modTop, 6.0 * knobX, "Modulation");

  const auto modTop1 = modTop + labelY;
  addKnob(modLeft, modTop1, knobWidth, colorBlue, "Attack", ID::modEnvelopeA,
    param.modEnvelopeA);
  addKnob(modLeft + 1.0 * knobX, modTop1, knobWidth, colorBlue, "Curve",
    ID::modEnvelopeCurve, param.modEnvelopeCurve);
  addKnob(modLeft + 2.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq1",
    ID::modEnvelopeToFreq1, param.modEnvelopeToFreq1);
  addKnob(modLeft + 3.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync1",
    ID::modEnvelopeToSync1, param.modEnvelopeToSync1);
  addKnob(modLeft + 4.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq2",
    ID::modEnvelopeToFreq2, param.modEnvelopeToFreq2);
  addKnob(modLeft + 5.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync2",
    ID::modEnvelopeToSync2, param.modEnvelopeToSync2);

  const auto modTop2 = modTop1 + knobY;
  addKnob(modLeft, modTop2, knobWidth, colorBlue, "LFO", ID::modLFOFrequency,
    param.modLFOFrequency);
  addKnob(modLeft + 1.0 * knobX, modTop2, knobWidth, colorBlue, "NoiseMix",
    ID::modLFONoiseMix, param.modLFONoiseMix);
  addKnob(modLeft + 2.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq1",
    ID::modLFOToFreq1, param.modLFOToFreq1);
  addKnob(modLeft + 3.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync1",
    ID::modLFOToSync1, param.modLFOToSync1);
  addKnob(modLeft + 4.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq2",
    ID::modLFOToFreq2, param.modLFOToFreq2);
  addKnob(modLeft + 5.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync2",
    ID::modLFOToSync2, param.modLFOToSync2);

  // Gain.
  const auto gainTop = modTop2 + knobY + margin;
  const auto gainLeft = modLeft;
  addGroupLabel(gainLeft, gainTop, 6.0 * knobX, "Gain");

  const auto gainKnobTop = gainTop + labelY;
  addKnob(gainLeft, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain, param.gain);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA,
    param.gainA);
  addKnob(gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD,
    param.gainD);
  addKnob(gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS,
    param.gainS);
  addKnob(gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR,
    param.gainR);
  addKnob(gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
    ID::gainEnvelopeCurve, param.gainEnvelopeCurve);

  // Filter.
  const auto filterTop = gainKnobTop + knobY + margin;
  const auto filterLeft = modLeft;
  addGroupLabel(filterLeft, filterTop, 4.0 * knobX - 10.0, "Filter");
  addCheckbox(filterLeft + 4.0 * knobX, filterTop, "Dirty Buffer", ID::filterDirty);

  const auto filterTop1 = filterTop + labelY;
  addKnob(filterLeft, filterTop1, knobWidth, colorBlue, "Cut", ID::filterCutoff,
    param.filterCutoff);
  addKnob(filterLeft + 1.0 * knobX, filterTop1, knobWidth, colorBlue, "Res",
    ID::filterResonance, param.filterResonance);
  addKnob(filterLeft + 2.0 * knobX, filterTop1, knobWidth, colorBlue, "Feed",
    ID::filterFeedback, param.filterFeedback);
  addKnob(filterLeft + 3.0 * knobX, filterTop1, knobWidth, colorBlue, "Sat",
    ID::filterSaturation, param.filterSaturation);

  const auto filterMenuWidth = 100.0;
  std::vector<UTF8String> filterTypeOptions = {"LP", "HP", "BP", "Notch", "Bypass"};
  addOptionMenu(filterLeft + 4.0 * knobX, filterTop1, filterMenuWidth, ID::filterType,
    filterTypeOptions);
  std::vector<UTF8String> filterShaperOptions
    = {"HardClip", "Tanh", "ShaperA", "ShaperB"};
  addOptionMenu(filterLeft + 4.0 * knobX, filterTop1 + labelY, filterMenuWidth,
    ID::filterShaper, filterShaperOptions);

  const auto filterTop2 = filterTop1 + knobY;
  addKnob(filterLeft, filterTop2, knobWidth, colorBlue, "A", ID::filterA, param.filterA);
  addKnob(filterLeft + 1.0 * knobX, filterTop2, knobWidth, colorBlue, "D", ID::filterD,
    param.filterD);
  addKnob(filterLeft + 2.0 * knobX, filterTop2, knobWidth, colorBlue, "S", ID::filterS,
    param.filterS);
  addKnob(filterLeft + 3.0 * knobX, filterTop2, knobWidth, colorBlue, "R", ID::filterR,
    param.filterR);
  addKnob(filterLeft + 4.0 * knobX, filterTop2, knobWidth, colorBlue, "To Cut",
    ID::filterCutoffAmount, param.filterCutoffAmount);
  addKnob(filterLeft + 5.0 * knobX, filterTop2, knobWidth, colorBlue, "To Res",
    ID::filterResonanceAmount, param.filterResonanceAmount);

  const auto filterTop3 = filterTop2 + knobY;
  addKnob(filterLeft, filterTop3, knobWidth, colorBlue, "Key->Cut", ID::filterKeyToCutoff,
    param.filterKeyToCutoff);
  addKnob(filterLeft + 1.0 * knobX, filterTop3, knobWidth, colorBlue, "Key->Feed",
    ID::filterKeyToFeedback, param.filterKeyToFeedback);

  // Plugin name.
  const auto splashTop = filterTop2 + knobY + 2.0 * margin;
  const auto splashLeft = modLeft + 2.0 * knobX;
  addSplashScreen(CRect(splashLeft + 0.25 * knobX, splashTop, splashLeft + 3.75 * knobX,
                    splashTop + 40.0),
    CRect(viewRect.left + 20.0, viewRect.top + 20.0, viewRect.right - 20.0,
      viewRect.bottom - 20.0));

  return true;
}

void PlugEditor::addOscillatorSection(UTF8String label,
  double left,
  double top,
  ParamID tagGain,
  ParamID tagSemi,
  ParamID tagCent,
  ParamID tagSync,
  ParamID tagSyncType,
  ParamID tagPTROrder,
  ParamID tagPhase,
  ParamID tagPhaseLock,
  float defaultGain,
  float defaultSemi,
  float defaultCent,
  float defaultSync,
  float defaultPhase)
{
  addGroupLabel(left, top, 2.0 * knobX, label);

  top += labelHeight + 10.0;
  auto knobCenterX = margin + left + knobX / 2.0;
  addKnob(knobCenterX, top, knobWidth, colorBlue, "Gain", tagGain, defaultGain);

  auto oscTop2 = top + knobY;
  auto knobLeft = margin + left;
  addKnob(knobLeft, oscTop2, knobWidth, colorBlue, "Semi", tagSemi, defaultSemi);
  addKnob(knobLeft + knobX, oscTop2, knobWidth, colorBlue, "Cent", tagCent, defaultCent);

  auto oscTop3 = oscTop2 + knobY;
  auto syncKnobSize = 2.0 * knobHeight;
  auto oscMenuWidth = 2.0 * knobX;
  addKnob(left + (oscMenuWidth - syncKnobSize) / 2.0, oscTop3, syncKnobSize, colorBlue,
    "Sync", tagSync, defaultSync);

  auto oscTop4 = oscTop3 + syncKnobSize + labelY - 10.0;
  std::vector<UTF8String> syncOptions = {"Off", "Ratio", "Fixed-Master", "Fixed-Slave"};
  addOptionMenu(left, oscTop4, oscMenuWidth, tagSyncType, syncOptions);

  auto oscTop5 = oscTop4 + labelY - margin;
  std::vector<UTF8String> ptrOrderOptions = {"Order 0", "Order 1", "Order 2", "Order 3",
    "Order 4", "Order 5", "Order 6", "Order 7", "Order 8", "Order 9", "Order 10", "Sin"};
  addOptionMenu(left, oscTop5, oscMenuWidth, tagPTROrder, ptrOrderOptions);

  auto oscTop6 = oscTop5 + labelY;
  addKnob(knobLeft, oscTop6, knobWidth, colorBlue, "Phase", tagPhase, defaultPhase);
  addCheckbox(
    knobLeft + knobX, oscTop6 + (knobHeight - labelHeight) / 2.0, "Lock", tagPhaseLock);
}

void PlugEditor::close()
{
  if (!frame) return;
  frame->forget();
  frame = nullptr;
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

CMouseEventResult PlugEditor::onMouseDown(
  CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  if (!buttons.isRightButton()) return kMouseEventNotHandled;

  auto componentHandler = controller->getComponentHandler();
  if (componentHandler == nullptr) return kMouseEventNotHandled;

  FUnknownPtr<IComponentHandler3> handler(componentHandler);
  if (handler == nullptr) return kMouseEventNotHandled;

  auto control = dynamic_cast<CControl *>(frame->getViewAt(where));
  if (control == nullptr) return kMouseEventNotHandled;

  // Context menu will not popup when the control has negative tag.
  ParamID id = control->getTag();
  if (id < 1 || id >= LONG_MAX) return kMouseEventNotHandled;

  IContextMenu *menu = handler->createContextMenu(this, &id);
  if (menu == nullptr) return kMouseEventNotHandled;

  menu->popup(where.x, where.y);
  menu->release();
  return kMouseEventHandled;
}
void PlugEditor::addLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font)
{
  auto bottom = top + 20.0;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  if (font == nullptr)
    font = new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace);
  label->setFont(font);
  label->setHoriAlign(CHoriTxtAlign::kCenterText);
  label->setStyle(CTextLabel::kNoFrame);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
}

void PlugEditor::addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto bottom = top + labelHeight;

  auto label = new CTextLabel(CRect(left, top, left + width, bottom), UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(0);
  label->setFrameWidth(frameWidth);
  label->setFrameColor(colorBlack);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  frame->addView(label);
}

void PlugEditor::addSplashScreen(CRect buttonRect, CRect splashRect)
{
  auto credit = new CreditView(splashRect, nullptr);
  auto splash = new SplashLabel(buttonRect, this, -666, credit, "SyncSawSynth");
  splash->setHighlightColor(colorOrange);
  credit->setListener(splash);
  frame->addView(splash);
}

void PlugEditor::addVSlider(CCoord left,
  CCoord top,
  CColor valueColor,
  UTF8String name,
  ParamID tag,
  float defaultValue,
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  // width, height = 100, 270.

  auto right = left + 70.0;
  auto bottom = top + 230.0;

  auto slider = new Slider(
    CRect(left, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);
  slider->setSliderMode(CSliderMode::FreeClick);
  slider->setStyle(CSlider::kBottom | CSlider::kVertical);
  slider->setDrawStyle(CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
    | (drawFromCenter ? CSlider::kDrawValueFromCenter | CSlider::kDrawInverted : 0));
  slider->setBackColor(colorWhite);
  slider->setValueColor(valueColor);
  slider->setHighlightColor(valueColor);
  slider->setDefaultFrameColor(colorBlack);
  slider->setHighlightWidth(3.0);
  slider->setValueNormalized(controller->getParamNormalized(tag));
  slider->setDefaultValue(defaultValue);
  slider->setTooltipText(tooltip);
  frame->addView(slider);

  top = bottom + 10.0;
  bottom = top + 30.0;
  right = left + 70.0;

  auto label = new CTextLabel(CRect(left, top, right, bottom), UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  frame->addView(label);
}

void PlugEditor::addButton(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
  auto bottom = top + 30.0;

  auto button = new TextButton(
    CRect(left, top, right, bottom), this, tag, title, (CTextButton::Style)style);
  button->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  button->setTextColor(colorBlack);
  button->setTextColorHighlighted(colorBlack);
  button->setGradient(CGradient::create(0.0, 1.0, colorWhite, colorWhite));
  button->setGradientHighlighted(CGradient::create(0.0, 1.0, colorOrange, colorOrange));
  button->setHighlightColor(colorOrange);
  button->setFrameColorHighlighted(colorBlack);
  button->setFrameWidth(1.0);
  button->setRoundRadius(0.0);
  button->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(button);
}

void PlugEditor::addCheckbox(
  CCoord left, CCoord top, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + 100.0;
  auto bottom = top + 20.0;

  auto checkbox
    = new CheckBox(CRect(left, top, right, bottom), this, tag, title, nullptr, style);
  checkbox->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  checkbox->setFontColor(colorBlack);
  checkbox->setBoxFrameColor(colorBlack);
  checkbox->setBoxFillColor(colorWhite);
  checkbox->setCheckMarkColor(colorBlue);
  checkbox->sizeToFit();
  checkbox->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(checkbox);
}

void PlugEditor::addOptionMenu(CCoord left,
  CCoord top,
  CCoord width,
  ParamID tag,
  const std::vector<UTF8String> &items)
{
  auto right = left + width;
  auto bottom = top + 20.0;

  auto menu = new OptionMenu(CRect(left, top, right, bottom), this, tag, nullptr, nullptr,
    COptionMenu::kCheckStyle);
  for (const auto &item : items) menu->addEntry(item);
  menu->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  menu->setFontColor(colorBlack);
  menu->setBackColor(colorWhite);
  menu->setDefaultFrameColor(colorBlack);
  menu->setHighlightColor(colorBlue);
  menu->sizeToFit();
  menu->setValueNormalized(controller->getParamNormalized(tag));
  frame->addView(menu);
}

void PlugEditor::addKnob(CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  float defaultValue,
  LabelPosition labelPosition,
  UTF8StringPtr tooltip)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(defaultValue);
  knob->setTooltipText(tooltip);
  frame->addView(knob);

  switch (labelPosition) {
    default:
    case LabelPosition::bottom:
      top = bottom;
      bottom = top + 30.0;
      break;

    case LabelPosition::right:
      left = right + margin;
      right = left + 100.0;
      break;
  }

  auto label
    = new CTextLabel(CRect(left - 10.0, top, right + 10.0, bottom), UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  label->setStyle(CParamDisplay::Style::kNoFrame);
  label->setTextTruncateMode(CTextLabel::kTruncateNone);
  label->setFontColor(colorBlack);
  label->setBackColor(colorWhite);
  label->setTooltipText(tooltip);
  if (labelPosition == LabelPosition::right) label->sizeToFit();
  frame->addView(label);
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
