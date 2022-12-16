// (c) 2020-2021 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"
#include "vstgui/lib/platform/iplatformframecallback.h"

#include "../parameterInterface.hpp"
#include "arraycontrol.hpp"
#include "style.hpp"

#include "barbox.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "knob.hpp"
#include "label.hpp"
#include "matrixknob.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "scrollbar.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "tabview.hpp"
#include "textview.hpp"
#include "xypad.hpp"

#include "x11runloop.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller) : VSTGUIEditor(controller)
  {
    setRect(viewRect);
    loadFont();
  }

  virtual ~PlugEditor()
  {
    for (auto &ctrl : controlMap)
      if (ctrl.second) ctrl.second->forget();

    for (auto &ctrl : arrayControlInstances)
      if (ctrl.second) ctrl.second->forget();
  }

  bool PLUGIN_API open(
    void *parent,
    const PlatformType &platformType = PlatformType::kDefaultNative) override
  {
    if (frame) return false;

    setIdleRate(1000 / 60);

    frame = new CFrame(
      CRect(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom), this);
    if (frame == nullptr) return false;
    frame->setBackgroundColor(palette.background());
    frame->registerMouseObserver(this);

    IPlatformFrameConfig *config = nullptr;
#if LINUX
    X11::FrameConfig x11config;
    x11config.runLoop = VSTGUI::owned(new RunLoop(plugFrame));
    config = &x11config;
#endif
    frame->open(parent, platformType, config);

    return prepareUI();
  }

  void PLUGIN_API close() override
  {
    if (frame != nullptr) {
      frame->unregisterMouseObserver(this);
      frame->forget();
      frame = nullptr;
    }
  }

  virtual void valueChanged(CControl *pControl) override
  {
    ParamID tag = pControl->getTag();
    ParamValue value = pControl->getValueNormalized();
    controller->setParamNormalized(tag, value);
    controller->performEdit(tag, value);
  }

  virtual void valueChanged(ParamID id, ParamValue normalized)
  {
    controller->setParamNormalized(id, normalized);
    controller->performEdit(id, normalized);
  }

  virtual void updateUI(Vst::ParamID id, ParamValue normalized)
  {
    auto vCtrl = controlMap.find(id);
    if (vCtrl != controlMap.end()) {
      vCtrl->second->setValueNormalized(normalized);
      vCtrl->second->invalid();
      return;
    }

    auto aCtrl = arrayControlMap.find(id);
    if (aCtrl != arrayControlMap.end()) {
      aCtrl->second->setValueAt(id - aCtrl->second->id.front(), normalized);
      aCtrl->second->invalid();
      return;
    }
  }

  virtual void onMouseEntered(CView *view, CFrame *frame) override {}
  virtual void onMouseExited(CView *view, CFrame *frame) override {}

  virtual void onMouseEvent(MouseEvent &event, CFrame *frame) override
  {
    if (!event.buttonState.isRight()) return;

    auto componentHandler = controller->getComponentHandler();
    if (componentHandler == nullptr) return;

    FUnknownPtr<IComponentHandler3> handler(componentHandler);
    if (handler == nullptr) return;

    auto control = dynamic_cast<CControl *>(frame->getViewAt(event.mousePosition));
    if (control == nullptr) return;

    // Context menu will not popup when the control has negative tag.
    ParamID id = control->getTag();
    if (id < 1 || id >= LONG_MAX) return;

    IContextMenu *menu = handler->createContextMenu(this, &id);
    if (menu == nullptr) return;

    menu->popup(event.mousePosition.x, event.mousePosition.y);
    menu->release();

    event.consumed = true;
  };

  DELEGATE_REFCOUNT(VSTGUIEditor);

  template<typename Scale>
  BarBox<Scale> *addBarBox(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    ParamID id0,
    size_t nElement,
    Scale &scale,
    std::string name)
  {
    std::vector<ParamID> id(nElement);
    for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + ParamID(i);
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      value[i] = controller->getParamNormalized(id[i]);
    std::vector<double> defaultValue(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      defaultValue[i] = param->getDefaultNormalized(id[i]);

    auto barBox = new BarBox<Scale>(
      this, CRect(left, top, left + width, top + height), id, scale, value, defaultValue,
      palette);
    barBox->setIndexFont(getFont(10));
    barBox->setNameFont(getFont(24));
    barBox->setName(name);
    frame->addView(barBox);

    addToArrayControlInstances(id0, barBox);

    for (ParamID i = 0; i < id.size(); ++i)
      arrayControlMap.emplace(std::make_pair(id0 + i, barBox));
    return barBox;
  }

  auto
  addXYPad(CCoord left, CCoord top, CCoord width, CCoord height, ParamID id0, ParamID id1)
  {
    std::vector<ParamID> id{id0, id1};
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      value[i] = controller->getParamNormalized(id[i]);
    std::vector<double> defaultValue(value);
    for (size_t i = 0; i < defaultValue.size(); ++i)
      defaultValue[i] = param->getDefaultNormalized(id[i]);

    auto xyPad = new XYPad(
      this, CRect(left, top, left + width, top + height), id, value, defaultValue,
      palette);
    frame->addView(xyPad);

    addToArrayControlInstances(id0, xyPad);

    arrayControlMap.emplace(std::make_pair(id0, xyPad));
    arrayControlMap.emplace(std::make_pair(id1, xyPad));
    return xyPad;
  }

  auto addMatrixKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    uint32_t nRow,
    uint32_t nColumn,
    std::vector<ParamID> id,
    TextView *textView = nullptr)
  {
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      value[i] = controller->getParamNormalized(id[i]);
    std::vector<double> defaultValue(value);
    for (size_t i = 0; i < defaultValue.size(); ++i)
      defaultValue[i] = param->getDefaultNormalized(id[i]);

    auto matrix = new MatrixKnob(
      this, CRect(left, top, left + width, top + height), id, value, defaultValue, nRow,
      nColumn, textView, palette);
    frame->addView(matrix);

    addToArrayControlInstances(id[0], matrix);

    for (const auto &ident : id) arrayControlMap.emplace(std::make_pair(ident, matrix));
    return matrix;
  }

  template<typename Parent>
  auto addScrollBar(CCoord left, CCoord top, CCoord width, CCoord height, Parent parent)
  {
    auto scrollBar = new ScrollBar<Parent>(
      CRect(left, top, left + width, top + height), this, parent, palette);
    frame->addView(scrollBar);
    return scrollBar;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addKickButton(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto button = new KickButton<style>(
      CRect(left, top, left + width, top + height), this, tag, name, getFont(textSize),
      palette);
    button->setValueNormalized(controller->getParamNormalized(tag));
    frame->addView(button);
    addToControlMap(tag, button);
    return button;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addToggleButton(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto button = new ToggleButton<style>(
      CRect(left, top, left + width, top + height), this, tag, name, getFont(textSize),
      palette);
    button->setValueNormalized(controller->getParamNormalized(tag));
    frame->addView(button);
    addToControlMap(tag, button);
    return button;
  }

  MessageButton *addStateButton(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    std::string message,
    std::string attribute)
  {
    auto button = new MessageButton(
      controller, CRect(left, top, left + width, top + height), name, message,
      getFont(textSize), palette);
    frame->addView(button);
    return button;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addCheckbox(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto checkbox = new CheckBox<style>(
      CRect(left, top, left + width, top + height), this, tag, name, getFont(textSize),
      palette);
    checkbox->setTextSize(textSize);
    checkbox->setValueNormalized(controller->getParamNormalized(tag));
    frame->addView(checkbox);
    addToControlMap(tag, checkbox);
    return checkbox;
  }

  auto addLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    CHoriTxtAlign align = CHoriTxtAlign::kCenterText)
  {
    auto label = new Label(
      CRect(left, top, left + width, top + height), this, name, getFont(textSize),
      palette, align);
    frame->addView(label);
    return label;
  }

  GroupLabel *addGroupLabel(
    CCoord left, CCoord top, CCoord width, float height, float textSize, std::string name)
  {
    auto label = new GroupLabel(
      CRect(left, top, left + width, top + height), this, name, getFont(textSize),
      palette);
    frame->addView(label);
    return label;
  }

  VGroupLabel *addGroupVerticalLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name)
  {
    return nullptr;

    // VSTGUI 4.9 can't draw roteted text.
    /*
    auto label = new VGroupLabel(
      CRect(left, top, left + height, top + width), this, name);
    label->setFont(getFont(textSize));
    frame->addView(label);
    return label;
    */
  };

  enum class LabelPosition {
    top,
    left,
    bottom,
    right,
  };

  auto addKnobLabel(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord margin,
    CCoord textSize,
    std::string name,
    LabelPosition labelPosition)
  {
    CHoriTxtAlign align;

    switch (labelPosition) {
      default:
      case LabelPosition::bottom:
        top = top + height - textSize * 0.25;
        height = textSize * 1.5;
        left -= 2 * margin;
        width += 4 * margin;
        align = kCenterText;
        break;

      case LabelPosition::right:
        height = width;
        left = left + width + margin;
        width *= 2.0f;
        align = kLeftText;
        break;
    }

    auto label = new Label(
      CRect(left, top, left + width, top + height), this, name, getFont(textSize),
      palette, align);
    frame->addView(label);
    return label;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag,
    LabelPosition labelPosition = LabelPosition::bottom,
    CCoord labelMargin = 10)
  {
    auto height = width;

    auto knob = new Knob<style>(
      CRect(left, top + margin, left + width, top + width - margin), this, tag, palette);
    knob->setSlitWidth(8.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addKnobLabel(
      left, top, width, height, labelMargin, textSize, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  template<Uhhyou::Style style = Uhhyou::Style::common, typename Scale>
  auto addNumberKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag,
    Scale &scale,
    int32_t offset = 0,
    LabelPosition labelPosition = LabelPosition::bottom,
    CCoord labelMargin = 10)
  {
    auto height = width;

    auto knob = new NumberKnob<Scale, style>(
      CRect(left, top + margin, left + width, top + width - margin), this, tag,
      getFont(textSize), palette, scale, offset);
    knob->setSlitWidth(8.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addKnobLabel(
      left, top, width, height, labelMargin, textSize, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addRotaryKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag,
    LabelPosition labelPosition = LabelPosition::bottom,
    CCoord labelMargin = 10)
  {
    auto height = width;

    auto knob = new RotaryKnob<style>(
      CRect(left + margin, top + margin, left + width - margin, top + width - margin),
      this, tag, palette);
    knob->setSlitWidth(8.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addKnobLabel(
      left, top, width, height, labelMargin, textSize, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  template<Uhhyou::Style style = Uhhyou::Style::common, typename Scale>
  auto addTextKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    ParamID tag,
    Scale &scale,
    bool isDecibel = false,
    uint32_t precision = 0,
    int32_t offset = 0)
  {
    auto knob = new TextKnob<Scale, style>(
      CRect(left, top, left + width, top + height), this, tag, getFont(textSize), palette,
      scale, isDecibel);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    knob->setPrecision(precision);
    knob->offset = offset;
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common, typename Scale>
  auto addRotaryTextKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    ParamID tag,
    Scale &scale,
    uint32_t precision = 0,
    int32_t offset = 0)
  {
    auto knob = new RotaryTextKnob<Scale, style>(
      CRect(left, top, left + width, top + height), this, tag, getFont(textSize), palette,
      scale);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    knob->setPrecision(precision);
    knob->offset = offset;
    frame->addView(knob);
    addToControlMap(tag, knob);
    return knob;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  OptionMenu *addOptionMenu(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    ParamID tag,
    const std::vector<std::string> &items)
  {
    auto menu = new OptionMenu(
      CRect(left, top, left + width, top + height), this, tag, nullptr, nullptr,
      COptionMenu::kCheckStyle);

    for (const auto &item : items) menu->addEntry(item.c_str());
    menu->setFont(getFont(textSize));
    menu->setFrameWidth(1.0);
    menu->setFontColor(palette.foreground());
    menu->setBackColor(palette.boxBackground());
    menu->setDefaultFrameColor(palette.border());
    if constexpr (style == Uhhyou::Style::accent) {
      menu->setHighlightColor(palette.highlightAccent());
    } else if (style == Uhhyou::Style::warning) {
      menu->setHighlightColor(palette.highlightWarning());
    } else {
      menu->setHighlightColor(palette.highlightMain());
    }
    // menu->sizeToFit();

    menu->setValueNormalized(controller->getParamNormalized(tag));
    frame->addView(menu);
    addToControlMap(tag, menu);
    return menu;
  }

  TabView *addTabView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    CCoord tabHeight,
    std::vector<std::string> tabs)
  {
    auto tabview = new TabView(
      tabs, getFont(textSize), palette, tabHeight,
      CRect(left, top, left + width, top + height));
    frame->addView(tabview);
    return tabview;
  }

  void addSplashScreen(
    CCoord buttonLeft,
    CCoord buttonTop,
    CCoord buttonWidth,
    CCoord buttonHeight,
    CCoord splashLeft,
    CCoord splashTop,
    CCoord splashWidth,
    CCoord splashHeight,
    CCoord buttonTextSize,
    std::string pluginName)
  {
    auto credit = new CreditView(
      CRect(splashLeft, splashTop, splashLeft + splashWidth, splashTop + splashHeight),
      this, getFont(18.0), getFont(12.0), palette);
    auto splash = new SplashLabel(
      CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
      this, 0, credit, pluginName, getFont(buttonTextSize), palette);
    frame->addView(splash);
    frame->addView(credit);
  }

  TextView *addTextView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string text)
  {
    auto view = new TextView(
      CRect(left, top, left + width, top + height), text, getFont(textSize), palette);
    frame->addView(view);
    return view;
  }

  TextTableView *addTextTableView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string text,
    CCoord cellWidth)
  {
    auto view = new TextTableView(
      CRect(left, top, left + width, top + height), text, cellWidth, getFont(textSize),
      palette);
    frame->addView(view);
    return view;
  }

  template<typename Scale>
  auto addValueTextView(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord textSize,
    std::string name,
    ParamID tag,
    Scale &scale)
  {
    auto view = new ValueTextView<Scale>(
      CRect(left, top, left + width, top + height), name, getFont(textSize), palette,
      scale);
    view->setValueNormalized(controller->getParamNormalized(tag));
    view->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(view);
    addToControlMap(tag, view);
    return view;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addVSlider(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord margin,
    CCoord labelHeight,
    CCoord textSize,
    std::string name,
    ParamID tag,
    bool drawFromCenter = false)
  {
    auto right = left + width;
    auto bottom = top + height;

    auto slider = new Slider(
      CRect(left, top, right, bottom), this, tag, top, bottom, nullptr, nullptr);

    slider->setSliderMode(CSliderMode::FreeClick);
    slider->setStyle(CSlider::kBottom | CSlider::kVertical);
    slider->setDrawStyle(
      CSlider::kDrawBack | CSlider::kDrawFrame | CSlider::kDrawValue
      | (drawFromCenter ? CSlider::kDrawValueFromCenter | CSlider::kDrawInverted : 0));
    slider->setBackColor(palette.boxBackground());
    slider->setDefaultFrameColor(palette.border());
    if constexpr (style == Uhhyou::Style::accent) {
      slider->setValueColor(palette.highlightAccent());
      slider->setHighlightColor(palette.highlightAccent());
    } else if (style == Uhhyou::Style::warning) {
      slider->setValueColor(palette.highlightWarning());
      slider->setHighlightColor(palette.highlightWarning());
    } else {
      slider->setValueColor(palette.highlightMain());
      slider->setHighlightColor(palette.highlightMain());
    }
    slider->setHighlightWidth(3.0);

    slider->setValueNormalized(controller->getParamNormalized(tag));
    slider->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(slider);
    addToControlMap(tag, slider);

    top = bottom + margin;
    bottom = top + labelHeight;

    auto label = new Label(
      CRect(left, top, right, bottom), this, name, getFont(textSize), palette,
      kCenterText);
    frame->addView(label);

    return std::make_tuple(slider, label);
  }

protected:
  void addToArrayControlInstances(Vst::ParamID id0, ArrayControl *control)
  {
    auto iter = arrayControlInstances.find(id0);
    if (iter != arrayControlInstances.end()) {
      iter->second->forget();
      arrayControlInstances.erase(iter);
    }
    control->remember();
    arrayControlInstances.emplace(std::make_pair(id0, control));
  }

  void addToControlMap(Vst::ParamID id, CControl *control)
  {
    auto iter = controlMap.find(id);
    if (iter != controlMap.end()) iter->second->forget();
    control->remember();
    controlMap.insert({id, control});
  }

  SharedPointer<CFontDesc> getFont(double size)
  {
    auto keySize = size_t(10.0 * size);
    auto found = fontMap.find(keySize);
    if (found != fontMap.end()) return found->second;
    auto inserted = fontMap.emplace(
      keySize,
      new CFontDesc(palette.fontName(), CCoord(keySize) / 10.0, palette.fontFace()));
    return inserted.first->second;
  }

  void loadFont()
  {
    std::vector<size_t> sizes{100, 120, 140, 160, 180, 200, 220, 240};
    for (const auto &sz : sizes) {
      fontMap.emplace(
        sz, new CFontDesc(palette.fontName(), CCoord(sz) / 10.0, palette.fontFace()));
    }
  }

  virtual bool prepareUI() = 0;

  std::unique_ptr<ParameterInterface> param;

  std::unordered_map<Vst::ParamID, CControl *> controlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlInstances;

  ViewRect viewRect{0, 0, 512, 512};

  Uhhyou::Palette palette;

  std::unordered_map<size_t, SharedPointer<CFontDesc>> fontMap; // key = 10 * fontSize.
};

} // namespace Vst
} // namespace Steinberg
