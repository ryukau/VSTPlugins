// SPDX-License-Identifier: GPL-3.0-only
//
// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// Takamitsu Endo (ryukau@gmail.com)

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "../../common/fxcontroller.hpp"
#include "editor.hpp"
#include "fuid.hpp"
#include "parameter.hpp"
#include "plugprocessor.hpp"
#include "version.hpp"

// Subcategory for this Plug-in (see PlugType in ivstaudioprocessor.h)
#define stringSubCategory Steinberg::Vst::PlugType::kFxPitchShift

BEGIN_FACTORY_DEF(stringCompanyName, stringCompanyWeb, stringCompanyEmail)

DEF_CLASS2(
  INLINE_UID_FROM_FUID(Steinberg::Synth::ProcessorUID),
  PClassInfo::kManyInstances, // cardinality
  kVstAudioEffectClass,       // the component category (do not changed this)
  stringPluginName,           // here the Plug-in name (to be changed)
  Vst::kDistributable,
  stringSubCategory, // Subcategory for this Plug-in (to be changed)
  FULL_VERSION_STR,  // Plug-in version (to be changed)
  kVstVersionString, // SDK Version (do not changed this, use always this define)
  Steinberg::Synth::PlugProcessor::createInstance)

using Controller = Steinberg::Synth::PlugController<Vst::Editor, GlobalParameter>;

DEF_CLASS2(
  INLINE_UID_FROM_FUID(Steinberg::Synth::ControllerUID),
  PClassInfo::kManyInstances,   // cardinality
  kVstComponentControllerClass, // the Controller category (do not changed this)
  stringPluginName
  "Controller",      // controller name (could be the same than component name)
  0,                 // not used here
  "",                // not used here
  FULL_VERSION_STR,  // Plug-in version (to be changed)
  kVstVersionString, // SDK Version (do not changed this, use always this define)
  Controller::createInstance)

END_FACTORY

//------------------------------------------------------------------------
//  Module init/exit
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// called after library was loaded
inline bool InitModule() { return true; }

//------------------------------------------------------------------------
// called after library is unloaded
inline bool DeinitModule() { return true; }
