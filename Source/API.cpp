#include "Breakpoint.hpp"
#include "Debug/Breakpoint.hpp"
#include "Debug.hpp"
#include "Syscall.hpp"
#include "Exception.hpp"

#include <cstdint>
#include <kernel/kernel.h>

namespace Library::Debug
{
    void Initialize()
    {
        Exception::Initialize();
        BreakpointManager::Initialize();

        KernelPatchSyscall(0xC0, reinterpret_cast<uint32_t>(&SC_SetDABR));
        KernelPatchSyscall(0xC1, reinterpret_cast<uint32_t>(&SC_SetIABR));
    }

    void Shutdown()
    {
        BreakpointManager::Shutdown();
    }

    void SetDataBreakpoint(uint32_t address, bool read, bool write, BreakpointSize size)
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::SetDataBreakpoint(address, read, write, size);
    }

    void UnsetDataBreakpoint()
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::UnsetDataBreakpoint();
    }

    std::vector<RegisterInfo> ConsumeDataBreakInfo()
    {
        return BreakpointManager::ConsumeDataBreakInfo();
    }

    void SetInstructionBreakpoint(uint32_t address)
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::SetInstructionBreakpoint(address);
    }

    void UnsetInstructionBreakpoint()
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::UnsetInstructionBreakpoint();
    }

    std::vector<RegisterInfo> ConsumeInstructionBreakInfo()
    {
        return BreakpointManager::ConsumeInstructionBreakInfo();
    }
}