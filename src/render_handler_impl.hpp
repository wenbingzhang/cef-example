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

        "})();";

    // Create an instance of my CefV8Handler object.
    m_handler = new V8HandlerImpl();
    // Register the extension.
    CefRegisterExtension("v8/app", extensionCode, m_handler);
  }

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {}

 private:
  CefRefPtr<V8HandlerImpl> m_handler;

  IMPLEMENT_REFCOUNTING(RenderProcessHandlerImpl);
};
}  // namespace app
