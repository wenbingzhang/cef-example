#include "include/cef_app.h"
#include "include/cef_v8.h"
#include "v8handler_impl.h"

namespace app {
class RenderProcessHandlerImpl : public CefRenderProcessHandler {
 public:
  RenderProcessHandlerImpl() = default;

  void OnWebKitInitialized() override {
    // Define the extension contents.
    std::string extensionCode =
        //-----------------------------------
        // 声明JavaScript里要调用的Cpp方法
        "var app;"
        "if (!app)"
        "  app = {};"
        "(function() {"

        "  app.call = function(name, arguments) {"
        "    native function call();"
        "    return call(name, arguments);"
        "  };"

        "  app.addEventListener = function(name, callback) {"
        "    native function addEventListener();"
        "    return addEventListener(name, callback);"
        "  };"

        "  app.removeEventListener = function(name) {"
        "    native function removeEventListener();"
        "    return removeEventListener(name);"
        "  };"

        "})();";

    // Register the extension.
    CefRegisterExtension("v8/app", extensionCode, V8HandlerImpl::instance());
  }

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {}

  // 处理从浏览器进程返回的消息
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override {
    CefString messageName = message->GetName();

    // 处理JS调用C++的响应消息
    if (messageName == "js_callback") {
      const CefRefPtr<CefV8Context> ctx = frame->GetV8Context();
      ctx->Enter();
      V8HandlerImpl::instance()->HandleProcessMessage(message);
      ctx->Exit();
      return true;
    }
    // 处理C++调用JS的消息
    else if (messageName == "cpp_call_js") {
      const CefRefPtr<CefV8Context> ctx = frame->GetV8Context();
      ctx->Enter();
      V8HandlerImpl::instance()->HandleProcessMessage(message);
      ctx->Exit();
      return true;
    }

    return false;
  }

 private:
  IMPLEMENT_REFCOUNTING(RenderProcessHandlerImpl);
};
}  // namespace app
