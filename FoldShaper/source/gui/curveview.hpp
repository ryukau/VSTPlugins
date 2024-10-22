// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "vstgui/vstgui.h"

#include "../../../common/gui/style.hpp"
#include "../../../common/gui/textview.hpp"
#include "../dsp/foldshaper.hpp"

#include <chrono>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace VSTGUI {

template<typename MeterScale> class CurveView : public CControl {
protected:
  inline float ampToDecibel(float x) { return 20.0f * std::log10(x); }

  CLineStyle lineStyle{CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound};
  CDrawContext::PointList points;

  Uhhyou::Palette &pal;
  MeterScale &meterScale;
  TextTableView *infoTextView;

  static constexpr int64_t holdDuration = 500; // milli seconds.
  int64_t holdTime = 2 * holdDuration;
  float holdValue = 0;
  std::chrono::system_clock::time_point lastTime;

  inline float ampToWidth(float clip, float value)
  {
    return float(getWidth()) * value / clip;
  }

  inline float ampToHeight(float clip, float value)
  {
    return float(getHeight()) * (clip - value) / clip;
  }

public:
  SomeDSP::FoldShaper<float> shaper;
  float inputGain = 1.0f;

  CurveView(
    const CRect &size,
    Uhhyou::Palette &palette,
    MeterScale &meterScale,
    TextTableView *infoTextView)
    : CControl(size)
    , pal(palette)
    , meterScale(meterScale)
    , lastTime()
    , infoTextView(infoTextView)
  {
    this->infoTextView->remember();
  }

  ~CurveView()
  {
    if (infoTextView) infoTextView->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    // Prepare drawing.
    const auto width = getWidth();
    const auto height = getHeight();

    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Update amplitude.
    auto now = std::chrono::system_clock::now();
    int64_t duration
      = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
    lastTime = now;

    if (duration > 0 && duration < (std::numeric_limits<int64_t>::max() - holdTime))
      holdTime += duration;

    auto &&clip = inputGain; // [0, clip] is the range of horizontal axis.
    auto &&inputAmplitude = inputGain * meterScale.map(getValueNormalized());
    if (inputAmplitude > holdValue) {
      holdTime = 0;
      holdValue = inputAmplitude;
    } else if (holdTime >= holdDuration) {
      holdValue = std::fmin(holdValue, clip);
      holdValue = std::max(0.0f, holdValue - float(duration) / holdDuration);
    }

    // Update wave shaping curve.
    float minCurveY = 0.0f;
    float maxCurveY = 0.0f;
    points.resize(size_t(width));
    for (size_t idx = 0; idx < points.size(); ++idx) {
      auto &&value = shaper.process(clip * float(idx) / (float(width) - 1.0f));
      points[idx].x = idx;
      points[idx].y = value;
      if (minCurveY > value) minCurveY = value;
      if (maxCurveY < value) maxCurveY = value;
    }
    for (size_t idx = 0; idx < points.size(); ++idx) {
      points[idx].y = ampToHeight(maxCurveY - minCurveY, points[idx].y - minCurveY);
    }

    // Zero line.
    float zeroHeight = ampToHeight(maxCurveY - minCurveY, -minCurveY);
    pContext->setLineWidth(1.0);
    pContext->setFrameColor(pal.highlightMain());
    pContext->drawLine(CPoint(0.0, zeroHeight), CPoint(width, zeroHeight));

    // Immediate value.
    auto &&immediateAmp = ampToWidth(clip, inputAmplitude);
    auto &&immediateIndex = size_t(immediateAmp);
    if (immediateIndex >= points.size()) immediateIndex = points.size() - 1;
    auto &&immediatePoint = points[immediateIndex];
    pContext->setFillColor(pal.overlayHighlight());
    pContext->drawRect(
      CRect(0.0, immediatePoint.y, immediateAmp, zeroHeight), kDrawFilled);

    // Peak value.
    pContext->setLineWidth(1.0);
    pContext->setFrameColor(pal.highlightAccent());
    auto &&peakWidth = ampToWidth(clip, holdValue);
    auto peakIndex = size_t(peakWidth);
    if (peakIndex >= points.size()) peakIndex = points.size() - 1;
    auto &&peakPoint = points[peakIndex];
    pContext->drawLine(CPoint(peakWidth, peakPoint.y), CPoint(peakWidth, zeroHeight));
    pContext->drawLine(CPoint(0.0, peakPoint.y), CPoint(peakPoint.x, peakPoint.y));

    // Wave shaping curve.
    pContext->setFrameColor(pal.foreground());
    pContext->drawPolygon(points, kDrawStroked);

    // Border.
    pContext->setFrameColor(pal.border());
    pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawStroked);

    // Update text. Probably shouldn't be done here.
    auto &&rdlm = TextTableView::rowDelimiter;
    auto &&cdlm = TextTableView::colDelimiter;
    std::ostringstream os;
    os.precision(5);
    os << std::fixed << "Input Peak [dB]" << cdlm << ampToDecibel(holdValue) << rdlm;
    infoTextView->setText(os.str());
    infoTextView->setDirty();
  }

  CLASS_METHODS(CurveView, CControl);
};

} // namespace VSTGUI
