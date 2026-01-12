// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_impl.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include "shared/client_util.h"

namespace app {

Client::Client() : m_shouldStopTimer(false) {}

Client::~Client() {
  // 确保定时器线程已停止
  m_shouldStopTimer = true;
  if (m_timerThread.joinable()) {
    m_timerThread.join();
  }
}

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

  // 只在第一次创建主浏览器时启动定时器，不为 DevTools 浏览器启动
  // 检查定时器是否已经在运行
  if (!m_timerThread.joinable()) {
    // 启动一个定时器，演示C++主动调用JS
    m_timerThread = std::thread([this]() {
      // 等待让页面完全加载
      for (int i = 0; i < 2 && !m_shouldStopTimer; ++i) {
        if (i > 0) {
          std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        if (m_shouldStopTimer)
          break;

        // 主动调用JavaScript
        if (i == 0) {
          std::vector<std::string> args = {
              "来自C++的消息", "这是参数2",
              "时间戳:" +
                  std::to_string(
                      std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count())};
          CallJavaScript("cppEvent", args);
        } else {
          std::vector<std::string> args2 = {"这是第二次调用", "C++->JS通信",
                                            "成功!"};
          CallJavaScript("cppEvent", args2);
        }
      }
    });
  }
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  // 停止定时器线程
  m_shouldStopTimer = true;

  // 等待定时器线程完成（最多等待1秒）
  if (m_timerThread.joinable()) {
    m_timerThread.join();
  }

  // 清理浏览器引用
  m_browser = nullptr;

  // Call the default shared implementation.
  shared::OnBeforeClose(browser);
}

// 新增：C++主动调用JS的实现
void Client::CallJavaScript(const std::string& eventName,
                            const std::vector<std::string>& args) {
  // 检查是否应该停止
  if (m_shouldStopTimer) {
    return;
  }

  // CEF 要求某些操作必须在 UI 线程上执行
  if (!CefCurrentlyOn(TID_UI)) {
    // 如果不在 UI 线程，将任务调度到 UI 线程执行
    CefPostTask(TID_UI,
                base::BindOnce(&Client::CallJavaScript, this, eventName, args));
    return;
  }

  // 在 UI 线程中再次检查
  if (m_shouldStopTimer) {
    return;
  }

  if (!m_browser) {
    return;
  }

  // 检查浏览器是否仍然有效
  if (!m_browser->GetHost() || !m_browser->GetMainFrame()) {
    return;
  }

  CefRefPtr<CefProcessMessage> message =
      CefProcessMessage::Create("cpp_call_js");
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

bool Client::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefProcessId source_process,
                                      CefRefPtr<CefProcessMessage> message) {
  CefString messageName = message->GetName();
  const CefRefPtr<CefListValue> args = message->GetArgumentList();

  if (args->GetSize() >= 2) {
    const std::string funcName = args->GetString(0);
    const std::string promiseId = args->GetString(1);

    CefRefPtr<CefProcessMessage> response =
        CefProcessMessage::Create("js_callback");
    CefRefPtr<CefListValue> responseArgs = response->GetArgumentList();

    responseArgs->SetString(0, promiseId);
    responseArgs->SetBool(1, true);

    CefRefPtr<CefValue> result = CefValue::Create();

    // 启动一个线程来处理耗时操作，但需要正确管理生命周期
    std::thread workerThread(
        [frame, funcName, args, response, responseArgs, result]() {
          if (funcName == "hello") {
            const std::string name = args->GetString(2);
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            result->SetString("hello " + name);
          } else {
            result->SetString("Unknown function: " + funcName);
            responseArgs->SetBool(1, false);
          }
          responseArgs->SetValue(2, result);

          // 发送响应到渲染进程
          frame->SendProcessMessage(PID_RENDERER, response);
        });

    // 使用detach而不是join，避免阻塞消息处理
    workerThread.detach();
  }

  return true;
}

}  // namespace app
