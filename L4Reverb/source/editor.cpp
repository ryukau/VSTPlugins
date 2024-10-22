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
constexpr float scrollBarHeight = 10.0f;
constexpr float barboxWidth = 2 * nDepth1;
constexpr float barboxHeight = 2 * knobY + labelY + scrollBarHeight;
constexpr float barboxWidthSmall = 4 * nDepth2;

constexpr float barboxSectionWidth = barboxWidth + barboxWidthSmall + 3 * labelY;
constexpr float barboxSectionHeight
  = 3 * scrollBarHeight + 3 * barboxHeight + 2 * labelHeight;

constexpr float leftPanelWidth = 4 * textKnobX + 6 * margin;

constexpr uint32_t defaultWidth
  = uint32_t(leftPanelWidth + labelY + barboxSectionWidth + 2 * uiMargin);
constexpr uint32_t defaultHeight = uint32_t(barboxSectionHeight + 2 * uiMargin);

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
  const auto mulTop5 = mulTop4 + labelY;
  const auto mulTop6 = mulTop5 + labelY;
  const auto mulTop7 = mulTop6 + labelY;
  const auto mulTop8 = mulTop7 + 2 * labelY;
  const auto mulLeft0 = left0;
  const auto mulLeft1 = mulLeft0 + textKnobX + 2 * margin;
  const auto mulLeft2 = mulLeft1 + textKnobX + 2 * margin;
  const auto mulLeft3 = mulLeft2 + textKnobX + 2 * margin;

  addGroupLabel(
    mulLeft0, mulTop0, leftPanelWidth, labelHeight, midTextSize, "Multiplier");

  addLabel(mulLeft1, mulTop1, textKnobX, labelHeight, uiTextSize, "Base");
  addLabel(mulLeft2, mulTop1, textKnobX, labelHeight, uiTextSize, "Offset");
  addLabel(mulLeft3, mulTop1, textKnobX, labelHeight, uiTextSize, "Modulation");

  addLabel(mulLeft0, mulTop2, textKnobX, labelHeight, uiTextSize, "Time", kLeftText);
  addLabel(mulLeft0, mulTop3, textKnobX, labelHeight, uiTextSize, "InnerFeed", kLeftText);
  addLabel(mulLeft0, mulTop4, textKnobX, labelHeight, uiTextSize, "D1 Feed", kLeftText);
  addLabel(mulLeft0, mulTop5, textKnobX, labelHeight, uiTextSize, "D2 Feed", kLeftText);
  addLabel(mulLeft0, mulTop6, textKnobX, labelHeight, uiTextSize, "D3 Feed", kLeftText);
  addLabel(mulLeft0, mulTop7, textKnobX, labelHeight, uiTextSize, "D4 Feed", kLeftText);

  addTextKnob(
    mulLeft1, mulTop2, textKnobX, labelHeight, uiTextSize, ID::timeMultiply,
    Scales::timeMultiply, false, 8);
  addTextKnob(
    mulLeft1, mulTop3, textKnobX, labelHeight, uiTextSize, ID::innerFeedMultiply,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft1, mulTop4, textKnobX, labelHeight, uiTextSize, ID::d1FeedMultiply,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft1, mulTop5, textKnobX, labelHeight, uiTextSize, ID::d2FeedMultiply,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft1, mulTop6, textKnobX, labelHeight, uiTextSize, ID::d3FeedMultiply,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft1, mulTop7, textKnobX, labelHeight, uiTextSize, ID::d4FeedMultiply,
    Scales::defaultScale, false, 4);

  addTextKnob(
    mulLeft2, mulTop2, textKnobX, labelHeight, uiTextSize, ID::timeOffsetRange,
    Scales::timeOffsetRange, false, 8);
  addTextKnob(
    mulLeft2, mulTop3, textKnobX, labelHeight, uiTextSize, ID::innerFeedOffsetRange,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft2, mulTop4, textKnobX, labelHeight, uiTextSize, ID::d1FeedOffsetRange,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft2, mulTop5, textKnobX, labelHeight, uiTextSize, ID::d2FeedOffsetRange,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft2, mulTop6, textKnobX, labelHeight, uiTextSize, ID::d3FeedOffsetRange,
    Scales::defaultScale, false, 4);
  addTextKnob(
    mulLeft2, mulTop7, textKnobX, labelHeight, uiTextSize, ID::d4FeedOffsetRange,
    Scales::defaultScale, false, 4);

  const auto mulLeft3Mid = mulLeft3 + floorf(textKnobX / 2) - 5;
  addCheckbox(
    mulLeft3Mid, mulTop2, textKnobX, labelHeight, uiTextSize, "", ID::timeModulation);
  addCheckbox(
    mulLeft3Mid, mulTop3, textKnobX, labelHeight, uiTextSize, "",
    ID::innerFeedModulation);
  addCheckbox(
    mulLeft3Mid, mulTop4, textKnobX, labelHeight, uiTextSize, "", ID::d1FeedModulation);
  addCheckbox(
    mulLeft3Mid, mulTop5, textKnobX, labelHeight, uiTextSize, "", ID::d2FeedModulation);
  addCheckbox(
    mulLeft3Mid, mulTop6, textKnobX, labelHeight, uiTextSize, "", ID::d3FeedModulation);
  addCheckbox(
    mulLeft3Mid, mulTop7, textKnobX, labelHeight, uiTextSize, "", ID::d4FeedModulation);

  // Panic button.

  const auto panicButtonLeft = mulLeft1;
  const auto panicButtonTop = mulTop8 - labelHeight / 2;
  auto panicButton = new PanicButton(
    CRect(
      panicButtonLeft, panicButtonTop, panicButtonLeft + 2 * textKnobX + 2 * margin,
      panicButtonTop + 2 * labelHeight),
    this, 0, "Panic!", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  // Mix.
  const auto offsetKnobX = floorf((textKnobX - knobX) / 2);

  const auto mixTop0 = mulTop0 + 11 * labelY;
  const auto mixTop1 = mixTop0 + labelY;

  const auto mixLeft0 = left0 + 2 * offsetKnobX;
  const auto mixLeft1 = mixLeft0 + knobX + 2 * margin;

  addGroupLabel(
    mixLeft0, mixTop0, 2 * knobX + 2 * margin, labelHeight, midTextSize, "Mix");
  addKnob(mixLeft0, mixTop1, knobX, margin, uiTextSize, "Dry", ID::dry);
  addKnob(mixLeft1, mixTop1, knobX, margin, uiTextSize, "Wet", ID::wet);

  // Stereo.
  const auto stereoLeft0 = left0 + 2 * textKnobX + 4 * margin + 2 * offsetKnobX;
  const auto stereoLeft1 = stereoLeft0 + knobX + 2 * margin;

  addGroupLabel(
    stereoLeft0, mixTop0, 2 * knobX + 2 * margin, labelHeight, midTextSize, "Stereo");
  addKnob<Style::warning>(
    stereoLeft0, mixTop1, knobX, margin, uiTextSize, "Cross", ID::stereoCross);
  addKnob<Style::warning>(
    stereoLeft1, mixTop1, knobX, margin, uiTextSize, "Spread", ID::stereoSpread);

  // Misc.
  const auto miscTop0 = mixTop0 + labelY + knobY;
  const auto miscTop1 = miscTop0 + labelY;

  const auto miscLeft0 = mulLeft1;
  const auto miscLeft1 = miscLeft0 + textKnobX + 2 * margin;

  addLabel(miscLeft0, miscTop1, textKnobX, labelHeight, uiTextSize, "Seed", kCenterText);
  auto textKnobSeed = addTextKnob(
    miscLeft0, miscTop1 + labelY, textKnobX, labelHeight, uiTextSize, ID::seed,
    Scales::seed);
  textKnobSeed->sensitivity = 0.001f;
  textKnobSeed->lowSensitivity = 1.0f / Scales::seed.getMax();

  addKnob(
    miscLeft1 + offsetKnobX, miscTop1, knobX, margin, uiTextSize, "Smooth",
    ID::smoothness);

  // Right side.
  const auto tabViewLeft = left0 + leftPanelWidth + labelY;

  const auto tabInsideTop0 = top0;
  const auto tabInsideTop1 = tabInsideTop0 + barboxHeight + scrollBarHeight + labelHeight;
  const auto tabInsideTop2 = tabInsideTop1 + barboxHeight + scrollBarHeight + labelHeight;
  const auto tabInsideLeft0 = tabViewLeft;
  const auto tabInsideLeft1 = tabInsideLeft0 + labelY;

  // Base tab.
  addGroupVerticalLabel(
    tabInsideLeft0, tabInsideTop0, barboxHeight, labelHeight, midTextSize, "Time");
  auto barboxTime = addBarBox(
    tabInsideLeft1, tabInsideTop0, 2 * nDepth1, barboxHeight, ID::time0, nDepth1,
    Scales::time, "Time");
  addScrollBar(
    tabInsideLeft1, tabInsideTop0 + barboxHeight - 1, 2 * nDepth1, scrollBarHeight,
    barboxTime);

  addGroupVerticalLabel(
    tabInsideLeft0, tabInsideTop1, barboxHeight, labelHeight, midTextSize, "InnerFeed");
  auto barboxInnerFeed = addBarBox(
    tabInsideLeft1, tabInsideTop1, 2 * nDepth1, barboxHeight, ID::innerFeed0, nDepth1,
    Scales::feed, "InnerFeed");
  barboxInnerFeed->sliderZero = 0.5f;
  addScrollBar(
    tabInsideLeft1, tabInsideTop1 + barboxHeight - 1, 2 * nDepth1, scrollBarHeight,
    barboxInnerFeed);

  addGroupVerticalLabel(
    tabInsideLeft0, tabInsideTop2, barboxHeight, labelHeight, midTextSize, "D1 Feed");
  auto barboxD1Feed = addBarBox(
    tabInsideLeft1, tabInsideTop2, 2 * nDepth1, barboxHeight, ID::d1Feed0, nDepth1,
    Scales::feed, "D1 Feed");
  barboxD1Feed->sliderZero = 0.5f;
  addScrollBar(
    tabInsideLeft1, tabInsideTop2 + barboxHeight - 1, 2 * nDepth1, scrollBarHeight,
    barboxD1Feed);

  const auto tabInsideLeft2 = tabInsideLeft1 + barboxWidth + labelY;
  const auto tabInsideLeft3 = tabInsideLeft2 + labelY;

  addGroupVerticalLabel(
    tabInsideLeft2, tabInsideTop0, barboxHeight, labelHeight, midTextSize, "D2 Feed");
  auto barboxD2Feed = addBarBox(
    tabInsideLeft3, tabInsideTop0, barboxWidthSmall, barboxHeight, ID::d2Feed0, nDepth2,
    Scales::feed, "D2 Feed");
  barboxD2Feed->sliderZero = 0.5f;

  addGroupVerticalLabel(
    tabInsideLeft2, tabInsideTop1, barboxHeight, labelHeight, midTextSize, "D3 Feed");
  auto barboxD3Feed = addBarBox(
    tabInsideLeft3, tabInsideTop1, barboxWidthSmall, barboxHeight, ID::d3Feed0, nDepth3,
    Scales::feed, "D3 Feed");
  barboxD3Feed->sliderZero = 0.5f;

  addGroupVerticalLabel(
    tabInsideLeft2, tabInsideTop2, barboxHeight, labelHeight, midTextSize, "D4 Feed");
  auto barboxD4Feed = addBarBox(
    tabInsideLeft3, tabInsideTop2, barboxWidthSmall, barboxHeight, ID::d4Feed0, nDepth4,
    Scales::feed, "D4 Feed");
  barboxD4Feed->sliderZero = 0.5f;

  // Plugin name.
  const auto splashTop = defaultHeight - splashHeight - uiMargin;
  const auto splashLeft = mulLeft1;
  addSplashScreen(
    splashLeft, splashTop, 2 * textKnobX + 2 * margin, splashHeight, uiMargin, uiMargin,
    defaultWidth - splashHeight, defaultHeight - splashHeight, pluginNameTextSize,
    "L4Reverb");

  return true;
}

} // namespace Vst
} // namespace Steinberg
