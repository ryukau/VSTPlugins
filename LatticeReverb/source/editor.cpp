// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

constexpr float uiMargin = 20.0f;
constexpr float uiTextSize = 12.0f;
constexpr float midTextSize = 12.0f;
constexpr float pluginNameTextSize = 18.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float textKnobX = 80.0f;
constexpr float splashHeight = 40.0f;
constexpr float barboxWidth = 4 * textKnobX;
constexpr float barboxHeight = 2 * knobY;

constexpr float tabViewWidth = barboxWidth + labelY + 2 * uiMargin;
constexpr float tabViewHeight
  = labelY + 3 * barboxHeight + 2 * labelHeight + 2 * uiMargin;

constexpr float leftPanelWidth = 4 * knobX + 6 * margin + labelHeight;

constexpr uint32_t defaultWidth
  = uint32_t(leftPanelWidth + labelY + tabViewWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(tabViewHeight + 2 * uiMargin);

enum tabIndex { tabBase, tabOffset, tabModulation };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  viewRect = ViewRect{0, 0, int32(defaultWidth), int32(defaultHeight)};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;

  // Multipliers.
  const auto mulTop0 = top0;
  const auto mulTop1 = mulTop0 + labelY;
  const auto mulTop2 = mulTop1 + labelY;
  const auto mulTop3 = mulTop2 + labelY;
  const auto mulTop4 = mulTop3 + labelY;
  const auto mulLeft0 = left0;
  const auto mulLeft1 = mulLeft0 + textKnobX + 2 * margin;
  const auto mulLeft2 = mulLeft1 + textKnobX + 2 * margin;

  addGroupLabel(
    mulLeft0, mulTop0, leftPanelWidth, labelHeight, midTextSize, "Multiplier");

  addLabel(mulLeft1, mulTop1, textKnobX, labelHeight, uiTextSize, "Base");
  addLabel(mulLeft2, mulTop1, textKnobX, labelHeight, uiTextSize, "Offset");

  addLabel(mulLeft0, mulTop2, textKnobX, labelHeight, uiTextSize, "Time", kLeftText);
  addLabel(mulLeft0, mulTop3, textKnobX, labelHeight, uiTextSize, "OuterFeed", kLeftText);
  addLabel(mulLeft0, mulTop4, textKnobX, labelHeight, uiTextSize, "InnerFeed", kLeftText);

  addTextKnob(
    mulLeft1, mulTop2, textKnobX, labelHeight, uiTextSize, ID::timeMultiply,
    Scales::multiply, false, 4);
  addTextKnob(
    mulLeft1, mulTop3, textKnobX, labelHeight, uiTextSize, ID::outerFeedMultiply,
    Scales::multiply, false, 4);
  addTextKnob(
    mulLeft1, mulTop4, textKnobX, labelHeight, uiTextSize, ID::innerFeedMultiply,
    Scales::multiply, false, 4);
  addTextKnob(
    mulLeft2, mulTop2, textKnobX, labelHeight, uiTextSize, ID::timeOffsetMultiply,
    Scales::multiply, false, 4);
  addTextKnob(
    mulLeft2, mulTop3, textKnobX, labelHeight, uiTextSize, ID::outerFeedOffsetMultiply,
    Scales::multiply, false, 4);
  addTextKnob(
    mulLeft2, mulTop4, textKnobX, labelHeight, uiTextSize, ID::innerFeedOffsetMultiply,
    Scales::multiply, false, 4);

  // Panic button.
  const auto panicButtonLeft = left0 + 1.5f * knobX;
  const auto panicButtonTop = mulTop4 + 2 * labelY;
  auto panicButton = new PanicButton(
    CRect(
      panicButtonLeft, panicButtonTop, panicButtonLeft + leftPanelWidth - 3.0f * knobX,
      panicButtonTop + 2 * labelHeight),
    this, 0, "Panic!", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  // Mix.
  const auto miscTop0 = mulTop0 + 8 * labelY + 3 * margin;
  const auto miscTop1 = miscTop0 + labelY;

  const auto mixLeft0 = left0;
  const auto mixLeft1 = mixLeft0 + knobX + 2 * margin;

  addGroupLabel(
    mixLeft0, miscTop0, 2 * knobX + 2 * margin, labelHeight, midTextSize, "Mix");
  addKnob(mixLeft0, miscTop1, knobX, margin, uiTextSize, "Dry", ID::dry);
  addKnob(mixLeft1, miscTop1, knobX, margin, uiTextSize, "Wet", ID::wet);

  // Misc.
  const auto miscLeft0 = left0 + 2 * knobX + 2 * margin + labelHeight;
  const auto miscLeft1 = miscLeft0 + knobX + 2 * margin;

  addGroupLabel(
    miscLeft0, miscTop0, 2 * knobX + 2 * margin, labelHeight, midTextSize, "Stereo");
  addKnob(miscLeft0, miscTop1, knobX, margin, uiTextSize, "Cross", ID::stereoCross);
  addKnob(miscLeft1, miscTop1, knobX, margin, uiTextSize, "Spread", ID::stereoSpread);

  // Smooth.
  const auto leftSmooth = left0 + 1.5f * knobX + 3 * margin + 0.5f * labelHeight;
  addKnob(
    leftSmooth, miscTop1 + knobY + labelY, knobX, margin, uiTextSize, "Smooth",
    ID::smoothness);

  // Right side.
  const auto tabViewLeft = left0 + leftPanelWidth + labelY;

  std::vector<std::string> tabs{"Base", "Offset", "Modulation"};
  auto tabview = addTabView(
    tabViewLeft, top0, tabViewWidth, tabViewHeight, uiTextSize, labelY, tabs);

  const auto tabInsideTop0 = top0 + labelY + uiMargin;
  const auto tabInsideTop1 = tabInsideTop0 + barboxHeight + labelHeight;
  const auto tabInsideTop2 = tabInsideTop1 + barboxHeight + labelHeight;
  const auto tabInsideLeft0 = tabViewLeft + uiMargin;
  const auto tabInsideLeft1 = tabInsideLeft0 + labelY;

  // Base tab.
  tabview->addWidget(
    tabBase,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop0, barboxHeight, labelHeight, midTextSize, "Time"));
  tabview->addWidget(
    tabBase,
    addBarBox(
      tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::time0, nestingDepth,
      Scales::time, "Time"));

  tabview->addWidget(
    tabBase,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop1, barboxHeight, labelHeight, midTextSize,
      "OuterFeed"));
  auto barboxOuterFeed = addBarBox(
    tabInsideLeft1, tabInsideTop1, barboxWidth, barboxHeight, ID::outerFeed0,
    nestingDepth, Scales::feed, "OuterFeed");
  barboxOuterFeed->sliderZero = 0.5f;
  tabview->addWidget(tabBase, barboxOuterFeed);

  tabview->addWidget(
    tabBase,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop2, barboxHeight, labelHeight, midTextSize,
      "InnerFeed"));
  auto barboxInnerFeed = addBarBox(
    tabInsideLeft1, tabInsideTop2, barboxWidth, barboxHeight, ID::innerFeed0,
    nestingDepth, Scales::feed, "InnerFeed");
  barboxInnerFeed->sliderZero = 0.5f;
  tabview->addWidget(tabBase, barboxInnerFeed);

  // Tab offset.
  tabview->addWidget(
    tabOffset,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop0, barboxHeight, labelHeight, midTextSize, "Time"));
  auto barboxTimeOffset = addBarBox(
    tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::timeOffset0,
    nestingDepth, Scales::timeOffset, "Time");
  barboxTimeOffset->sliderZero = 0.5f;
  tabview->addWidget(tabOffset, barboxTimeOffset);

  tabview->addWidget(
    tabOffset,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop1, barboxHeight, labelHeight, midTextSize,
      "OuterFeed"));
  auto barboxOuterOffset = addBarBox(
    tabInsideLeft1, tabInsideTop1, barboxWidth, barboxHeight, ID::outerFeedOffset0,
    nestingDepth, Scales::feedOffset, "OuterFeed");
  barboxOuterOffset->sliderZero = 0.5f;
  tabview->addWidget(tabOffset, barboxOuterOffset);

  tabview->addWidget(
    tabOffset,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop2, barboxHeight, labelHeight, midTextSize,
      "InnerFeed"));
  auto barboxInnerOffset = addBarBox(
    tabInsideLeft1, tabInsideTop2, barboxWidth, barboxHeight, ID::innerFeedOffset0,
    nestingDepth, Scales::feedOffset, "InnerFeed");
  barboxInnerOffset->sliderZero = 0.5f;
  tabview->addWidget(tabOffset, barboxInnerOffset);

  // Tab modulation
  tabview->addWidget(
    tabModulation,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop0, barboxHeight, labelHeight, midTextSize, "Time LFO"));
  tabview->addWidget(
    tabModulation,
    addBarBox(
      tabInsideLeft1, tabInsideTop0, barboxWidth, barboxHeight, ID::timeLfoAmount0,
      nestingDepth, Scales::time, "Time LFO"));

  const auto tabViewCenter1 = tabInsideTop1 + (barboxHeight - labelHeight) / 2;
  tabview->addWidget(
    tabModulation,
    addLabel(
      tabInsideLeft0, tabViewCenter1, barboxHeight, labelHeight, uiTextSize,
      "Time LFO Cutoff"));
  tabview->addWidget(
    tabModulation,
    addTextKnob(
      tabInsideLeft0 + 2 * textKnobX, tabViewCenter1, textKnobX, labelHeight, uiTextSize,
      ID::timeLfoLowpass, Scales::timeLfoLowpas, false, 5));

  tabview->addWidget(
    tabModulation,
    addGroupVerticalLabel(
      tabInsideLeft0, tabInsideTop2, barboxHeight, labelHeight, midTextSize,
      "Lowpass Cutoff"));
  tabview->addWidget(
    tabModulation,
    addBarBox(
      tabInsideLeft1, tabInsideTop2, barboxWidth, barboxHeight, ID::lowpassCutoff0,
      nestingDepth, Scales::defaultScale, "Lowpass Cutoff"));

  tabview->refreshTab();

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashLeft = left0 + knobX;
  addSplashScreen(
    splashLeft, splashTop, leftPanelWidth - 2 * knobX, splashHeight, uiMargin, uiMargin,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "LatticeReverb");

  return true;
}

} // namespace Vst
} // namespace Steinberg
