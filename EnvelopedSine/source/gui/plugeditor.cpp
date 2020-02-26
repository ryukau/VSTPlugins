// (c) 2019 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "plugeditor.hpp"
#include "x11runloop.hpp"

#include "barbox.hpp"
#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "textbutton.hpp"

#include <algorithm>

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

  IPlatformFrameConfig *config = nullptr;
#if LINUX
  X11::FrameConfig x11config;
  x11config.runLoop = VSTGUI::owned(new RunLoop(plugFrame));
  config = &x11config;
#endif
  frame->open(parent, platformType, config);

  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const float top0 = 20.0f;
  const float left0 = 20.0f;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0;
  addGroupLabel(gainLeft, gainTop, 2.0 * knobX, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Boost", ID::gainBoost);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain);

  // Pitch.
  const auto pitchTop0 = gainTop + labelY + knobY;
  const auto pitchLeft0 = left0;
  addGroupLabel(pitchLeft0, pitchTop0, 2.0f * knobX, "Pitch");
  addCheckbox(
    pitchLeft0, pitchTop0 + labelY - 2.0f * margin, knobX, "Add Aliasing", ID::aliasing);

  const auto pitchLabelWidth = knobX - 2.0f * margin;
  const auto pitchLeft1 = pitchLeft0 + margin;
  const auto pitchLeft2 = pitchLeft0 + knobX;

  const auto pitchTop1 = pitchTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(pitchLeft1, pitchTop1, pitchLabelWidth, "Octave");
  addTextKnob(
    pitchLeft2, pitchTop1, knobX, colorBlue, ID::masterOctave, Scales::masterOctave);

  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(pitchLeft1, pitchTop2, pitchLabelWidth, "Multiply");
  addTextKnob(
    pitchLeft2, pitchTop2, knobX, colorBlue, ID::pitchMultiply, Scales::pitchMultiply,
    false, 3);

  const auto pitchTop3 = pitchTop2 + labelY;
  addLabel(pitchLeft1, pitchTop3, pitchLabelWidth, "Modulo");
  addTextKnob(
    pitchLeft2, pitchTop3, knobX, colorBlue, ID::pitchModulo, Scales::pitchModulo, false,
    3);

  const auto pitchTop4 = pitchTop3 + labelY;
  addLabel(pitchLeft1, pitchTop4, pitchLabelWidth, "Expand");
  addTextKnob(
    pitchLeft2, pitchTop4, knobX, colorBlue, ID::overtoneExpand, Scales::overtoneExpand,
    false, 3);

  const auto pitchTop5 = pitchTop4 + labelY;
  addLabel(pitchLeft1, pitchTop5, pitchLabelWidth, "Shift");
  addTextKnob(
    pitchLeft2, pitchTop5, knobX, colorBlue, ID::overtoneShift, Scales::overtoneShift,
    false, 3);

  // Random.
  const auto randomTop0 = pitchTop0 + labelHeight + 6.0 * labelY - margin;
  const auto randomLeft0 = left0;
  const auto randomLeft1 = randomLeft0 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 2.0f * knobX, "Random");
  addCheckbox(
    randomLeft0, randomTop0 + labelY - 2.0f * margin, knobX, "Retrigger",
    ID::randomRetrigger);

  const auto randomTop1 = randomTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2.0f * margin, "Seed");
  addTextKnob(
    randomLeft1 - 2.0f * margin, randomTop1, knobX, colorBlue, ID::seed, Scales::seed);

  const auto randomTop2 = randomTop1 + labelY;
  addKnob(randomLeft0, randomTop2, knobWidth, colorBlue, "To Gain", ID::randomGain);
  addKnob(randomLeft1, randomTop2, knobWidth, colorBlue, "To Pitch", ID::randomFrequency);

  const auto randomTop3 = randomTop2 + knobY;
  addKnob(randomLeft0, randomTop3, knobWidth, colorBlue, "To Attack", ID::randomAttack);
  addKnob(randomLeft1, randomTop3, knobWidth, colorBlue, "To Decay", ID::randomDecay);

  const auto randomTop4 = randomTop3 + knobY;
  addKnob(randomLeft0, randomTop4, knobWidth, colorBlue, "To Sat.", ID::randomSaturation);
  addKnob(randomLeft1, randomTop4, knobWidth, colorBlue, "To Phase", ID::randomPhase);

  // Misc.
  const auto miscTop = randomTop4 + knobY;
  const auto miscLeft = left0;
  addGroupLabel(miscLeft, miscTop, 2.0f * knobX, "Misc.");

  addKnob(miscLeft, miscTop + labelY, knobWidth, colorBlue, "Smooth", ID::smoothness);

  const auto miscLeft0 = miscLeft + knobX - (checkboxWidth - knobWidth) / 2.0f;
  const auto miscTop0 = miscTop + labelY;
  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(miscLeft0, miscTop0, checkboxWidth, ID::nVoice, nVoiceOptions);
  addCheckbox(miscLeft0, miscTop0 + labelY, checkboxWidth, "Unison", ID::unison);

  // Modifier.
  const auto modTop = top0 + 4.0f * (barboxY + margin);
  const auto modLeft = left0 + 2.0f * knobX + 4.0f * margin + labelY;
  addGroupLabel(modLeft, modTop, 4.0f * knobX, "Modifier");

  const auto modTop0 = modTop + labelY;
  addKnob(modLeft, modTop0, knobWidth, colorBlue, "Attack*", ID::attackMultiplier);
  addKnob(modLeft + knobX, modTop0, knobWidth, colorBlue, "Decay*", ID::decayMultiplier);
  addKnob(modLeft + 2.0f * knobX, modTop0, knobWidth, colorBlue, "Gain^", ID::gainPower);
  addKnob(
    modLeft + 3.0f * knobX, modTop0, knobWidth, colorBlue, "Sat. Mix", ID::saturationMix);

  const auto modTop1 = modTop0 + knobY;
  addCheckbox(modLeft + 0.4f * knobX, modTop1, checkboxWidth, "Declick", ID::declick);

  // Phaser.
  const auto phaserTop = modTop;
  const auto phaserLeft = modLeft + 4.0f * knobX + 4.0f * margin;
  addGroupLabel(phaserLeft, phaserTop, 7.0f * knobX + labelY, "Phaser");

  const auto phaserTop0 = phaserTop + labelY;
  addKnob(phaserLeft, phaserTop0, knobWidth, colorBlue, "Mix", ID::phaserMix);
  addKnob(
    phaserLeft + knobX, phaserTop0, knobWidth, colorBlue, "Freq", ID::phaserFrequency);
  addKnob(
    phaserLeft + 2.0f * knobX, phaserTop0, knobWidth, colorRed, "Feedback",
    ID::phaserFeedback);
  addKnob(
    phaserLeft + 3.0f * knobX, phaserTop0, knobWidth, colorBlue, "Range",
    ID::phaserRange);
  addKnob(
    phaserLeft + 4.0f * knobX, phaserTop0, knobWidth, colorBlue, "Min", ID::phaserMin);
  addKnob(
    phaserLeft + 5.0f * knobX, phaserTop0, knobWidth, colorBlue, "Offset",
    ID::phaserOffset);
  addRotaryKnob(
    phaserLeft + 6.0f * knobX, phaserTop0, knobWidth + labelY, colorBlue, "Phase",
    ID::phaserPhase);

  const auto phaserTop1 = phaserTop0 + knobY;
  std::vector<UTF8String> phaserStageItems{
    "Stage 1",  "Stage 2",  "Stage 3",  "Stage 4",  "Stage 5",  "Stage 6",
    "Stage 7",  "Stage 8",  "Stage 9",  "Stage 10", "Stage 11", "Stage 12",
    "Stage 13", "Stage 14", "Stage 15", "Stage 16"};
  addOptionMenu(
    phaserLeft - margin, phaserTop1, knobX, ID::phaserStage, phaserStageItems);

  // Attack.
  const auto attackTop = top0;
  const auto attackLeft = left0 + 2.0f * knobX + 4.0f * margin;
  // addGroupVerticalLabel(attackLeft, attackTop, barboxHeight, "Attack");

  const auto attackLeft0 = attackLeft + labelY;
  addBarBox(attackLeft0, attackTop, barboxWidth, barboxHeight, ID::attack0, "Attack");

  // Decay.
  const auto decayTop = attackTop + barboxY + margin;
  const auto decayLeft = attackLeft;
  // addGroupVerticalLabel(decayLeft, decayTop, barboxHeight, "Decay");

  const auto decayLeft0 = decayLeft + labelY;
  addBarBox(decayLeft0, decayTop, barboxWidth, barboxHeight, ID::decay0, "Decay");

  // Overtone.
  const auto overtoneTop = decayTop + barboxY + margin;
  const auto overtoneLeft = attackLeft;
  // addGroupVerticalLabel(overtoneLeft, overtoneTop, barboxHeight, "Gain");

  const auto overtoneLeft0 = overtoneLeft + labelY;
  addBarBox(overtoneLeft0, overtoneTop, barboxWidth, barboxHeight, ID::overtone0, "Gain");

  // Saturation.
  const auto saturationTop = overtoneTop + barboxY + margin;
  const auto saturationLeft = attackLeft;
  // addGroupVerticalLabel(saturationLeft, saturationTop, barboxHeight, "Saturation");

  const auto saturationLeft0 = saturationLeft + labelY;
  addBarBox(
    saturationLeft0, saturationTop, barboxWidth, barboxHeight, ID::saturation0,
    "Saturation");

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 20.0f;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, 2.5f * knobX, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, "EnvelopedSine");

  return true;
}

