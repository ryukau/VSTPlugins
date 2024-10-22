// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

constexpr size_t nOscillator = 2;
constexpr size_t nOscWavetable = 64;

namespace ModID {

// env: Envelope.
// ext: External modulation source.
enum ModID : size_t {
  env0,
  env1,
  lfo0,
  lfo1,
  ext0,
  ext1,
  MODID_ENUM_LENGTH,
};

} // namespace ModID

constexpr size_t nWaveModInput = ModID::lfo1 + 2;
constexpr size_t nModulation = nOscillator * ModID::MODID_ENUM_LENGTH;
