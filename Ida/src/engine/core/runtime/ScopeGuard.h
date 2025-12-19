#pragma once

#include <functional>
#include <unordered_map>

namespace core
{
    class ScopeGuard
    {
    private:
        std::unordered_map<void *, std::function<void(void *)>> mAllocations;

    public:
        ScopeGuard();
        ~ScopeGuard();

        template <typename T>
        T *create(T *obj)
        {
            mAllocations[obj] = [](void *ptr) { delete static_cast<T *>(ptr); };
            return obj;
        }

        void destroy(void *p);

        static ScopeGuard *getInstance();
    };
}  // namespace core
