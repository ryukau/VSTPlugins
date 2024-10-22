// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <cinttypes>

class ParameterInterface {
public:
  virtual double getDefaultNormalized(int32_t tag) = 0;
};
