// SPDX-License-Identifier: GPL-3.0-only
//
// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "pluginterfaces/base/fplatform.h"

#define MAJOR_VERSION_STR "0"
#define MAJOR_VERSION_INT 0

#define SUB_VERSION_STR "1"
#define SUB_VERSION_INT 1

#define RELEASE_NUMBER_STR "20"
#define RELEASE_NUMBER_INT 20

#define BUILD_NUMBER_STR "21"
#define BUILD_NUMBER_INT 21

#define FULL_VERSION_STR                                                                 \
  MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringPluginName "BasicLimiter"

#define stringOriginalFilename "BasicLimiter.vst3"
#if SMTG_PLATFORM_64
  #define stringFileDescription stringPluginName " VST3-SDK (64Bit)"
#else
  #define stringFileDescription stringPluginName " VST3-SDK"
#endif
#define stringCompanyName "Uhhyou\0"
#define stringCompanyWeb ""
#define stringCompanyEmail "ryukau@gmail.com"

#define stringLegalCopyright "Copyright 2021 Takamitsu Endo"
#define stringLegalTrademarks "VST is a trademark of Steinberg Media Technologies GmbH"
