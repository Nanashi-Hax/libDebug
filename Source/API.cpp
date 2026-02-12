#include "Breakpoint.hpp"
#include "Exception/Patch.hpp"
#include "Debug.hpp"
#include "Syscall.hpp"

#include <cstdint>
#include <kernel/kernel.h>

namespace Library::Debug
{
    void Initialize()
    {
        Patch::apply();
        
        BreakpointManager::Initialize();

        //KernelPatchSyscall(0x6D, reinterpret_cast<uint32_t>(&SC_SetIABR));
        KernelPatchSyscall(0x6D, reinterpret_cast<uint32_t>(&SC_SetDABR));
    }

    void Shutdown()
    {
        BreakpointManager::Shutdown();
    }

    void SetDataBreakpoint(uint32_t address, bool read, bool write)
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::SetDataBreakpoint(address, read, write);
    }

    void UnsetDataBreakpoint()
    {
        if(!BreakpointManager::IsInitialized()) return;
        BreakpointManager::UnsetDataBreakpoint();
    }
}