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
#include "grouplabel.hpp"
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

  using ID = Synth::ParameterID::ID;
  Synth::GlobalParameter param;

  // Oscillators.
  const auto oscWidth = 2.0 * knobWidth + 4.0 * margin;
  const auto oscTop = 10.0;
  const auto oscLeft1 = 10.0;
  addOscillatorSection(
    "Osc1", oscLeft1, oscTop, ID::osc1Gain, ID::osc1Semi, ID::osc1Cent, ID::osc1Sync,
    ID::osc1SyncType, ID::osc1PTROrder, ID::osc1Phase, ID::osc1PhaseLock,
    param.value[ID::osc1Gain]->getDefaultNormalized(),
    param.value[ID::osc1Semi]->getDefaultNormalized(),
    param.value[ID::osc1Cent]->getDefaultNormalized(),
    param.value[ID::osc1Sync]->getDefaultNormalized(),
    param.value[ID::osc1Phase]->getDefaultNormalized());

  const auto oscLeft2 = oscLeft1 + oscWidth + 10.0;
  addOscillatorSection(
    "Osc2", oscLeft2, oscTop, ID::osc2Gain, ID::osc2Semi, ID::osc2Cent, ID::osc2Sync,
    ID::osc2SyncType, ID::osc2PTROrder, ID::osc2Phase, ID::osc2PhaseLock,
    param.value[ID::osc2Gain]->getDefaultNormalized(),
    param.value[ID::osc2Semi]->getDefaultNormalized(),
    param.value[ID::osc2Cent]->getDefaultNormalized(),
    param.value[ID::osc2Sync]->getDefaultNormalized(),
    param.value[ID::osc2Phase]->getDefaultNormalized());

  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(oscLeft2 - knobX / 2.0, oscTop + labelY, 60.0, ID::nVoice, nVoiceOptions);
  addCheckbox(oscLeft2 - knobX / 2.0, oscTop + 2.0 * labelY, "Unison", ID::unison);

  const auto oscTop2 = 4.0 * labelY + 2.0 * knobY + 2.0 * knobHeight - margin;
  addCheckbox(
    oscLeft2 + margin + knobX, oscTop2 + labelY + (knobHeight - labelHeight) / 2.0 - 10.0,
    "Invert", ID::osc2Invert);

  // Cross modulation.
  const auto crossTop = oscTop2 + knobY;
  const auto crossLeft = oscLeft1;
  addGroupLabel(crossLeft, crossTop, 2.0 * oscWidth + 10.0, "Modulation");

  const auto crossTop2 = crossTop + labelY;
  const auto crossKnobLeft = margin + crossLeft;
  addKnob(
    crossKnobLeft, crossTop2, 1.5 * knobWidth, colorBlue, "Osc1->Sync1",
    ID::fmOsc1ToSync1, param.value[ID::fmOsc1ToSync1]->getDefaultNormalized());
  addKnob(
    crossKnobLeft + 1.5 * knobWidth + 10.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc1->Freq2", ID::fmOsc1ToFreq2,
    param.value[ID::fmOsc1ToFreq2]->getDefaultNormalized());
  addKnob(
    crossKnobLeft + 3.0 * knobWidth + 20.0, crossTop2, 1.5 * knobWidth, colorBlue,
    "Osc2->Sync1", ID::fmOsc2ToSync1,
    param.value[ID::fmOsc2ToSync1]->getDefaultNormalized());

  // Modulation envelope and LFO.
  const auto modTop = oscTop;
  const auto modLeft = oscLeft2 + oscWidth + 20.0;
  addGroupLabel(modLeft, modTop, 6.0 * knobX, "Modulation");

  const auto modTop1 = modTop + labelY;
  addKnob(
    modLeft, modTop1, knobWidth, colorBlue, "Attack", ID::modEnvelopeA,
    param.value[ID::modEnvelopeA]->getDefaultNormalized());
  addKnob(
    modLeft + 1.0 * knobX, modTop1, knobWidth, colorBlue, "Curve", ID::modEnvelopeCurve,
    param.value[ID::modEnvelopeCurve]->getDefaultNormalized());
  addKnob(
    modLeft + 2.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq1",
    ID::modEnvelopeToFreq1, param.value[ID::modEnvelopeToFreq1]->getDefaultNormalized());
  addKnob(
    modLeft + 3.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync1",
    ID::modEnvelopeToSync1, param.value[ID::modEnvelopeToSync1]->getDefaultNormalized());
  addKnob(
    modLeft + 4.0 * knobX, modTop1, knobWidth, colorBlue, "To Freq2",
    ID::modEnvelopeToFreq2, param.value[ID::modEnvelopeToFreq2]->getDefaultNormalized());
  addKnob(
    modLeft + 5.0 * knobX, modTop1, knobWidth, colorBlue, "To Sync2",
    ID::modEnvelopeToSync2, param.value[ID::modEnvelopeToSync2]->getDefaultNormalized());

  const auto modTop2 = modTop1 + knobY;
  addKnob(
    modLeft, modTop2, knobWidth, colorBlue, "LFO", ID::modLFOFrequency,
    param.value[ID::modLFOFrequency]->getDefaultNormalized());
  addKnob(
    modLeft + 1.0 * knobX, modTop2, knobWidth, colorBlue, "NoiseMix", ID::modLFONoiseMix,
    param.value[ID::modLFONoiseMix]->getDefaultNormalized());
  addKnob(
    modLeft + 2.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq1", ID::modLFOToFreq1,
    param.value[ID::modLFOToFreq1]->getDefaultNormalized());
  addKnob(
    modLeft + 3.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync1", ID::modLFOToSync1,
    param.value[ID::modLFOToSync1]->getDefaultNormalized());
  addKnob(
    modLeft + 4.0 * knobX, modTop2, knobWidth, colorBlue, "To Freq2", ID::modLFOToFreq2,
    param.value[ID::modLFOToFreq2]->getDefaultNormalized());
  addKnob(
    modLeft + 5.0 * knobX, modTop2, knobWidth, colorBlue, "To Sync2", ID::modLFOToSync2,
    param.value[ID::modLFOToSync2]->getDefaultNormalized());

  // Gain.
  const auto gainTop = modTop2 + knobY + margin;
  const auto gainLeft = modLeft;
  addGroupLabel(gainLeft, gainTop, 6.0 * knobX, "Gain");

  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain,
    param.value[ID::gain]->getDefaultNormalized());
  addKnob(
    gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA,
    param.value[ID::gainA]->getDefaultNormalized());
  addKnob(
    gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD,
    param.value[ID::gainD]->getDefaultNormalized());
  addKnob(
    gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS,
    param.value[ID::gainS]->getDefaultNormalized());
  addKnob(
    gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR,
    param.value[ID::gainR]->getDefaultNormalized());
  addKnob(
    gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
    ID::gainEnvelopeCurve, param.value[ID::gainEnvelopeCurve]->getDefaultNormalized());

  // Filter.
  const auto filterTop = gainKnobTop + knobY + margin;
  const auto filterLeft = modLeft;
  addGroupLabel(filterLeft, filterTop, 4.0 * knobX - 10.0, "Filter");
  addCheckbox(filterLeft + 4.0 * knobX, filterTop, "Dirty Buffer", ID::filterDirty);

  const auto filterTop1 = filterTop + labelY;
  addKnob(
    filterLeft, filterTop1, knobWidth, colorBlue, "Cut", ID::filterCutoff,
    param.value[ID::filterCutoff]->getDefaultNormalized());
  addKnob(
    filterLeft + 1.0 * knobX, filterTop1, knobWidth, colorBlue, "Res",
    ID::filterResonance, param.value[ID::filterResonance]->getDefaultNormalized());
  addKnob(
    filterLeft + 2.0 * knobX, filterTop1, knobWidth, colorBlue, "Feed",
    ID::filterFeedback, param.value[ID::filterFeedback]->getDefaultNormalized());
  addKnob(
    filterLeft + 3.0 * knobX, filterTop1, knobWidth, colorBlue, "Sat",
    ID::filterSaturation, param.value[ID::filterSaturation]->getDefaultNormalized());

  const auto filterMenuWidth = 100.0;
  std::vector<UTF8String> filterTypeOptions = {"LP", "HP", "BP", "Notch", "Bypass"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1, filterMenuWidth, ID::filterType,
    filterTypeOptions);
  std::vector<UTF8String> filterShaperOptions
    = {"HardClip", "Tanh", "ShaperA", "ShaperB"};
  addOptionMenu(
    filterLeft + 4.0 * knobX, filterTop1 + labelY, filterMenuWidth, ID::filterShaper,
    filterShaperOptions);

  const auto filterTop2 = filterTop1 + knobY;
  addKnob(
    filterLeft, filterTop2, knobWidth, colorBlue, "A", ID::filterA,
    param.value[ID::filterA]->getDefaultNormalized());
  addKnob(
    filterLeft + 1.0 * knobX, filterTop2, knobWidth, colorBlue, "D", ID::filterD,
    param.value[ID::filterD]->getDefaultNormalized());
  addKnob(
    filterLeft + 2.0 * knobX, filterTop2, knobWidth, colorBlue, "S", ID::filterS,
    param.value[ID::filterS]->getDefaultNormalized());
  addKnob(
    filterLeft + 3.0 * knobX, filterTop2, knobWidth, colorBlue, "R", ID::filterR,
    param.value[ID::filterR]->getDefaultNormalized());
  addKnob(
    filterLeft + 4.0 * knobX, filterTop2, knobWidth, colorBlue, "To Cut",
    ID::filterCutoffAmount, param.value[ID::filterCutoffAmount]->getDefaultNormalized());
  addKnob(
    filterLeft + 5.0 * knobX, filterTop2, knobWidth, colorBlue, "To Res",
    ID::filterResonanceAmount,
    param.value[ID::filterResonanceAmount]->getDefaultNormalized());

  const auto filterTop3 = filterTop2 + knobY;
  addKnob(
    filterLeft, filterTop3, knobWidth, colorBlue, "Key->Cut", ID::filterKeyToCutoff,
    param.value[ID::filterKeyToCutoff]->getDefaultNormalized());
  addKnob(
    filterLeft + 1.0 * knobX, filterTop3, knobWidth, colorBlue, "Key->Feed",
    ID::filterKeyToFeedback,
    param.value[ID::filterKeyToFeedback]->getDefaultNormalized());

  // Plugin name.
  const auto splashTop = filterTop2 + knobY + 2.0 * margin;
  const auto splashLeft = modLeft + 2.0 * knobX;
  addSplashScreen(
    CRect(
      splashLeft + 0.25 * knobX, splashTop, splashLeft + 3.75 * knobX, splashTop + 40.0),
    CRect(
      viewRect.left + 20.0, viewRect.top + 20.0, viewRect.right - 20.0,
      viewRect.bottom - 20.0));

  return true;
}

