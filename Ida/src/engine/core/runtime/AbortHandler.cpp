#include "AbortHandler.h"

#include <iostream>

using namespace v8;

namespace core
{
    AbortHandler::AbortHandler(v8::Isolate *isolate, v8::Local<v8::Object> abortContext,
                               std::function<void()> abortFunction)
        : mIsolate(isolate), mAbortFunction(std::move(abortFunction))
    {
        HandleScope handleScope(isolate);

        auto context = isolate->GetCurrentContext();
        mAbortContext.Reset(isolate, abortContext);

        Local<Function> abortCallback = Function::New(
                                            context,
                                            [](const FunctionCallbackInfo<Value> &args) {
                                                auto abortCallbackFunction = static_cast<std::function<void()> *>(
                                                    args.Data().As<External>()->Value());
                                                (*abortCallbackFunction)();
                                            },
                                            External::New(isolate, &mAbortFunction))
                                            .ToLocalChecked();
        mAbortCallback.Reset(isolate, abortCallback);
    }

    AbortHandler::~AbortHandler()
    {
        mAbortContext.Reset();
        mAbortCallback.Reset();
    }

    void AbortHandler::subscribe()
    {
        HandleScope handleScope(mIsolate);

        Local<Object> ctx = mAbortContext.Get(mIsolate);
        // Checking if this context is already aborted - then abourt immediately instead of subscribing
        if (inscope_isContextAborted(mIsolate, ctx))
        {
            std::cout << "Aborting immediately\n";
            mAbortFunction();
            return;
        }

        // Call abortContext.addOnAbort(abortCallback)
        auto context = mIsolate->GetCurrentContext();
        Local<Function> addOnAbort = Local<Function>::Cast(
            ctx->Get(context, String::NewFromUtf8(mIsolate, "addOnAbort").ToLocalChecked()).ToLocalChecked());

        Local<Value> args[1] = {mAbortCallback.Get(mIsolate)};
        auto addOnAbortResult = addOnAbort->Call(context, ctx, 1, args);

        mIsSubscribed = true;
    }

    void AbortHandler::unsubscribe()
    {
        if (!mIsSubscribed)
        {
            return;
        }

        HandleScope handleScope(mIsolate);

        std::cout << "Unsubscribing from abort context\n";
        auto context = mIsolate->GetCurrentContext();
        Local<Object> ctx = mAbortContext.Get(mIsolate);
        Local<Function> removeOnAbort = Local<Function>::Cast(
            ctx->Get(context, String::NewFromUtf8(mIsolate, "removeOnAbort").ToLocalChecked()).ToLocalChecked());

        Local<Value> args[1] = {mAbortCallback.Get(mIsolate)};
        auto removeOnAbortResult = removeOnAbort->Call(context, ctx, 1, args);

        mIsSubscribed = false;
    }

    bool AbortHandler::inscope_isContextAborted(Isolate *isolate, Local<Object> taskContext)
    {
        if (taskContext.IsEmpty())
        {
            return false;
        }

        auto context = isolate->GetCurrentContext();
        Local<Boolean> isAborted = Local<Boolean>::Cast(
            taskContext->Get(context, String::NewFromUtf8(isolate, "isAborted").ToLocalChecked()).ToLocalChecked());
        return isAborted->BooleanValue(isolate);
    }

    Local<Symbol> AbortHandler::inscope_getSigAbort(Isolate *isolate)
    {
        auto context = isolate->GetCurrentContext();
        auto global = context->Global();

        auto sigabort = Local<Symbol>::Cast(
            global->Get(context, String::NewFromUtf8(isolate, "SIGABORT").ToLocalChecked()).ToLocalChecked());

        return sigabort;
    }
}  // namespace core
