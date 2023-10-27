// (c) 2023 Takamitsu Endo
//
// This file is part of ClangCymbal.
//
// ClangCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ClangCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ClangCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/vstgui.h"

#include "../../../common/gui/plugeditor.hpp"
#include "../../../common/gui/style.hpp"
#include "../parameter.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>
#include <string>

namespace VSTGUI {

class RandomizeButton : public CControl {
  // Note: GCC and Clang can't deduce template when `generateFilterTable` and
  // `generateEnvTable` are defined before use.
private:
  template<typename Rng>
  inline auto generateFilterTable(Rng &rng, Steinberg::Vst::ParamValue maxValue)
  {
    using Pv = Steinberg::Vst::ParamValue;
    constexpr int length = int(fdnMatrixSize);

    std::array<Pv, length> table{};

    std::uniform_real_distribution<Pv> uniform{Pv(-1), Pv(1)};
    std::generate(table.begin(), table.end(), [&]() { return uniform(rng); });

    // Bidirectional filtering.
    Pv v1 = 0;
    for (int idx = 0; idx < length; ++idx) {
      v1 += Pv(0.3) * (table[idx] - v1);
      table[idx] = v1;
    }
    v1 = 0;
    for (int idx = length - 1; idx >= 0; --idx) {
      v1 += Pv(0.3) * (table[idx] - v1);
      table[idx] = v1;
    }

    // Random scaling.
    Pv max = 0;
    for (int i = 0; i < length; ++i) max = std::max(max, std::abs(table[i]));
    std::uniform_real_distribution<Pv> uniform01{Pv(0), Pv(maxValue)};
    if (max != 0) {
      auto scaler = uniform01(rng) / max;
      for (int i = 0; i < length; ++i) table[i] *= scaler;
    }

    // Normalze range from [-1, 1] to [0, 1].
    for (int i = 0; i < length; ++i) table[i] = Pv(0.5) * (Pv(1) + table[i]);

    return table;
  }

  template<typename Rng> inline auto generateEnvTable(Rng &rng)
  {
    using Pv = Steinberg::Vst::ParamValue;

    std::array<Pv, nModEnvelopeWavetable> table{};

    std::uniform_real_distribution<Pv> uniform{Pv(0), Pv(1)};
    std::generate(table.begin(), table.end(), [&]() { return uniform(rng); });

    Pv v1 = Pv(0);
    Pv v2 = Pv(1);
    std::uniform_real_distribution<Pv> uniformLower{Pv(0), Pv(0.25)};
    Pv k1 = std::pow(uniformLower(rng), Pv(1.25));
    Pv k2 = uniform(rng) * Pv(0.5);
    k2 *= k2;
    for (int idx = 0; idx < nModEnvelopeWavetable; ++idx) {
      v1 += k1 * (Pv(1) - v1);
      v2 += k2 * (Pv(0) - v2);
      table[idx] = v1 * v2;
    }

    const auto iter = std::max_element(table.begin(), table.end());
    if (iter == table.end()) return table;
    auto max = *iter;
    if (max == 0) return table;
    for (int idx = 0; idx < nModEnvelopeWavetable; ++idx) table[idx] /= max;

    return table;
  }

public:
  std::string label;

  RandomizeButton(
    const CRect &size,
    IControlListener *listener,
    int32_t tag,
    std::string label,
    const SharedPointer<CFontDesc> &fontId,
    Uhhyou::Palette &palette,
    Steinberg::Vst::PlugEditor *editor)
    : CControl(size, listener, tag)
    , label(label)
    , fontId(fontId)
    , pal(palette)
    , editor(editor)
  {
    if (editor) editor->remember();
  }

  ~RandomizeButton()
  {
    if (editor) editor->forget();
  }

