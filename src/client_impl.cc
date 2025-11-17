// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_impl.h"

#include <iostream>

#include "shared/client_util.h"

namespace app {

Client::Client() {}

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  // Call the default shared implementation.
  shared::OnTitleChange(browser, title);
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  shared::OnAfterCreated(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::OnBeforeClose(browser);
}

bool Client::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefProcessId source_process,
                                      CefRefPtr<CefProcessMessage> message) {
  CefString messageName = message->GetName();
  const CefRefPtr<CefListValue> args = message->GetArgumentList();

  if (args->GetSize() >= 2) {
    const std::string funcName = args->GetString(0);
    const std::string promiseId = args->GetString(1);

    CefRefPtr<CefProcessMessage> response = CefProcessMessage::Create("js_callback");
    CefRefPtr<CefListValue> responseArgs = response->GetArgumentList();

    responseArgs->SetString(0, promiseId);
    responseArgs->SetBool(1, true);

    CefRefPtr<CefValue> result = CefValue::Create();

    if (funcName == "hello") {
      const std::string name = args->GetString(2);
      result->SetString("hello "+ name);
    }
    else {
      result->SetString("Unknown function: " + funcName);
      responseArgs->SetBool(1, false);
    }
    responseArgs->SetValue(2, result);

    // 发送响应到渲染进程
    frame->SendProcessMessage(PID_RENDERER, response);
  }

  return true;
}

}  // namespace app
