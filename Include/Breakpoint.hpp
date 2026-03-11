#pragma once

#include <cstdint>
#include <atomic>
#include <vector>

#include <coreinit/thread.h>

#include "Debug/Breakpoint.hpp"
#include "Buffer.hpp"

namespace Library::Debug
{
    extern "C"
    {
        using OSSwitchThreadCallbackFn = void (*)(OSThread* thread, OSThreadQueue * queue);
        OSSwitchThreadCallbackFn OSSetSwitchThreadCallback(OSSwitchThreadCallbackFn callback);
        extern OSSwitchThreadCallbackFn OSSwitchThreadCallbackDefault;
    }

    class BreakpointManager
    {
    public:
        static void Initialize();
        static void Shutdown();

        static bool IsInitialized();

        static void SetDataBreakpoint(uint32_t address, bool read, bool write, BreakpointSize size);
        static void UnsetDataBreakpoint();

        static void SetInstructionBreakpoint(uint32_t address);
        static void UnsetInstructionBreakpoint();

        static std::vector<RegisterInfo> ConsumeDataBreakInfo();
        static std::vector<RegisterInfo> ConsumeInstructionBreakInfo();

    private:
        static void SetIABR(uint32_t value);
        static void SetDABR(uint32_t value);

        static void SetSwitchThreadCallback(OSSwitchThreadCallbackFn function);

        static BOOL DSIHandler(OSContext* context);
        static BOOL BreakpointHandler(OSContext* context);
        static BOOL TraceHandler(OSContext* context);
        static void SwitchThreadHandler(OSThread* thread, OSThreadQueue*);

    private:
        static inline std::atomic<uint32_t> dabr{0};
        static inline std::atomic<uint32_t> dBreakpointAddress{0};
        static inline std::atomic<uint32_t> dBreakpointSize{0};
        static inline RingBuffer<RegisterInfo, 256> dInfoBuffer{};

    private:
        static inline std::atomic<uint32_t> iabr{0};
        static inline std::atomic<uint32_t> iBreakpointAddress{0};
        static inline RingBuffer<RegisterInfo, 256> iInfoBuffer{};

    private:
        static inline Map<uint32_t, uint32_t, 256> dMap{};
        static inline Map<uint32_t, uint32_t, 256> iMap{}; 

        static constexpr const uint32_t MATCH_DABR_BIT = 1 << 22;
        static constexpr const uint32_t SINGLE_STEP_BIT = 1 << 10;

        static inline bool isInitialized = false;
    };
}