#pragma once
#include <map>
#include <string>
#include "include/cef_v8.h"

class V8HandlerImpl : public CefV8Handler {
 public:
  V8HandlerImpl();
  ~V8HandlerImpl() override;

 public:
  /**
   *	CefV8Handler Methods, Which will be called when the V8 extension
   *  is called in the Javascript environment
   */
  bool Execute(const CefString& name,
               CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments,
               CefRefPtr<CefV8Value>& retval,
               CefString& exception) override;
  CefRefPtr<CefValue> ConvertV8ToCefValue(CefRefPtr<CefV8Value> val);

 private:

  IMPLEMENT_REFCOUNTING(V8HandlerImpl);
};

