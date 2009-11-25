// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Declaration of a Windows event trace provider class, to allow using
// Windows Event Tracing for logging transport and control.
#ifndef BASE_EVENT_TRACE_PROVIDER_WIN_H_
#define BASE_EVENT_TRACE_PROVIDER_WIN_H_

#include <windows.h>
#include <wmistr.h>
#include <evntrace.h>
#include "base/basictypes.h"

typedef GUID EtwEventClass;
typedef UCHAR EtwEventType;
typedef UCHAR EtwEventLevel;
typedef USHORT EtwEventVersion;
typedef ULONG EtwEventFlags;

// Base class is a POD for correctness.
template <size_t N> struct EtwMofEventBase {
  EVENT_TRACE_HEADER header;
  MOF_FIELD fields[N];
};

// Utility class to auto-initialize event trace header structures.
template <size_t N> class EtwMofEvent: public EtwMofEventBase<N> {
 public:
  typedef EtwMofEventBase<N> Super;

  EtwMofEvent() {
    memset(static_cast<Super*>(this), 0, sizeof(Super));
  }

  EtwMofEvent(const EtwEventClass& event_class, EtwEventType type,
              EtwEventLevel level) {
    memset(static_cast<Super*>(this), 0, sizeof(Super));
    header.Size = sizeof(Super);
    header.Guid = event_class;
    header.Class.Type = type;
    header.Class.Level = level;
    header.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR;
  }

  EtwMofEvent(const EtwEventClass& event_class, EtwEventType type,
              EtwEventVersion version, EtwEventLevel level) {
    memset(static_cast<Super*>(this), 0, sizeof(Super));
    header.Size = sizeof(Super);
    header.Guid = event_class;
    header.Class.Type = type;
    header.Class.Version = version;
    header.Class.Level = level;
    header.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR;
  }

  void SetField(int field, size_t size, const void *data) {
    // DCHECK(field < N);
    if (field < N) {
      fields[field].DataPtr = reinterpret_cast<ULONG64>(data);
      fields[field].Length = size;
    }
  }

  EVENT_TRACE_HEADER* get() { return& header; }

 private:
  DISALLOW_COPY_AND_ASSIGN(EtwMofEvent);
};

// Trace provider with Event Tracing for Windows. The trace provider
// registers with ETW by its name which is a GUID. ETW calls back to
// the object whenever the trace level or enable flags for this provider
// name changes.
// Users of this class can test whether logging is currently enabled at
// a particular trace level, and whether particular enable flags are set,
// before other resources are consumed to generate and issue the log
// messages themselves.
class EtwTraceProvider {
 public:
  // Creates an event trace provider identified by provider_name, which
  // will be the name registered with Event Tracing for Windows (ETW).
  explicit EtwTraceProvider(const GUID& provider_name);

  // Creates an unnamed event trace provider, the provider must be given
  // a name before registration.
  EtwTraceProvider();
  virtual ~EtwTraceProvider();

  // Registers the trace provider with Event Tracing for Windows.
  // Note: from this point forward ETW may call the provider's control
  //    callback. If the provider's name is enabled in some trace session
  //    already, the callback may occur recursively from this call, so
  //    call this only when you're ready to handle callbacks.
  ULONG Register();
  // Unregisters the trace provider with ETW.
  ULONG Unregister();

  // Accessors.
  void set_provider_name(const GUID& provider_name) {
    provider_name_ = provider_name;
  }
  const GUID& provider_name() const { return provider_name_; }
  TRACEHANDLE registration_handle() const { return registration_handle_; }
  TRACEHANDLE session_handle() const { return session_handle_; }
  EtwEventFlags enable_flags() const { return enable_flags_; }
  EtwEventLevel enable_level() const { return enable_level_; }

  // Returns true iff logging should be performed for "level" and "flags".
  // Note: flags is treated as a bitmask, and should normally have a single
  //      bit set, to test whether to log for a particular sub "facility".
  bool ShouldLog(EtwEventLevel level, EtwEventFlags flags) {
    return NULL != session_handle_ && level >= enable_level_ &&
        (0 != (flags & enable_flags_));
  }

  // Simple wrappers to log Unicode and ANSI strings.
  // Do nothing if !ShouldLog(level, 0xFFFFFFFF).
  ULONG Log(const EtwEventClass& event_class, EtwEventType type,
            EtwEventLevel level, const char *message);
  ULONG Log(const EtwEventClass& event_class, EtwEventType type,
            EtwEventLevel level, const wchar_t *message);

  // Log the provided event.
  ULONG Log(EVENT_TRACE_HEADER* event);

 protected:
  // These are called after events have been enabled or disabled.
  // Override them if you want to do processing at the start or
  // end of collection.
  // Note: These may be called ETW's thread and they may be racy.
  virtual void OnEventsEnabled() {}
  virtual void OnEventsDisabled() {}

 private:
  ULONG EnableEvents(PVOID buffer);
  ULONG DisableEvents();
  ULONG Callback(WMIDPREQUESTCODE request, PVOID buffer);
  static ULONG WINAPI ControlCallback(WMIDPREQUESTCODE request, PVOID context,
                                      ULONG *reserved, PVOID buffer);

  GUID provider_name_;
  TRACEHANDLE registration_handle_;
  TRACEHANDLE session_handle_;
  EtwEventFlags enable_flags_;
  EtwEventLevel enable_level_;

  // We don't use this, but on XP we're obliged to pass one in to
  // RegisterTraceGuids. Non-const, because that's how the API needs it.
  static TRACE_GUID_REGISTRATION obligatory_guid_registration_;

  DISALLOW_COPY_AND_ASSIGN(EtwTraceProvider);
};

#endif  // BASE_EVENT_TRACE_PROVIDER_WIN_H_