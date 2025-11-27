// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_impl.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "shared/client_util.h"

namespace app {

Client::Client() = default;

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  // Call the default shared implementation.
  shared::OnTitleChange(browser, title);
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  shared::OnAfterCreated(browser);

  // 保存浏览器引用以供后续调用
  m_browser = browser;

  // 启动一个定时器，演示C++主动调用JS
  std::thread([this]() {
    // 等待5秒让页面完全加载
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 主动调用JavaScript
    std::vector<std::string> args = {"来自C++的消息", "这是参数2", "时间戳:" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count())};
    CallJavaScript("cppEvent", args);

    // 再过5秒再次调用
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::vector<std::string> args2 = {"这是第二次调用", "C++->JS通信", "成功!"};
    CallJavaScript("cppEvent", args2);

  }).detach();
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::OnBeforeClose(browser);
}

// 新增：C++主动调用JS的实现
void Client::CallJavaScript(const std::string& eventName, const std::vector<std::string>& args) {
  if (m_browser) {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cpp_call_js");
    CefRefPtr<CefListValue> argsList = message->GetArgumentList();

    // 设置事件名
    argsList->SetString(0, eventName);

    // 设置参数
    for (size_t i = 0; i < args.size(); i++) {
      argsList->SetString(i + 1, args[i]);
    }

    // 发送消息到渲染进程
    m_browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
  }
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

    std::thread t([frame, funcName, args, response, responseArgs, result]() {
      if (funcName == "hello") {
        const std::string name = args->GetString(2);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        result->SetString("hello "+ name);
      }
      else {
        result->SetString("Unknown function: " + funcName);
        responseArgs->SetBool(1, false);
      }
      responseArgs->SetValue(2, result);

      // 发送响应到渲染进程
      frame->SendProcessMessage(PID_RENDERER, response);
    });
    t.detach();
  }

  return true;
}

}  // namespace app