void PlugEditor::close()
{
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }
}

void PlugEditor::valueChanged(CControl *pControl)
{
  ParamID tag = pControl->getTag();
  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(tag, value);
  controller->performEdit(tag, value);
}

void PlugEditor::valueChanged(ParamID id, ParamValue normalized)
{
  controller->setParamNormalized(id, normalized);
  controller->performEdit(id, normalized);
}

void PlugEditor::updateUI(Vst::ParamID id, ParamValue normalized)
{
  auto iter = controlMap.find(id);
  if (iter != controlMap.end()) {
    iter->second->setValueNormalized(normalized);
    iter->second->invalid();
    return;
  }

  using ID = Synth::ParameterID::ID;

  for (auto &ctrl : arrayControls) {
    if (id < ctrl->id.front() && id > ctrl->id.back()) continue;
    ctrl->setValueAt(id - ctrl->id.front(), normalized);
    ctrl->invalid();
  }
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

CMouseEventResult
PlugEditor::onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons)
{
  return kMouseEventNotHandled;
}

void PlugEditor::addBarBox(
  CCoord left, CCoord top, CCoord width, CCoord height, ParamID id0, UTF8String name)
{
  std::vector<ParamID> id(64);
  for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);

  std::vector<double> value(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    value[i] = controller->getParamNormalized(id[i]);

  std::vector<double> defaultValue(id.size());
  for (size_t i = 0; i < value.size(); ++i)
    defaultValue[i] = param.value[id[i]]->getDefaultNormalized();

  auto barBox = new BarBox(
    CRect(left, top, left + width, top + height), this, id, value, defaultValue);
  barBox->setIndexFont(new CFontDesc(Style::fontName(), 10.0, CTxtFace::kBoldFace));
  barBox->setNameFont(new CFontDesc(Style::fontName(), 24.0, CTxtFace::kNormalFace));
  barBox->setBorderColor(colorBlack);
  barBox->setValueColor(colorBlue);
  barBox->setName(name);
  frame->addView(barBox);

  auto iter = std::find_if(
    arrayControls.begin(), arrayControls.end(),
    [&](const ArrayControl *elem) { return elem->id[0] == id0; });
  if (iter != arrayControls.end()) {
    (*iter)->forget();
    arrayControls.erase(iter);
  }
  barBox->remember();
  arrayControls.push_back(barBox);
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

void PlugEditor::addGroupVerticalLabel(
  CCoord left, CCoord top, CCoord width, UTF8String name)
{
  auto label = new VGroupLabel(
    CRect(left, top, left + labelHeight, top + width), this, UTF8String(name));
  label->setFont(new CFontDesc(Style::fontName(), 14.0, CTxtFace::kBoldFace));
  frame->addView(label);
};

void PlugEditor::addSplashScreen(
  float buttonLeft,
  float buttonTop,
  float buttonWidth,
  float buttonHeight,
  float splashLeft,
  float splashTop,
  float splashWidth,
  float splashHeight,
  const char *pluginName)
{
  auto credit = new CreditView(
    CRect(splashLeft, splashTop, splashLeft + splashWidth, splashTop + splashHeight),
    this);
  auto splash = new SplashLabel(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, 0, credit, pluginName);
  splash->setHighlightColor(colorOrange);
  frame->addView(splash);
  frame->addView(credit);
}

void PlugEditor::addVSlider(
  CCoord left,
  CCoord top,
  CColor valueColor,
  UTF8String name,
  ParamID tag,
  UTF8StringPtr tooltip,
  bool drawFromCenter)
{
  auto right = left + 70.0f;
  auto bottom = top + 230.0f;

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
  slider->setDefaultValue(param.value[tag]->getDefaultNormalized());
  slider->setTooltipText(tooltip);
  frame->addView(slider);
  addToControlMap(tag, slider);

  top = bottom + margin;
  bottom = top + labelHeight;

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
  auto bottom = top + labelHeight;

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
  addToControlMap(tag, button);
}

void PlugEditor::addCheckbox(
  CCoord left, CCoord top, CCoord width, UTF8String title, ParamID tag, int32_t style)
{
  auto right = left + width;
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
  addToControlMap(tag, checkbox);
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
  addToControlMap(tag, menu);
}

void PlugEditor::addKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new Knob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

void PlugEditor::addRotaryKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  UTF8String name,
  ParamID tag,
  LabelPosition labelPosition)
{
  auto bottom = top + width - 10.0;
  auto right = left + width;

  auto knob = new RotaryKnob(CRect(left + 5.0, top, right - 5.0, bottom), this, tag);
  knob->setSlitWidth(8.0);
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

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
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  frame->addView(knob);
  addToControlMap(tag, knob);

  addKnobLabel(left, top, right, bottom, name, labelPosition);
}

void PlugEditor::addKnobLabel(
  CCoord left,
  CCoord top,
  CCoord right,
  CCoord bottom,
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

template<typename Scale>
void PlugEditor::addTextKnob(
  CCoord left,
  CCoord top,
  CCoord width,
  CColor highlightColor,
  ParamID tag,
  Scale &scale,
  bool isDecibel,
  uint32_t precision)
{
  auto bottom = top + labelHeight;
  auto right = left + width;

  auto knob
    = new TextKnob<Scale>(CRect(left, top, right, bottom), this, tag, scale, isDecibel);
  knob->setFont(new CFontDesc(Style::fontName(), fontSize, CTxtFace::kNormalFace));
  knob->setHighlightColor(highlightColor);
  knob->setValueNormalized(controller->getParamNormalized(tag));
  knob->setDefaultValue(param.value[tag]->getDefaultNormalized());
  knob->setPrecision(precision);
  frame->addView(knob);
  addToControlMap(tag, knob);
}

ParamValue PlugEditor::getPlainValue(ParamID tag)
{
  auto normalized = controller->getParamNormalized(tag);
  return controller->normalizedParamToPlain(tag, normalized);
};

} // namespace Vst
} // namespace Steinberg
