#pragma once
#include <v8.h>

#include "AbortHandler.h"

namespace core
{
    class PromiseHandler
    {
    private:
        v8::Persistent<v8::Promise::Resolver> mResolver;
        v8::Persistent<v8::Value> mResult;
        v8::Isolate *mIsolate;

        std::unique_ptr<AbortHandler> mAbortHandler;

        bool mSuccess = false;
        bool mDone = false;

        static void microTaskCallback(const v8::FunctionCallbackInfo<v8::Value> &info);

    public:
        PromiseHandler(v8::Isolate *isolate, std::unique_ptr<AbortHandler> abortHandler = nullptr);
        ~PromiseHandler();

        void inscope_resolve(v8::Local<v8::Value> value = v8::Undefined(v8::Isolate::GetCurrent()));
        void inscope_reject(v8::Local<v8::Value> reason = v8::Undefined(v8::Isolate::GetCurrent()));

        v8::Local<v8::Promise> getPromise()
        {
            return mResolver.Get(mIsolate)->GetPromise();
        }
    };
}  // namespace core
