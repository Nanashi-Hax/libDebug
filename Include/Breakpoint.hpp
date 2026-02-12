#pragma once

#include <cstdint>
#include <atomic>
#include <array>

#include <coreinit/thread.h>

namespace Library::Debug
{
    extern "C"
    {
        using OSSwitchThreadCallbackFn = void (*)(OSThread* thread, OSThreadQueue * queue);
        OSSwitchThreadCallbackFn OSSetSwitchThreadCallback(OSSwitchThreadCallbackFn callback);
        extern OSSwitchThreadCallbackFn OSSwitchThreadCallbackDefault;
    }

    struct DSIInfo
    {
        uint32_t dar;
        uint32_t srr0;
    };

    class BreakpointManager
    {
    public:
        static void Initialize();
        static void Shutdown();

        static bool IsInitialized();

        static void SetDataBreakpoint(uint32_t address, bool read, bool write);
        static void UnsetDataBreakpoint();

    private:
        static void SetIABR(uint32_t value);
        static void SetDABR(uint32_t value);

        static void ExcecuteInstruction(OSContext* context, uint32_t instruction);

        static void SetDSICallback(OSExceptionCallbackFn callback);
        static void SetSwitchThreadCallback(OSSwitchThreadCallbackFn function);

        static BOOL DSIHandler(OSContext* context);
        static void SwitchThreadHandler(OSThread* thread, OSThreadQueue*);

        static inline std::atomic<uint32_t> dabr{0};

        static constexpr size_t DAR_BUF_SIZE = 256;
        static constexpr uint32_t DAR_MASK = DAR_BUF_SIZE - 1;
        static_assert((DAR_BUF_SIZE & (DAR_BUF_SIZE - 1)) == 0, "DAR_BUF_SIZE must be power of two");

        static inline std::array<DSIInfo, DAR_BUF_SIZE> dsiBuf{};
        static inline std::atomic<uint32_t> dsiHead{0};
        static inline std::atomic<uint32_t> dsiTail{0};

        static inline std::atomic<uint32_t> lastThreadAddr = 0;
        static inline std::atomic<uint32_t> lastDabrSet = 0;

        static inline bool isInitialized = false;
    };
}