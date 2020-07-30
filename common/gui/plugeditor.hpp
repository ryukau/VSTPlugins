// (c) 2020 Takamitsu Endo
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

#include "../parameterInterface.hpp"
#include "arraycontrol.hpp"
#include "style.hpp"

#include "barbox.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "knob.hpp"
#include "label.hpp"
#include "optionmenu.hpp"
#include "rotaryknob.hpp"
#include "scrollbar.hpp"
#include "slider.hpp"
#include "splash.hpp"
#include "tabview.hpp"
#include "textview.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);
  ~PlugEditor();

  bool PLUGIN_API
  open(void *parent, const PlatformType &platformType = kDefaultNative) override;
  void PLUGIN_API close() override;
  virtual void valueChanged(CControl *pControl) override;
  virtual void valueChanged(ParamID id, ParamValue normalized);
  virtual void updateUI(Vst::ParamID id, ParamValue normalized);

  virtual void onMouseEntered(CView *view, CFrame *frame) override {}
  virtual void onMouseExited(CView *view, CFrame *frame) override {}
  virtual CMouseEventResult
  onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;
  virtual CMouseEventResult
  onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;

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
    barBox->setIndexFont(new CFontDesc(Uhhyou::Font::name(), 10.0, CTxtFace::kBoldFace));
    barBox->setNameFont(new CFontDesc(Uhhyou::Font::name(), 24.0, CTxtFace::kNormalFace));
    barBox->setName(name);
    frame->addView(barBox);

    auto iter = arrayControlInstances.find(id0);
    if (iter != arrayControlInstances.end()) {
      iter->second->forget();
      arrayControlInstances.erase(iter);
    }
    barBox->remember();
    arrayControlInstances.emplace(std::make_pair(id0, barBox));
    for (ParamID i = 0; i < id.size(); ++i)
      arrayControlMap.emplace(std::make_pair(id0 + i, barBox));
    return barBox;
  }

  template<typename Parent>
  auto addScrollBar(float left, float top, float width, float height, Parent parent)
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
      CRect(left, top, left + width, top + height), this, tag, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette);
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
      CRect(left, top, left + width, top + height), this, tag, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette);
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
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette);
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
      CRect(left, top, left + width, top + height), this, tag, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette);
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
      CRect(left, top, left + width, top + height), this, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      align);
    frame->addView(label);
    return label;
  }

  GroupLabel *addGroupLabel(
    CCoord left, CCoord top, CCoord width, float height, float textSize, std::string name)
  {
    auto label = new GroupLabel(
      CRect(left, top, left + width, top + height), this, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace), palette);
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
    label->setFont(new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kBoldFace));
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
      CRect(left, top, left + width, top + height), this, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      align);
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
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      scale, offset);
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
      CRect(left, top, left + width, top + height), this, tag,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      scale, isDecibel);
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
    menu->setFont(new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace));
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
      tabs, new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      tabHeight, CRect(left, top, left + width, top + height));
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
      this, palette);
    auto splash = new SplashLabel(
      CRect(buttonLeft, buttonTop, buttonLeft + buttonWidth, buttonTop + buttonHeight),
      this, 0, credit, pluginName, buttonTextSize, palette);
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
      CRect(left, top, left + width, top + height), text,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette);
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
      CRect(left, top, left + width, top + height), text, cellWidth,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette);
    frame->addView(view);
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
      CRect(left, top, right, bottom), this, name,
      new CFontDesc(Uhhyou::Font::name(), textSize, CTxtFace::kNormalFace), palette,
      kCenterText);
    frame->addView(label);

    return std::make_tuple(slider, label);
  }

protected:
  void addToControlMap(Vst::ParamID id, CControl *control);
  virtual bool prepareUI() = 0;

  std::unique_ptr<ParameterInterface> param;

  std::unordered_map<Vst::ParamID, CControl *> controlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlMap;
  std::unordered_map<Vst::ParamID, ArrayControl *> arrayControlInstances; // Dirty hack.

  ViewRect viewRect{0, 0, 512, 512};

  Uhhyou::Palette palette;
};

} // namespace Vst
} // namespace Steinberg
