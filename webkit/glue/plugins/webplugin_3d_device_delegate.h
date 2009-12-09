// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_GLUE_PLUGINS_WEBPLUGIN_3D_DEVICE_DELEGATE_H_
#define WEBKIT_GLUE_PLUGINS_WEBPLUGIN_3D_DEVICE_DELEGATE_H_

#include "base/basictypes.h"
#include "third_party/npapi/bindings/npapi_extensions.h"

namespace webkit_glue {

// Interface for the NPAPI 3D device extension. This class implements "NOP"
// versions of all these functions so it can be used seamlessly by the
// "regular" plugin delegate while being overridden by the "pepper" one.
class WebPlugin3DDeviceDelegate {
 public:
  virtual NPError Device3DQueryCapability(int32 capability, int32* value) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DQueryConfig(const NPDeviceContext3DConfig* request,
                                      NPDeviceContext3DConfig* obtain) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DInitializeContext(
      const NPDeviceContext3DConfig* config,
      NPDeviceContext3D* context) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DSetStateContext(NPDeviceContext3D* context,
                                          int32 state,
                                          int32 value) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DGetStateContext(NPDeviceContext3D* context,
                                          int32 state,
                                          int32* value) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DFlushContext(NPP id,
                                       NPDeviceContext3D* context,
                                       NPDeviceFlushContextCallbackPtr callback,
                                       void* user_data) {
    return NPERR_GENERIC_ERROR;
  }
  virtual NPError Device3DDestroyContext(NPDeviceContext3D* context) {
    return NPERR_GENERIC_ERROR;
  }

 protected:
  WebPlugin3DDeviceDelegate() {}
  virtual ~WebPlugin3DDeviceDelegate() {}
};

}  // namespace webkit_glue

#endif  // WEBKIT_GLUE_PLUGINS_WEBPLUGIN_3D_DEVICE_DELEGATE_H_