#include "Debug/Thread.hpp"

#include <coreinit/thread.h>

namespace Library::Debug
{
    OSThread* Thread::raw()
    {
        return _raw;
    }

std::vector<Thread> Thread::all()
{
    OSThread* it = OSGetCurrentThread();
    if (!it) return {};

    // 先頭まで戻る
    while (it->link.prev)
    {
        it = it->link.prev;
    }

    std::vector<Thread> threads;

    // 最後まで走査
    for (OSThread* cur = it; cur; cur = cur->link.next)
    {
        Thread t(cur);
        threads.emplace_back(t);
    }

    return threads;
}

    Thread::Thread(OSThread* raw) : _raw(raw) {}
}