
#include "v8handler_impl.h"

#include <iostream>

V8HandlerImpl::V8HandlerImpl() {}

V8HandlerImpl::~V8HandlerImpl() {}

// in CefV8HandlerImpl.cpp
bool V8HandlerImpl::Execute(
    const CefString& name  // JavaScript调用的C++方法名字
    ,
    CefRefPtr<CefV8Value> object  // JavaScript调用者对象
    ,
    const CefV8ValueList& arguments  // JavaScript传递的参数
    ,
    CefRefPtr<CefV8Value>& retval  // 需要返回给JavaScript的值设置给这个对象
    ,
    CefString& exception)  // 通知异常信息给JavaScript
{
  bool handle = false;
  if (name == "call" && arguments.size() == 2 && arguments[0]->IsString() &&
      arguments[1]->IsArray()) {
    std::string funcName = arguments[0]->GetStringValue();
    const CefRefPtr<CefV8Value>& argumentsArray = arguments[1];

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(funcName);
    CefRefPtr<CefListValue> argsList = msg->GetArgumentList();
    argsList->SetString(0, funcName);
    for (int i = 0; i < argumentsArray->GetArrayLength(); i++) {
      CefRefPtr<CefV8Value> val = argumentsArray->GetValue(i);
      argsList->SetValue(i + 1, ConvertV8ToCefValue(val));
    }

    CefRefPtr<CefV8Context> ctx = CefV8Context::GetCurrentContext();
    ctx->GetFrame()->SendProcessMessage(PID_BROWSER, msg);

    retval = CefV8Value::CreateString("success");
    handle = true;
  }

  if (!handle) {
    exception = "not implement function";
  }

  return true;
}

CefRefPtr<CefValue> V8HandlerImpl::ConvertV8ToCefValue(
    CefRefPtr<CefV8Value> val) {
  CefRefPtr<CefValue> cefVal = CefValue::Create();

  if (!val.get()) {
    cefVal->SetNull();
    return cefVal;
  }

  if (val->IsString()) {
    cefVal->SetString(val->GetStringValue());
  } else if (val->IsInt() || val->IsUInt()) {
    cefVal->SetInt(val->GetIntValue());
  } else if (val->IsDouble()) {
    cefVal->SetDouble(val->GetDoubleValue());
  } else if (val->IsBool()) {
    cefVal->SetBool(val->GetBoolValue());
  } else if (val->IsNull() || val->IsUndefined()) {
    cefVal->SetNull();
  } else if (val->IsArray()) {
    int len = val->GetArrayLength();
    CefRefPtr<CefListValue> list = CefListValue::Create();

    for (int i = 0; i < len; i++) {
      CefRefPtr<CefV8Value> child = val->GetValue(i);
      list->SetValue(i, ConvertV8ToCefValue(child));
    }
    cefVal->SetList(list);
  } else if (val->IsObject()) {
    CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();

    std::vector<CefString> keys;
    val->GetKeys(keys);

    for (auto& k : keys) {
      CefRefPtr<CefV8Value> child = val->GetValue(k);
      dict->SetValue(k, ConvertV8ToCefValue(child));
    }
    cefVal->SetDictionary(dict);
  } else {
    // 不支持的类型：比如函数, 实例, DOM对象
    cefVal->SetString("[UNSUPPORTED_TYPE]");
  }

  return cefVal;
}
