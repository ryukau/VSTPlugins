// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "fftconvolver.hpp"

namespace SomeDSP {

std::mutex OverlapSaveConvolver::fftwMutex;

} // namespace SomeDSP
