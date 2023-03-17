// (c) 2023 Takamitsu Endo
//
// This file is part of TestBedSynth.
//
// TestBedSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TestBedSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TestBedSynth.  If not, see <https://www.gnu.org/licenses/>.

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

constexpr size_t nModulation = nOscillator * ModID::MODID_ENUM_LENGTH;
