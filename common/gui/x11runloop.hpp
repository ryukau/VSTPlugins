//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2018, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
//
// Modified by Takamitsu Endo, 2020.
//
// RunLoop class is copied from `vstgui4/vstgui/plugin-bindings/vst3editor.cpp`.
// Used only in Linux build.
//

#pragma once

#if LINUX

#include "base/source/fobject.h"
#include "vstgui/lib/vstguibase.h"

#include "pluginterfaces/gui/iplugview.h"
#include "vstgui/lib/platform/linux/x11frame.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

// Map Steinberg Vst Interface to VSTGUI Interface
class RunLoop : public X11::IRunLoop, public AtomicReferenceCounted {
public:
  struct EventHandler : Steinberg::Linux::IEventHandler, public Steinberg::FObject {
    X11::IEventHandler *handler{nullptr};

    void PLUGIN_API onFDIsSet(Steinberg::Linux::FileDescriptor) override
    {
      if (handler) handler->onEvent();
    }
    DELEGATE_REFCOUNT(Steinberg::FObject)
    DEFINE_INTERFACES
    DEF_INTERFACE(Steinberg::Linux::IEventHandler)
    END_DEFINE_INTERFACES(Steinberg::FObject)
  };
  struct TimerHandler : Steinberg::Linux::ITimerHandler, public Steinberg::FObject {
    X11::ITimerHandler *handler{nullptr};

    void PLUGIN_API onTimer() final
    {
      if (handler) handler->onTimer();
    }
    DELEGATE_REFCOUNT(Steinberg::FObject)
    DEFINE_INTERFACES
    DEF_INTERFACE(Steinberg::Linux::ITimerHandler)
    END_DEFINE_INTERFACES(Steinberg::FObject)
  };

  bool registerEventHandler(int fd, X11::IEventHandler *handler) final
  {
    if (!runLoop) return false;

    auto smtgHandler = Steinberg::owned(new EventHandler());
    smtgHandler->handler = handler;
    if (runLoop->registerEventHandler(smtgHandler, fd) == Steinberg::kResultTrue) {
      eventHandlers.push_back(smtgHandler);
      return true;
    }
    return false;
  }
  bool unregisterEventHandler(X11::IEventHandler *handler) final
  {
    if (!runLoop) return false;

    for (auto it = eventHandlers.begin(), end = eventHandlers.end(); it != end; ++it) {
      if ((*it)->handler == handler) {
        runLoop->unregisterEventHandler((*it));
        eventHandlers.erase(it);
        return true;
      }
    }
    return false;
  }
  bool registerTimer(uint64_t interval, X11::ITimerHandler *handler) final
  {
    if (!runLoop) return false;

    auto smtgHandler = Steinberg::owned(new TimerHandler());
    smtgHandler->handler = handler;
    if (runLoop->registerTimer(smtgHandler, interval) == Steinberg::kResultTrue) {
      timerHandlers.push_back(smtgHandler);
      return true;
    }
    return false;
  }
  bool unregisterTimer(X11::ITimerHandler *handler) final
  {
    if (!runLoop) return false;

    for (auto it = timerHandlers.begin(), end = timerHandlers.end(); it != end; ++it) {
      if ((*it)->handler == handler) {
        runLoop->unregisterTimer((*it));
        timerHandlers.erase(it);
        return true;
      }
    }
    return false;
  }

  RunLoop(Steinberg::FUnknown *runLoop) : runLoop(runLoop) {}

private:
  using EventHandlers = std::vector<Steinberg::IPtr<EventHandler>>;
  using TimerHandlers = std::vector<Steinberg::IPtr<TimerHandler>>;
  EventHandlers eventHandlers;
  TimerHandlers timerHandlers;
  Steinberg::FUnknownPtr<Steinberg::Linux::IRunLoop> runLoop;
};

} // namespace Vst
} // namespace Steinberg

#endif
