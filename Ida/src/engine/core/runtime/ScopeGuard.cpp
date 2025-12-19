#include "ScopeGuard.h"

namespace core
{

    static ScopeGuard *mInstance = nullptr;

    ScopeGuard::ScopeGuard()
    {
        mInstance = this;
    }

    ScopeGuard::~ScopeGuard()
    {
        mInstance = nullptr;
        for (auto &[ptr, deleter] : mAllocations)
        {
            deleter(ptr);
        }
        mAllocations.clear();
    }

    void ScopeGuard::destroy(void *p)
    {
        auto it = mAllocations.find(p);
        if (it != mAllocations.end())
        {
            // Deleting the object
            it->second(p);
            mAllocations.erase(it);
        }
    }

    ScopeGuard *ScopeGuard::getInstance()
    {
        return mInstance;
    }
}  // namespace core
