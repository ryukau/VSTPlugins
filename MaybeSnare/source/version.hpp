// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2022 Takamitsu Endo
//
// This file is part of MaybeSnare.
//
// MaybeSnare is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MaybeSnare is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MaybeSnare.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/base/fplatform.h"

#define MAJOR_VERSION_STR "0"
#define MAJOR_VERSION_INT 0

#define SUB_VERSION_STR "1"
#define SUB_VERSION_INT 1

#define RELEASE_NUMBER_STR "8"
#define RELEASE_NUMBER_INT 8

#define BUILD_NUMBER_STR "8"
#define BUILD_NUMBER_INT 8

#define FULL_VERSION_STR                                                                 \
  MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringPluginName "MaybeSnare"

#define stringOriginalFilename "MaybeSnare.vst3"
#if SMTG_PLATFORM_64
  #define stringFileDescription stringPluginName " VST3-SDK (64Bit)"
#else
  #define stringFileDescription stringPluginName " VST3-SDK"
#endif
#define stringCompanyName "Uhhyou\0"
#define stringCompanyWeb ""
#define stringCompanyEmail "ryukau@gmail.com"

#define stringLegalCopyright "Copyright 2022 Takamitsu Endo"
#define stringLegalTrademarks "VST is a trademark of Steinberg Media Technologies GmbH"
