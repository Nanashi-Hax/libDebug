#pragma once

#include <cstdint>

#include <coreinit/exception.h>
#include <coreinit/thread.h>

namespace Library::Debug
{
    void Initialize();
    void Shutdown();
    
    void SetDataBreakpoint(uint32_t address, bool read, bool write);
    void UnsetDataBreakpoint();
}