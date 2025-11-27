// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_EXAMPLES_MINIMAL_CLIENT_MINIMAL_H_
#define CEF_EXAMPLES_MINIMAL_CLIENT_MINIMAL_H_

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_render_process_handler.h"

namespace app {

// Minimal implementation of client handlers.
class Client : public CefClient,
               public CefDisplayHandler,
               public CefLifeSpanHandler,
               public CefRenderProcessHandler {
 public:
  Client();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

  // 新增：C++主动调用JS的接口
  void CallJavaScript(const std::string& eventName, const std::vector<std::string>& args);

 private:
  // 保存浏览器引用
  CefRefPtr<CefBrowser> m_browser;

  // 管理定时器线程
  std::thread m_timerThread;
  std::atomic<bool> m_shouldStopTimer;

  IMPLEMENT_REFCOUNTING(Client);
  DISALLOW_COPY_AND_ASSIGN(Client);
};

}  // namespace app

#endif  // CEF_EXAMPLES_MINIMAL_CLIENT_MINIMAL_H_
