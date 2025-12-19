#pragma once
#include <v8.h>

#include <string>

namespace core
{
    class AbortHandler
    {
    private:
        v8::Persistent<v8::Object> mAbortContext;
        v8::Persistent<v8::Function> mAbortCallback;
        std::function<void()> mAbortFunction;
        v8::Isolate *mIsolate;
        bool mIsSubscribed = false;

        static bool inscope_isContextAborted(v8::Isolate *isolate, v8::Local<v8::Object> taskContext);

    public:
        AbortHandler(v8::Isolate *isolate, v8::Local<v8::Object> abortContext, std::function<void()> abortFunction);
        ~AbortHandler();

        void subscribe();
        void unsubscribe();

        static constexpr std::string_view AbortError = "__abort";
        static constexpr std::string_view SIGABORT = "SIGABORT";
        static v8::Local<v8::Symbol> inscope_getSigAbort(v8::Isolate *isolate);
    };
}  // namespace core