void PlugEditor::addOscillatorSection(
  UTF8String label,
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
  addNumberKnob(
    knobLeft, oscTop2, knobWidth, colorBlue, "Semi", tagSemi, Synth::Scales::semi, 0,
    defaultSemi);
  addKnob(knobLeft + knobX, oscTop2, knobWidth, colorBlue, "Cent", tagCent, defaultCent);

  auto oscTop3 = oscTop2 + knobY;
  auto syncKnobSize = 2.0 * knobHeight;
  auto oscMenuWidth = 2.0 * knobX;
  addKnob(
    left + (oscMenuWidth - syncKnobSize) / 2.0, oscTop3, syncKnobSize, colorBlue, "Sync",
    tagSync, defaultSync);

  auto oscTop4 = oscTop3 + syncKnobSize + labelY - 10.0;
  std::vector<UTF8String> syncOptions = {"Off", "Ratio", "Fixed-Master", "Fixed-Slave"};
  addOptionMenu(left, oscTop4, oscMenuWidth, tagSyncType, syncOptions);

  auto oscTop5 = oscTop4 + labelY - margin;
  std::vector<UTF8String> ptrOrderOptions
    = {"Order 0",        "Order 1",        "Order 2",        "Order 3",
       "Order 4",        "Order 5",        "Order 6",        "Order 7",
       "Order 8",        "Order 9",        "Order 10",       "Sin",
       "Order 6 double", "Order 7 double", "Order 8 double", "Order 9 double",
       "Order 10 double"};
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

CMouseEventResult
PlugEditor::onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons)
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

  auto label
    = new GroupLabel(CRect(left, top, left + width, bottom), this, UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), 14.0, CTxtFace::kBoldFace));
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

