// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2022 Takamitsu Endo
//
// This file is part of NarrowingDelay.
//
// NarrowingDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NarrowingDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NarrowingDelay.  If not, see <https://www.gnu.org/licenses/>.

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "../../common/fxcontroller.hpp"
#include "editor.hpp"
#include "fuid.hpp"
#include "parameter.hpp"
#include "plugprocessor.hpp"
#include "version.hpp"

// Subcategory for this Plug-in (see PlugType in ivstaudioprocessor.h)
#define stringSubCategory Steinberg::Vst::PlugType::kFxDelay

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
