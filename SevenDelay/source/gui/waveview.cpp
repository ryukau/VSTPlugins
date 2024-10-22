// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "waveview.hpp"

#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
#endif

#include <cmath>

namespace VSTGUI {

WaveView::WaveView(const CRect &size, Uhhyou::Palette &palette)
  : CControl(size), pal(palette)
{
}

void WaveView::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();

  // Background.
  pContext->setFillColor(pal.boxBackground());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  // Waveform.
  pContext->setLineWidth(1.0);
  pContext->setLineStyle(lineStyle);
  pContext->setFrameColor(pal.highlightAccent());
  const size_t size = (size_t)(width + 1.0);
  if (points.size() != size) points.resize(size);
  for (size_t x = 0; x < points.size(); ++x)
    points[x] = CPoint((CCoord)x, height * lfo(x / width));
  pContext->drawPolygon(points);

  // Always draw border at last. Otherwise, inner object will be drawn over border.
  pContext->setLineWidth(1.0);
  pContext->setFrameColor(pal.border());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawStroked);

  setDirty(false);
}

double WaveView::lfo(double phase)
{
  phase = this->phase + phase * 2.0 * M_PI;
  if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
  auto sign = (M_PI < phase) - (phase < M_PI);
  auto wave = amount * sign * pow(fabs(sin(phase)), shape);
  return (wave + 1.0) * 0.5;
}

} // namespace VSTGUI