void PlugEditor::addVSlider(
  CCoord left,
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
  slider->setDrawStyle(
    CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
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

void PlugEditor::addOptionMenu(
  CCoord left,
  CCoord top,
  CCoord width,
  ParamID tag,
  const std::vector<UTF8String> &items)
{
  auto right = left + width;
  auto bottom = top + 20.0;

  auto menu = new OptionMenu(
    CRect(left, top, right, bottom), this, tag, nullptr, nullptr,
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

void PlugEditor::addKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  float defaultValue,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(defaultValue);
  frame->addView(knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

template<typename Scale>
void PlugEditor::addNumberKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  Scale &scale,
  int32_t offset,
  float defaultValue,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new NumberKnob<Scale>(
    CRect(left + 5.0, top, right - 5.0, bottom), this, tag, scale, offset);
  knob->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(defaultValue);
  frame->addView(knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

void PlugEditor::addKnobLabel(
  float left,
  float top,
  float right,
  float bottom,
  UTF8String name,
  LabelPosition labelPosition)
{
  switch (labelPosition) {
    default:
    case LabelPosition::bottom:
      top = bottom;
      bottom = top + 30.0;
      left -= 10.0;
      right += 10.0;
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
  label->setBackColor(CColor{0, 0, 0, 0});
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
