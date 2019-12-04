// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#include "vstgui4/vstgui/lib/platform/iplatformfont.h"

#include "barbox.hpp"
#include "checkbox.hpp"
#include "grouplabel.hpp"
#include "guistyle.hpp"
#include "knob.hpp"
#include "optionmenu.hpp"
#include "plugeditor.hpp"
#include "rotaryknob.hpp"
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
  using Scales = Synth::Scales;

  const float top0 = 20.0f;
  const float left0 = 20.0f;

  // Gain.
  const auto gainTop = top0;
  const auto gainLeft = left0 + knobX;
  addGroupLabel(gainLeft, gainTop, 7.0 * knobX, "Gain");
  const auto gainKnobTop = gainTop + labelY;
  addKnob(
    gainLeft + 0.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Boost", ID::gainBoost);
  addKnob(gainLeft + 1.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain);
  addKnob(gainLeft + 2.0 * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA);
  addKnob(gainLeft + 3.0 * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD);
  addKnob(gainLeft + 4.0 * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS);
  addKnob(gainLeft + 5.0 * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR);
  addKnob(
    gainLeft + 6.0 * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
    ID::gainEnvelopeCurve);

  // Random.
  const float randomTop0 = top0;
  const float randomLeft0 = left0 + 8.0f * knobX + 4.0f * margin;
  const float randomLeft1 = randomLeft0 + knobX;
  const float randomLeft2 = randomLeft1 + knobX;
  addGroupLabel(randomLeft0, randomTop0, 4.0f * knobX, "Random");
  addCheckbox(
    randomLeft0 + 8.0f, randomTop0 + labelY - 2.0f * margin, knobX, "Retrigger",
    ID::randomRetrigger);

  const float randomTop1 = randomTop0 + 2.0f * labelY - 3.0f * margin;
  addLabel(randomLeft0, randomTop1, knobX - 2.0f * margin, "Seed");
  addTextKnob(
    randomLeft1 - 2.0f * margin, randomTop1, knobX, colorBlue, ID::seed, Scales::seed);
  addKnob(
    randomLeft2, randomTop0 + labelY, knobWidth, colorBlue, "To Gain",
    ID::randomGainAmount);
  addKnob(
    randomLeft2 + knobX, randomTop0 + labelY, knobWidth, colorBlue, "To Pitch",
    ID::randomFrequencyAmount);

  // Shelving.
  const auto filterTop0 = gainTop + knobY + labelY;
  const auto filterLeft0 = left0;
  const auto filterLeft1 = filterLeft0 + knobX;
  const auto filterLeft2 = filterLeft1 + knobX;
  const auto filterTop1 = filterTop0 + labelY - 2.0f * margin;
  const auto filterTop2 = filterTop1 + labelY - margin;
  const auto filterTop3 = filterTop2 + labelY;
  addGroupLabel(filterLeft1, filterTop0, 2.0f * knobX, "Shelving");
  addLabel(filterLeft0, filterTop2, knobX, "Gain [dB]");
  addLabel(filterLeft0, filterTop3, knobX, "Semi");

  addLabel(filterLeft1, filterTop1, knobX, "Low");
  addTextKnob(
    filterLeft1, filterTop2, knobX, colorBlue, ID::lowShelfGain, Scales::shelvingGain,
    true, 2);
  addTextKnob(
    filterLeft1, filterTop3, knobX, colorBlue, ID::lowShelfPitch, Scales::shelvingPitch,
    false, 3);

  addLabel(filterLeft2, filterTop1, knobX, "High");
  addTextKnob(
    filterLeft2, filterTop2, knobX, colorBlue, ID::highShelfGain, Scales::shelvingGain,
    true, 2);
  addTextKnob(
    filterLeft2, filterTop3, knobX, colorBlue, ID::highShelfPitch, Scales::shelvingPitch,
    false, 3);

  // Pitch.
  const auto pitchTop0 = top0 + knobY + labelY;
  const auto pitchLeft0 = left0 + 3.0f * knobX + 4.0f * margin;
  addGroupLabel(pitchLeft0, pitchTop0, 4.0f * knobX, "Pitch");
  addCheckbox(
    pitchLeft0 + 0.0f * knobX, pitchTop0 + labelY - 2.0f * margin, knobX, "Add Aliasing",
    ID::aliasing);
  addCheckbox(
    pitchLeft0 + 1.8f * knobX, pitchTop0 + labelY - 2.0f * margin, knobX, "Reverse Semi",
    ID::negativeSemi);

  const auto pitchTop1 = pitchTop0 + 2.0f * labelY - 3.0f * margin;
  const auto pitchTop2 = pitchTop1 + labelY;
  addLabel(pitchLeft0 + margin, pitchTop1, knobX - 2.0f * margin, "Octave");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop1, knobX, colorBlue, ID::masterOctave,
    Scales::masterOctave);
  addLabel(pitchLeft0 + margin, pitchTop2, knobX - 2.0f * margin, "ET");
  addTextKnob(
    pitchLeft0 + knobX, pitchTop2, knobX, colorBlue, ID::equalTemperament,
    Scales::equalTemperament);

  const auto pitchLeft1 = pitchLeft0 + 2.1f * knobX;
  addLabel(pitchLeft1 + margin, pitchTop1, knobX - 2.0f * margin, "Multiply");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop1, knobX, colorBlue, ID::pitchMultiply,
    Scales::pitchMultiply, false, 3);

  addLabel(pitchLeft1 + margin, pitchTop2, knobX - 2.0f * margin, "Modulo");
  addTextKnob(
    pitchLeft1 + knobX, pitchTop2, knobX, colorBlue, ID::pitchModulo, Scales::pitchModulo,
    false, 3);

  // Misc.
  const auto miscLeft = pitchLeft0 + 4.5f * knobX + 2.0f * margin;
  addKnob(miscLeft, pitchTop0, knobWidth, colorBlue, "Smooth", ID::smoothness);
  std::vector<UTF8String> nVoiceOptions
    = {"Mono", "2 Voices", "4 Voices", "8 Voices", "16 Voices", "32 Voices"};
  addOptionMenu(
    miscLeft - (checkboxWidth - knobWidth) / 2.0f, pitchTop0 + knobY, checkboxWidth,
    ID::nVoice, nVoiceOptions);

  // Chorus.
  const float chorusTop0 = filterTop0;
  const float chorusLeft0 = randomLeft0 + knobX;
  const float chorusLeft1 = chorusLeft0 + knobX;
  const float chorusLeft2 = chorusLeft1 + knobX;
  addGroupLabel(chorusLeft0, chorusTop0, 3.0f * knobX, "Chorus");

  const float chorusTop1 = chorusTop0 + labelY;
  addKnob(chorusLeft0, chorusTop1, knobWidth, colorBlue, "Mix", ID::chorusMix);
  addKnob(chorusLeft1, chorusTop1, knobWidth, colorBlue, "Freq", ID::chorusFrequency);
  addKnob(chorusLeft2, chorusTop1, knobWidth, colorBlue, "Depth", ID::chorusDepth);

  const float chorusTop2 = chorusTop1 + knobY;
  addKnob(
    chorusLeft0, chorusTop2, knobWidth, colorBlue, "Range0", ID::chorusDelayTimeRange0);
  addKnob(
    chorusLeft1, chorusTop2, knobWidth, colorBlue, "Range1", ID::chorusDelayTimeRange1);
  addKnob(
    chorusLeft2, chorusTop2, knobWidth, colorBlue, "Range2", ID::chorusDelayTimeRange2);

  const float chorusTop3 = chorusTop2 + knobY;
  addKnob(
    chorusLeft0, chorusTop3, knobWidth, colorBlue, "Time0", ID::chorusMinDelayTime0);
  addKnob(
    chorusLeft1, chorusTop3, knobWidth, colorBlue, "Time1", ID::chorusMinDelayTime1);
  addKnob(
    chorusLeft2, chorusTop3, knobWidth, colorBlue, "Time2", ID::chorusMinDelayTime2);

  const float chorusTop4 = chorusTop3 + knobY;
  addRotaryKnob(chorusLeft0, chorusTop4, knobWidth, colorBlue, "Phase", ID::chorusPhase);
  addKnob(chorusLeft1, chorusTop4, knobWidth, colorBlue, "Offset", ID::chorusOffset);
  addKnob(chorusLeft2, chorusTop4, knobWidth, colorBlue, "Feedback", ID::chorusFeedback);

  const float chorusTop5 = chorusTop4 + knobY;
  addCheckbox(
    chorusLeft0 + 0.8f * knobX, chorusTop5 + margin, checkboxWidth, "Key Follow",
    ID::chorusKeyFollow);

  // Note.
  const float noteTop0 = filterTop0 + knobY + labelY;
  addGroupLabel(left0 + knobX, noteTop0, 8.0f * knobX, "Note");

  const float noteTop1 = noteTop0 + labelY;
  addLabel(left0, noteTop1, knobX, "Gain [dB]");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop1, knobX, colorBlue, ID::gain0, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop1, knobX, colorBlue, ID::gain1, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop1, knobX, colorBlue, ID::gain2, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop1, knobX, colorBlue, ID::gain3, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop1, knobX, colorBlue, ID::gain4, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop1, knobX, colorBlue, ID::gain5, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop1, knobX, colorBlue, ID::gain6, Scales::gainDecibel,
    true, 2);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop1, knobX, colorBlue, ID::gain7, Scales::gainDecibel,
    true, 2);

  const float noteTop2 = noteTop1 + labelY;
  addLabel(left0, noteTop2, knobX, "Semi");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop2, knobX, colorBlue, ID::semi0, Scales::oscSemi);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop2, knobX, colorBlue, ID::semi1, Scales::oscSemi);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop2, knobX, colorBlue, ID::semi2, Scales::oscSemi);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop2, knobX, colorBlue, ID::semi3, Scales::oscSemi);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop2, knobX, colorBlue, ID::semi4, Scales::oscSemi);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop2, knobX, colorBlue, ID::semi5, Scales::oscSemi);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop2, knobX, colorBlue, ID::semi6, Scales::oscSemi);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop2, knobX, colorBlue, ID::semi7, Scales::oscSemi);

  const float noteTop3 = noteTop2 + labelY;
  addLabel(left0, noteTop3, knobX, "Milli");
  addTextKnob(
    left0 + 1.0f * knobX, noteTop3, knobX, colorBlue, ID::milli0, Scales::oscMilli);
  addTextKnob(
    left0 + 2.0f * knobX, noteTop3, knobX, colorBlue, ID::milli1, Scales::oscMilli);
  addTextKnob(
    left0 + 3.0f * knobX, noteTop3, knobX, colorBlue, ID::milli2, Scales::oscMilli);
  addTextKnob(
    left0 + 4.0f * knobX, noteTop3, knobX, colorBlue, ID::milli3, Scales::oscMilli);
  addTextKnob(
    left0 + 5.0f * knobX, noteTop3, knobX, colorBlue, ID::milli4, Scales::oscMilli);
  addTextKnob(
    left0 + 6.0f * knobX, noteTop3, knobX, colorBlue, ID::milli5, Scales::oscMilli);
  addTextKnob(
    left0 + 7.0f * knobX, noteTop3, knobX, colorBlue, ID::milli6, Scales::oscMilli);
  addTextKnob(
    left0 + 8.0f * knobX, noteTop3, knobX, colorBlue, ID::milli7, Scales::oscMilli);

  // Chord.
  const float topChord0 = noteTop0 + 4.0f * labelY;
  addGroupLabel(left0 + knobX, topChord0, 4.0f * knobX, "Chord");

  const float topChord1 = topChord0 + labelY;
  addLabel(left0, topChord1, knobX, "Gain [dB]");
  addTextKnob(
    left0 + 1.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain0,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 2.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain1,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 3.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain2,
    Scales::gainDecibel, true, 2);
  addTextKnob(
    left0 + 4.0f * knobX, topChord1, knobX, colorBlue, ID::chordGain3,
    Scales::gainDecibel, true, 2);

  const float topChord2 = topChord1 + labelY;
  addLabel(left0, topChord2, knobX, "Semi");
  addTextKnob(
    left0 + 1.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi0, Scales::oscSemi);
  addTextKnob(
    left0 + 2.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi1, Scales::oscSemi);
  addTextKnob(
    left0 + 3.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi2, Scales::oscSemi);
  addTextKnob(
    left0 + 4.0f * knobX, topChord2, knobX, colorBlue, ID::chordSemi3, Scales::oscSemi);

  const float topChord3 = topChord2 + labelY;
  addLabel(left0, topChord3, knobX, "Milli");
  addTextKnob(
    left0 + 1.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli0, Scales::oscMilli);
  addTextKnob(
    left0 + 2.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli1, Scales::oscMilli);
  addTextKnob(
    left0 + 3.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli2, Scales::oscMilli);
  addTextKnob(
    left0 + 4.0f * knobX, topChord3, knobX, colorBlue, ID::chordMilli3, Scales::oscMilli);

  const float topChord4 = topChord3 + labelY;
  addLabel(left0, topChord4 + (knobX - labelY) / 2, knobX, "Pan");
  addKnob(
    left0 + 1.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan0);
  addKnob(
    left0 + 2.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan1);
  addKnob(
    left0 + 3.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan2);
  addKnob(
    left0 + 4.0f * knobX, topChord4 - margin, knobX, colorBlue, nullptr, ID::chordPan3);

  // Overtone.
  const float topOvertone0 = topChord0;
  const float leftOvertone = left0 + 5.0f * knobX + 2.0f * margin;
  addGroupLabel(leftOvertone, topOvertone0, 4.0f * knobX, "Overtone");

  std::vector<Steinberg::Vst::ParamID> sBoxId(16);
  for (uint32_t i = 0; i < sBoxId.size(); ++i) sBoxId[i] = ID::overtone1 + i;

  std::vector<double> sBoxValue(sBoxId.size());
  for (size_t i = 0; i < sBoxValue.size(); ++i)
    sBoxValue[i] = controller->getParamNormalized(sBoxId[i]);

  std::vector<double> sBoxDefaultValue(sBoxId.size());
  for (size_t i = 0; i < sBoxValue.size(); ++i)
    sBoxDefaultValue[i] = param.value[sBoxId[i]]->getDefaultNormalized();

  auto barBoxLeft = leftOvertone;
  auto barBoxTop = topOvertone0 + labelY;
  auto barBoxHeight = 4.0f * knobX;
  auto barBoxWidth = 2.0f * knobY;
  auto barBoxRect
    = CRect(barBoxLeft, barBoxTop, barBoxLeft + barBoxHeight, barBoxTop + barBoxWidth);
  auto barBox = new BarBox(barBoxRect, this, sBoxId, sBoxValue, sBoxDefaultValue);
  barBox->setFont(new CFontDesc(Style::fontName(), 10.0, CTxtFace::kBoldFace));
  barBox->setBorderColor(colorBlack);
  barBox->setValueColor(colorBlue);
  frame->addView(barBox);
  if (overtoneControl != nullptr) overtoneControl->forget();
  overtoneControl = barBox;
  overtoneControl->remember();

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - 20.0f;
  const auto splashLeft = defaultWidth - 3.0f * knobX - 15.0f;
  addSplashScreen(
    splashLeft, splashTop, 3.0f * knobX, splashHeight, 20.0f, 20.0f,
    defaultWidth - splashHeight, defaultHeight - splashHeight, "IterativeSinCluster");

  return true;
}

void PlugEditor::close()
{
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }

  if (overtoneControl != nullptr) {
    overtoneControl->forget();
    overtoneControl = nullptr;
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

  if (overtoneControl != nullptr && id >= ID ::overtone1 && id <= ID::overtone16) {
    overtoneControl->setValueAt(id - ID::overtone1, normalized);
    overtoneControl->invalid();
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
    nullptr);
  auto splash = new SplashLabel(
    CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
    this, -666, credit, pluginName);
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
