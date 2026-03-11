#pragma once

#include <cstdint>
#include <vector>

#include <coreinit/exception.h>
#include <coreinit/thread.h>

#include "Debug/Breakpoint.hpp"

namespace Library::Debug
{
    void Initialize();
    void Shutdown();
    
    void SetDataBreakpoint(uint32_t address, bool read, bool write, BreakpointSize size);
    void UnsetDataBreakpoint();
    std::vector<RegisterInfo> ConsumeDataBreakInfo();

    void SetInstructionBreakpoint(uint32_t address);
    void UnsetInstructionBreakpoint();
    std::vector<RegisterInfo> ConsumeInstructionBreakInfo();
}