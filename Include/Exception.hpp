#pragma once

#include <coreinit/kernel.h>

namespace Library::Debug::Exception
{
    void Initialize(); // Initialization required for each core
    void SetCallback(OSExceptionType type, OSExceptionCallbackFn function);
}