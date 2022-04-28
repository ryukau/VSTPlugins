// (c) 2022 Takamitsu Endo
//
// This file is part of OddPowShaper.
//
// OddPowShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OddPowShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OddPowShaper.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vstgui/vstgui.h"

#include "../../../common/gui/style.hpp"
#include "../../../common/gui/textview.hpp"
#include "../dsp/oddpowshaper.hpp"

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
  SomeDSP::OddPowShaper<float> shaper;

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

    auto &&clip = 2.0f; // [0, clip] is the range of horizontal axis.
    auto &&inputAmplitude = meterScale.map(getValueNormalized());
    if (inputAmplitude > holdValue) {
      holdTime = 0;
      holdValue = inputAmplitude;
    } else if (holdTime >= holdDuration) {
      holdValue = std::fmin(holdValue, clip);
      holdValue = std::max(0.0f, holdValue - float(duration) / holdDuration);
    }

    // Update wave shaping curve.
    float maxCurveY = 0.0f;
    points.resize(size_t(width));
    for (size_t idx = 0; idx < points.size(); ++idx) {
      auto &&value = shaper.process(clip * float(idx) / (float(width) - 1.0f));
      points[idx].x = idx;
      points[idx].y = value;
      if (maxCurveY < value) maxCurveY = value;
    }
    for (size_t idx = 0; idx < points.size(); ++idx) {
      points[idx].y = ampToHeight(maxCurveY, points[idx].y);
    }

    // Immediate value.
    auto &&immediateAmp = ampToWidth(clip, inputAmplitude);
    auto &&immediateIndex = size_t(immediateAmp);
    if (immediateIndex >= points.size()) immediateIndex = points.size() - 1;
    auto &&immediatePoint = points[immediateIndex];
    pContext->setFillColor(pal.overlayHighlight());
    pContext->drawRect(CRect(0.0, immediatePoint.y, immediateAmp, height), kDrawFilled);

    // Peak value.
    pContext->setLineWidth(1.0);
    pContext->setFrameColor(pal.highlightAccent());
    auto &&peakWidth = ampToWidth(clip, holdValue);
    auto peakIndex = size_t(peakWidth);
    if (peakIndex >= points.size()) peakIndex = points.size() - 1;
    auto &&peakPoint = points[peakIndex];
    pContext->drawLine(CPoint(peakWidth, peakPoint.y), CPoint(peakWidth, height));
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
