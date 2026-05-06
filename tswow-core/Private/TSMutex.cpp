#include "TSMutex.h"
#include <mutex>

TSMutex::TSMutex() = default;

TSMutex::TSMutex(TSMutex const&)
{
    // Creates a new independent mutex; needed because TSWorldEntity is copyable
    // (Battlegrounds path) and the default copy constructor on std::mutex is
    // deleted.
}

void TSMutex::lock()
{
    _lock.lock();
}

void TSMutex::unlock()
{
    _lock.unlock();
}

bool TSMutex::try_lock()
{
    return _lock.try_lock();
}
