
#include "v8handler_impl.h"

#include <chrono>
#include <iostream>

#include "shared/uuid_v4.hpp"

V8HandlerImpl::V8HandlerImpl() = default;

V8HandlerImpl::~V8HandlerImpl() = default;

CefRefPtr<V8HandlerImpl> V8HandlerImpl::instance() {
  static CefRefPtr<V8HandlerImpl> instance = new V8HandlerImpl();
  return instance;
}

// in CefV8HandlerImpl.cpp
bool V8HandlerImpl::Execute(
    const CefString& name,  // the name of the c method called by javascript
    CefRefPtr<CefV8Value> object,  // javascript caller object
    const CefV8ValueList& arguments,  // parameters passed by javascript
    CefRefPtr<CefV8Value>& retval,  // The value returned to JavaScript needs to be set to this object
    CefString& exception)  // notify javascript of the exception information
{
  bool handle = false;

  // 原有的JS调用C++功能
  if (name == "call" && arguments.size() == 2 && arguments[0]->IsString() &&
      arguments[1]->IsArray()) {
    const std::string funcName = arguments[0]->GetStringValue();
    const CefRefPtr<CefV8Value>& argumentsArray = arguments[1];

    const CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(funcName);
    const CefRefPtr<CefListValue> argsList = msg->GetArgumentList();
    argsList->SetString(0, funcName);

    const std::string callId = shared::uuid_v4();

    argsList->SetString(1, callId);

    for (int i = 0; i < argumentsArray->GetArrayLength(); i++) {
      const CefRefPtr<CefV8Value> val = argumentsArray->GetValue(i);
      argsList->SetValue(i + 2, ConvertV8ToCefValue(val));
    }

    const CefRefPtr<CefV8Context> ctx = CefV8Context::GetCurrentContext();
    ctx->GetFrame()->SendProcessMessage(PID_BROWSER, msg);

    const CefRefPtr<CefV8Value> promise = CefV8Value::CreatePromise();
    m_promises[callId] = promise;
    retval = promise;

    handle = true;
  }

  // 新增：JS事件监听器注册功能
  else if (name == "addEventListener" && arguments.size() == 2 &&
           arguments[0]->IsString() && arguments[1]->IsFunction()) {
    const std::string eventName = arguments[0]->GetStringValue();
    const CefRefPtr<CefV8Value> callback = arguments[1];

    m_eventListeners[eventName] = callback;
    retval = CefV8Value::CreateBool(true);
    handle = true;
  }

  // 新增：JS移除事件监听器功能
  else if (name == "removeEventListener" && arguments.size() == 1 &&
           arguments[0]->IsString()) {
    const std::string eventName = arguments[0]->GetStringValue();

    auto it = m_eventListeners.find(eventName);
    if (it != m_eventListeners.end()) {
      m_eventListeners.erase(it);
      retval = CefV8Value::CreateBool(true);
    } else {
      retval = CefV8Value::CreateBool(false);
    }
    handle = true;
  }

  if (!handle) {
    exception = "not implement function: " + name.ToString();
  }

  return true;
}
void V8HandlerImpl::HandleProcessMessage(CefRefPtr<CefProcessMessage> message) {
  const std::string messageName = message->GetName();
  const auto args = message->GetArgumentList();

  // 处理JS调用C++的响应消息
  if (messageName == "js_callback") {
    const auto id = args->GetString(0);
    const auto isSuccess = args->GetBool(1);
    const auto result = ConvertCefValueToV8(args->GetValue(2));

    const auto it = m_promises.find(id.ToString());
    if (it != m_promises.end()) {
      auto promise = it->second;
      if (promise && promise->IsPromise()) {
        const CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        if (context) {
          context->Enter();
          if (isSuccess) {
            promise->ResolvePromise(result);
          } else {
            promise->RejectPromise("processing failed");
          }
          context->Exit();
        }
      }
      m_promises.erase(it);
    }
  }
  // 处理C++调用JS的消息
  else if (messageName == "cpp_call_js") {
    const std::string eventName = args->GetString(0);
    std::vector<std::string> callArgs;

    // 提取参数
    for (size_t i = 1; i < args->GetSize(); i++) {
      if (args->GetValue(i)->GetType() == VTYPE_STRING) {
        callArgs.push_back(args->GetString(i));
      }
    }

    // 调用JavaScript函数
    CallJavaScript(eventName, callArgs);
  }
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

CefRefPtr<CefV8Value> V8HandlerImpl::ConvertCefValueToV8(
    CefRefPtr<CefValue> value) {
  if (!value)
    return CefV8Value::CreateNull();

  switch (value->GetType()) {
    case VTYPE_NULL:
      return CefV8Value::CreateNull();
    case VTYPE_BOOL:
      return CefV8Value::CreateBool(value->GetBool());
    case VTYPE_INT:
      return CefV8Value::CreateInt(value->GetInt());
    case VTYPE_DOUBLE:
      return CefV8Value::CreateDouble(value->GetDouble());
    case VTYPE_STRING:
      return CefV8Value::CreateString(value->GetString());
    case VTYPE_LIST: {
      CefRefPtr<CefListValue> list = value->GetList();
      CefRefPtr<CefV8Value> array = CefV8Value::CreateArray(list->GetSize());
      for (size_t i = 0; i < list->GetSize(); i++) {
        array->SetValue(i, ConvertCefValueToV8(list->GetValue(i)));
      }
      return array;
    }
    case VTYPE_DICTIONARY: {
      CefRefPtr<CefDictionaryValue> dict = value->GetDictionary();
      CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);

      CefDictionaryValue::KeyList keys;
      dict->GetKeys(keys);

      for (const auto& key : keys) {
        object->SetValue(key, ConvertCefValueToV8(dict->GetValue(key)),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      }
      return object;
    }
    default:
      return CefV8Value::CreateString("[UNSUPPORTED_TYPE]");
  }
}

// 新增：C++调用JS的实现
void V8HandlerImpl::CallJavaScript(const std::string& eventName, const std::vector<std::string>& args) {
  auto it = m_eventListeners.find(eventName);
  if (it != m_eventListeners.end() && it->second && it->second->IsFunction()) {
    const CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    if (context) {
      context->Enter();

      // 准备参数
      CefV8ValueList v8Args;
      for (const auto& arg : args) {
        v8Args.push_back(CefV8Value::CreateString(arg));
      }

      // 执行JavaScript回调函数
      it->second->ExecuteFunction(nullptr, v8Args);

      context->Exit();
    }
  }
}