  void draw(CDrawContext *pContext) override
  {
    pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
    CDrawContext::Transform t(
      *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

    // Border.
    const double borderW = isMouseEntered ? 2 * borderWidth : borderWidth;
    const double halfBorderWidth = int(borderW / 2.0);
    pContext->setFillColor(isPressed ? pal.highlightButton() : pal.boxBackground());
    pContext->setFrameColor(
      isMouseEntered && !isPressed ? pal.highlightButton() : pal.border());
    pContext->setLineWidth(borderW);
    pContext->drawRect(
      CRect(
        halfBorderWidth, halfBorderWidth, getWidth() - halfBorderWidth,
        getHeight() - halfBorderWidth),
      kDrawFilledAndStroked);

    // Text
    pContext->setFont(fontId);
    pContext->setFontColor(pal.foreground());
    pContext->drawString(
      label.c_str(), CRect(0, 0, getWidth(), getHeight()), kCenterText);
  }

  void onMouseEnterEvent(MouseEnterEvent &event) override
  {
    isMouseEntered = true;
    invalid();
    event.consumed = true;
  }

  void onMouseExitEvent(MouseExitEvent &event) override
  {
    if (value == 1.0f) {
      value = 0.0f;
    }
    isPressed = false;
    isMouseEntered = false;
    invalid();
    event.consumed = true;
  }

  void onMouseDownEvent(MouseDownEvent &event) override
  {
    using ID = Steinberg::Synth::ParameterID::ID;
    using Pv = Steinberg::Vst::ParamValue;

    if (!event.buttonState.isLeft()) return;
    isPressed = true;
    value = 1.0f;

    if (editor) {
      using Rng = std::mt19937_64;

      std::random_device source;
      std::random_device::result_type
        random_data[(Rng::state_size - 1) / sizeof(source()) + 1];
      std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
      std::seed_seq seeds(std::begin(random_data), std::end(random_data));
      Rng rng(seeds);

      std::uniform_real_distribution<Pv> uniform{Pv(0), Pv(1)};
      std::uniform_real_distribution<Pv> uniformUpperHalf{Pv(0.5), Pv(1)};
      std::uniform_real_distribution<Pv> uniformLowerHalf{Pv(0), Pv(0.5)};
      if (uniform(rng) < Pv(0.5)) {
        setParam(ID::impulseGain, Pv(1));
        setParam(ID::oscGain, uniform(rng));
      } else {
        setParam(ID::impulseGain, uniform(rng));
        setParam(ID::oscGain, Pv(1));
      }
      setParam(ID::oscNoisePulseRatio, uniform(rng));
      setParam(ID::oscAttack, uniformLowerHalf(rng));
      setParam(ID::oscDecay, uniformUpperHalf(rng));
      std::uniform_real_distribution<Pv> uniformOscDensityHz{Pv(0.5), Pv(5) / Pv(6)};
      setParam(ID::oscDensityHz, uniformOscDensityHz(rng));
      // setParam(ID::oscDensityKeyFollow, uniform(rng));
      setParam(ID::oscNoiseDecay, uniform(rng));
      setParam(ID::oscBounce, uniform(rng));
      setParam(ID::oscBounceCurve, uniform(rng));
      setParam(ID::oscJitter, uniform(rng));
      setParam(ID::oscPulseAmpRandomness, uniform(rng));
      // setParam(ID::oscLowpassCutoffSemi, uniform(rng));
      setParam(ID::oscLowpassQ, uniform(rng));
      setParam(ID::oscLowpassKeyFollow, uniform(rng));

      setParam(ID::fdnMatrixIdentityAmount, uniform(rng));
      // setParam(ID::fdnFeedback, uniform(rng));
      setParam(ID::fdnOvertoneAdd, uniform(rng));
      std::uniform_real_distribution<Pv> uniformFdnOvertoneMul{Pv(0), Pv(0.6)};
      setParam(ID::fdnOvertoneMul, uniformFdnOvertoneMul(rng));
      setParam(ID::fdnOvertoneOffset, uniform(rng));
      setParam(ID::fdnOvertoneModulo, uniform(rng));
      std::uniform_real_distribution<Pv> uniformFdnOvertoneRandomness{Pv(0), Pv(0.01)};
      setParam(ID::fdnOvertoneRandomness, uniformFdnOvertoneRandomness(rng));
      std::uniform_real_distribution<Pv> uniformFdnInterpRate{Pv(0.25), Pv(0.75)};
      setParam(ID::fdnInterpRate, uniformFdnInterpRate(rng));
      setParam(ID::fdnInterpLowpassSecond, uniform(rng));
      setParam(ID::fdnSeed, uniform(rng));
      std::uniform_real_distribution<Pv> uniformFdnRandomizeRatio{Pv(0), Pv(0.2)};
      setParam(ID::fdnRandomizeRatio, uniformFdnRandomizeRatio(rng));

      auto lpCut = generateFilterTable(rng, Pv(1));
      auto lpQ = generateFilterTable(rng, Pv(1));
      auto hpCut = generateFilterTable(rng, Pv(0.1));
      auto hpQ = generateFilterTable(rng, Pv(1));
      for (int idx = 0; idx < fdnMatrixSize; ++idx) {
        setParam(ID::fdnLowpassCutoffSemiOffset0 + idx, lpCut[idx]);
        setParam(ID::fdnLowpassQOffset0 + idx, lpQ[idx]);
        setParam(ID::fdnHighpassCutoffSemiOffset0 + idx, hpCut[idx]);
        setParam(ID::fdnHighpassQOffset0 + idx, hpQ[idx]);
      }

      std::uniform_real_distribution<Pv> uniformLpSemi{Pv(0.6), Pv(1)};
      setParam(ID::fdnLowpassCutoffSemi, uniformLpSemi(rng));
      // setParam(ID::fdnLowpassCutoffSlope, uniform(rng));
      // setParam(ID::fdnLowpassQ, uniform(rng));
      setParam(ID::fdnLowpassQSlope, uniform(rng));
      // setParam(ID::fdnLowpassKeyFollow, uniform(rng));
      std::uniform_real_distribution<Pv> uniformHpSemi{Pv(0), Pv(0.55)};
      setParam(ID::fdnHighpassCutoffSemi, uniformHpSemi(rng));
      // setParam(ID::fdnHighpassCutoffSlope, uniform(rng));
      // setParam(ID::fdnHighpassQ, uniform(rng));
      setParam(ID::fdnHighpassQSlope, uniform(rng));
      // setParam(ID::fdnHighpassKeyFollow, uniform(rng));

      auto table = generateEnvTable(rng);
      for (int idx = 0; idx < nModEnvelopeWavetable; ++idx) {
        setParam(ID::modEnvelopeWavetable0 + idx, table[idx]);
      }
      setParam(ID::modEnvelopeTime, uniform(rng));
      if (uniform(rng) < Pv(0.25)) {
        setParam(ID::modEnvelopeToFdnLowpassCutoff, uniform(rng));
        setParam(ID::modEnvelopeToFdnHighpassCutoff, uniform(rng));
      } else {
        setParam(ID::modEnvelopeToFdnLowpassCutoff, Pv(0.5));
        setParam(ID::modEnvelopeToFdnHighpassCutoff, Pv(0.5));
      }
      if (uniform(rng) < Pv(0.33)) {
        setParam(ID::modEnvelopeToFdnPitch, uniform(rng));
        setParam(ID::modEnvelopeToFdnOvertoneAdd, uniform(rng));
      } else {
        setParam(ID::modEnvelopeToFdnPitch, Pv(0.5));
        setParam(ID::modEnvelopeToFdnOvertoneAdd, Pv(0));
      }
      setParam(ID::modEnvelopeToOscJitter, uniform(rng));
      setParam(ID::modEnvelopeToOscNoisePulseRatio, uniform(rng));

      // setParam(ID::tremoloMix, uniform(rng));
      setParam(ID::tremoloDepth, uniform(rng));
      setParam(ID::tremoloDelayTime, uniform(rng));
      setParam(ID::tremoloModulationToDelayTimeOffset, uniform(rng));
      setParam(ID::tremoloModulationRateHz, uniform(rng));
    }

    invalid();
    event.consumed = true;
  }

  void onMouseUpEvent(MouseUpEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0.0f;
      invalid();
    }
    event.consumed = true;
  }

  void onMouseCancelEvent(MouseCancelEvent &event) override
  {
    if (isPressed) {
      isPressed = false;
      value = 0;
      invalid();
    }
    isMouseEntered = false;
    event.consumed = true;
  }

  void setBorderWidth(CCoord width) { borderWidth = width < 0 ? 0 : width; }

  CLASS_METHODS(RandomizeButton, CControl);

private:
  // Before calling check if `editor` is not nullptr.
  inline void setParam(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value)
  {
    editor->valueChanged(id, value);
    editor->updateUI(id, value);
  }

  Steinberg::Vst::PlugEditor *editor = nullptr;

  SharedPointer<CFontDesc> fontId;
  Uhhyou::Palette &pal;

  CCoord borderWidth = 1.0;

  bool isPressed = false;
  bool isMouseEntered = false;
};

} // namespace VSTGUI
