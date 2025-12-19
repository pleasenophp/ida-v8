#pragma once

#include <v8.h>

// This file contains utility functions to setup the JS objects templates

namespace Ida
{
    template <typename... Events>
    void inscope_declareEvents(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> tmpl, Events... events)
    {
        v8::Local<v8::ObjectTemplate> eventsTpl = v8::ObjectTemplate::New(isolate);

        (void)std::initializer_list<int>{(eventsTpl->Set(v8::String::NewFromUtf8(isolate, events).ToLocalChecked(),
                                                         v8::String::NewFromUtf8(isolate, events).ToLocalChecked()),
                                          0)...};

        tmpl->Set(v8::String::NewFromUtf8(isolate, "Events").ToLocalChecked(), eventsTpl);
    }

    inline void inscope_bindFunctions(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> tmpl,
                                      std::initializer_list<std::pair<const char *, v8::FunctionCallback>> funcs)
    {
        for (auto &f : funcs)
        {
            tmpl->Set(v8::String::NewFromUtf8(isolate, f.first).ToLocalChecked(),
                      v8::FunctionTemplate::New(isolate, f.second));
        }
    }
}  // namespace Ida

// *****  Macros for the templates *****

/// @brief Macro to bind a function to the template
#define FN(fn) {#fn, fn}

#define BEGIN_SCOPE                           \
    v8::Isolate *isolate = args.GetIsolate(); \
    v8::HandleScope handleScope(isolate);

// #define BIND_BRIDGE auto *idaBridge = static_cast<IdaBridge *>(args.This()->GetAlignedPointerFromInternalField(0));
