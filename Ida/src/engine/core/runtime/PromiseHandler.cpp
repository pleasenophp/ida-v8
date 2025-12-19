#include "PromiseHandler.h"

#include "ScopeGuard.h"

using namespace v8;

namespace core
{

    PromiseHandler::PromiseHandler(v8::Isolate *isolate, std::unique_ptr<AbortHandler> abortHandler)
        : mIsolate(isolate), mAbortHandler(std::move(abortHandler))
    {
        HandleScope handleScope(isolate);

        auto context = isolate->GetCurrentContext();
        auto promiseResolver = Promise::Resolver::New(context).ToLocalChecked();
        mResolver.Reset(isolate, promiseResolver);
    }

    PromiseHandler::~PromiseHandler()
    {
        mResolver.Reset();
        mResult.Reset();
    }

    void PromiseHandler::inscope_resolve(Local<Value> value)
    {
        if (mDone)
        {
            return;
        }

        mSuccess = true;
        mResult.Reset(mIsolate, value);

        Local<Function> microtaskFunction =
            Function::New(mIsolate->GetCurrentContext(), microTaskCallback, External::New(mIsolate, this))
                .ToLocalChecked();

        mIsolate->EnqueueMicrotask(microtaskFunction);
        mDone = true;
    }

    void PromiseHandler::inscope_reject(Local<Value> reason)
    {
        if (mDone)
        {
            return;
        }

        mSuccess = false;
        mResult.Reset(mIsolate, reason);

        Local<Function> microtaskFunction =
            Function::New(mIsolate->GetCurrentContext(), microTaskCallback, External::New(mIsolate, this))
                .ToLocalChecked();

        mIsolate->EnqueueMicrotask(microtaskFunction);
        mDone = true;
    }

    void PromiseHandler::microTaskCallback(const v8::FunctionCallbackInfo<v8::Value> &info)
    {
        auto promiseHandler = static_cast<PromiseHandler *>(info.Data().As<External>()->Value());
        Isolate *isolate = promiseHandler->mIsolate;

        HandleScope handleScope(isolate);

        Local<Context> context = isolate->GetCurrentContext();
        Local<Promise::Resolver> localResolver = promiseHandler->mResolver.Get(isolate);
        Local<Value> result = promiseHandler->mResult.Get(isolate);

        // Make sure we unsubscribe the abort callback function
        if (promiseHandler->mAbortHandler)
        {
            promiseHandler->mAbortHandler->unsubscribe();
        }

        bool success = promiseHandler->mSuccess;

        ScopeGuard::getInstance()->destroy(promiseHandler);
        promiseHandler = nullptr;

        if (success)
        {
            localResolver->Resolve(context, result).ToChecked();
        }
        else
        {
            std::string abortError(AbortHandler::AbortError);
            if (result->Equals(context, String::NewFromUtf8(isolate, abortError.c_str()).ToLocalChecked()).ToChecked())
            {
                result = AbortHandler::inscope_getSigAbort(isolate);
            }
            localResolver->Reject(context, result).ToChecked();
        }
    }
}  // namespace core
