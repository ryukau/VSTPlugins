// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

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
