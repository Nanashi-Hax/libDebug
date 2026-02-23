#include <atomic>
#include <coreinit/kernel.h>
#include <coreinit/debug.h>
#include <coreinit/thread.h>
#include <coreinit/exception.h>
#include <coreinit/core.h>

namespace Library::Debug::Exception
{
    struct Callback
    {
        OSExceptionCallbackFn systemReset;
        OSExceptionCallbackFn machineCheck;
        OSExceptionCallbackFn dsi;
        OSExceptionCallbackFn isi;
        OSExceptionCallbackFn externalInterrupt;
        OSExceptionCallbackFn alignment;
        OSExceptionCallbackFn program;
        OSExceptionCallbackFn floatingPoint;
        OSExceptionCallbackFn decrementer;
        OSExceptionCallbackFn systemCall;
        OSExceptionCallbackFn trace;
        OSExceptionCallbackFn performanceMonitor;
        OSExceptionCallbackFn breakpoint;
        OSExceptionCallbackFn systemInterrupt;
        OSExceptionCallbackFn ici;
    };

    struct ChainInfo
    {
        OSExceptionChainInfo systemReset;
        OSExceptionChainInfo machineCheck;
        OSExceptionChainInfo dsi;
        OSExceptionChainInfo isi;
        OSExceptionChainInfo externalInterrupt;
        OSExceptionChainInfo alignment;
        OSExceptionChainInfo program;
        OSExceptionChainInfo floatingPoint;
        OSExceptionChainInfo decrementer;
        OSExceptionChainInfo systemCall;
        OSExceptionChainInfo trace;
        OSExceptionChainInfo performanceMonitor;
        OSExceptionChainInfo breakpoint;
        OSExceptionChainInfo systemInterrupt;
        OSExceptionChainInfo ici;
    };

    static constexpr const uint32_t STACK_SIZE = 0x1000;
    alignas(16) static uint8_t sStack[3][STACK_SIZE];
    static OSContext sContext[3];
    static OSThread sThread[3];
    static Callback sCallback[3];
    static ChainInfo sChain[3];
    // 明示的に初期化しておく（静的領域でもゼロ初期化されるが明示的な方が確実で読みやすい）
    static std::atomic<bool> sInHandler[3] = { false, false, false };

    OSExceptionCallbackFn GetCallback(OSExceptionType type, uint32_t core)
    {
        Callback& c = sCallback[core];
        switch (type)
        {
            case OS_EXCEPTION_TYPE_SYSTEM_RESET: return c.systemReset;
            case OS_EXCEPTION_TYPE_MACHINE_CHECK: return c.machineCheck;
            case OS_EXCEPTION_TYPE_DSI: return c.dsi;
            case OS_EXCEPTION_TYPE_ISI: return c.isi;
            case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT: return c.externalInterrupt;
            case OS_EXCEPTION_TYPE_ALIGNMENT: return c.alignment;
            case OS_EXCEPTION_TYPE_PROGRAM: return c.program;
            case OS_EXCEPTION_TYPE_FLOATING_POINT: return c.floatingPoint;
            case OS_EXCEPTION_TYPE_DECREMENTER: return c.decrementer;
            case OS_EXCEPTION_TYPE_SYSTEM_CALL: return c.systemCall;
            case OS_EXCEPTION_TYPE_TRACE: return c.trace;
            case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR: return c.performanceMonitor;
            case OS_EXCEPTION_TYPE_BREAKPOINT: return c.breakpoint;
            case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT: return c.systemInterrupt;
            case OS_EXCEPTION_TYPE_ICI: return c.ici;
            default: return nullptr;
        }
    }

    char const * GetString(OSExceptionType type)
    {
        switch (type)
        {
            case OS_EXCEPTION_TYPE_SYSTEM_RESET: return "SystemReset";
            case OS_EXCEPTION_TYPE_MACHINE_CHECK: return "MachineCheck";
            case OS_EXCEPTION_TYPE_DSI: return "DSI";
            case OS_EXCEPTION_TYPE_ISI: return "ISI";
            case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT: return "ExternalInterrupt";
            case OS_EXCEPTION_TYPE_ALIGNMENT: return "Alignment";
            case OS_EXCEPTION_TYPE_PROGRAM: return "Program";
            case OS_EXCEPTION_TYPE_FLOATING_POINT: return "FloatingPoint";
            case OS_EXCEPTION_TYPE_DECREMENTER: return "Decrementer";
            case OS_EXCEPTION_TYPE_SYSTEM_CALL: return "SystemCall";
            case OS_EXCEPTION_TYPE_TRACE: return "Trace";
            case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR: return "PerformanceMonitor";
            case OS_EXCEPTION_TYPE_BREAKPOINT: return "Breakpoint";
            case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT: return "SystemInterrupt";
            case OS_EXCEPTION_TYPE_ICI: return "ICI";
            default: return "Unknown";
        }
    }

    OSExceptionChainInfo * GetChainInfo(OSExceptionType type, uint32_t core)
    {
        ChainInfo& c = sChain[core];
        switch (type)
        {
            case OS_EXCEPTION_TYPE_SYSTEM_RESET: return &c.systemReset;
            case OS_EXCEPTION_TYPE_MACHINE_CHECK: return &c.machineCheck;
            case OS_EXCEPTION_TYPE_DSI: return &c.dsi;
            case OS_EXCEPTION_TYPE_ISI: return &c.isi;
            case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT: return &c.externalInterrupt;
            case OS_EXCEPTION_TYPE_ALIGNMENT: return &c.alignment;
            case OS_EXCEPTION_TYPE_PROGRAM: return &c.program;
            case OS_EXCEPTION_TYPE_FLOATING_POINT: return &c.floatingPoint;
            case OS_EXCEPTION_TYPE_DECREMENTER: return &c.decrementer;
            case OS_EXCEPTION_TYPE_SYSTEM_CALL: return &c.systemCall;
            case OS_EXCEPTION_TYPE_TRACE: return &c.trace;
            case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR: return &c.performanceMonitor;
            case OS_EXCEPTION_TYPE_BREAKPOINT: return &c.breakpoint;
            case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT: return &c.systemInterrupt;
            case OS_EXCEPTION_TYPE_ICI: return &c.ici;
            default: return nullptr;
        }
    }

    void Handler(OSExceptionType type, OSContext * interruptedContext, OSContext * callbackContext)
    {
        // 1) core をまず安全に決める（interruptedContext->upir が信頼できる前提だが範囲チェック）
        uint32_t core = interruptedContext ? interruptedContext->upir : OSGetCoreId();
        if (core >= 3) core = OSGetCoreId(); // フォールバック（安全策）

        std::atomic<bool>& in = sInHandler[core];

        // 2) 再入検出：取れなければ即抜け（**スピン禁止**）
        bool expected = false;
        if (!in.compare_exchange_strong(expected, true,
                                         std::memory_order_acquire,
                                         std::memory_order_relaxed))
        {
            // 既に同コアで処理中 → 例外ハンドラ内でスピンは危険なので即戻る
            // ここでは何もしないで戻る。必要なら統計/ログフラグだけ立てる（ただしログも慎重に）
            return;
        }

        // 3) 実処理：可能な限り最小に。フラグはコンテキスト切替より前にクリアする必要がある。
        __OSSetCurrentUserContext(callbackContext);

        auto callback = GetCallback(type, core);
        if (callback)
        {
            if (callback(interruptedContext) == TRUE)
            {
                // 成功時：フラグをクリアして元のコンテキストへ戻す
                in.store(false, std::memory_order_release);
                __OSSetAndLoadContext(interruptedContext); // たぶん戻らない
                return; // 防御的に書く（たいていここには戻らない）
            }
            // callback が FALSE を返した場合は下で致命処理へ（フラグはクリアしておく）
        }

        // 失敗/未ハンドル時：フラグをクリアして致命
        in.store(false, std::memory_order_release);
        OSFatal(GetString(type));
    }

    void SetExceptionHandler()
    {
        uint32_t core = OSGetCoreId();

        OSExceptionChainInfo cur;
        cur.callback = Handler;
        cur.stack = sStack[core] + STACK_SIZE;
        cur.context = &sContext[core];
    
        OSExceptionType type;
        OSExceptionChainInfo* prev;
    
        type = OS_EXCEPTION_TYPE_SYSTEM_RESET;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_MACHINE_CHECK;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_DSI;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_ISI;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_ALIGNMENT;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_PROGRAM;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_FLOATING_POINT;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_DECREMENTER;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_SYSTEM_CALL;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_TRACE;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_BREAKPOINT;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        type = OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT;
        prev = GetChainInfo(type, core);
        __KernelSetUserModeExHandler(type, &cur, prev);
    
        // ICI is not set
    }

    int Initializer(int argc, const char **argv)
    {
        Exception::SetExceptionHandler();
        return 0;
    }

    void Initialize()
    {
        for(uint32_t i = 0; i < 3; i++)
        {
            OSThreadAttributes attribute;
            switch(i)
            {
                case 0: attribute = OS_THREAD_ATTRIB_AFFINITY_CPU0; break;
                case 1: attribute = OS_THREAD_ATTRIB_AFFINITY_CPU1; break;
                case 2: attribute = OS_THREAD_ATTRIB_AFFINITY_CPU2; break;
                default: return;
            }

            OSCreateThread
            (
                &sThread[i],
                Initializer,
                0,
                nullptr,
                sStack[i] + STACK_SIZE,
                STACK_SIZE,
                16,
                attribute
            );
        
            OSResumeThread(&sThread[i]);
        }
    }

    void SetCallback(OSExceptionType type, OSExceptionCallbackFn function)
    {
        for(uint32_t i = 0; i < 3; i++)
        {
            Callback& c = sCallback[i];
            switch (type)
            {
                case OS_EXCEPTION_TYPE_SYSTEM_RESET: c.systemReset = function; break;
                case OS_EXCEPTION_TYPE_MACHINE_CHECK: c.machineCheck = function; break;
                case OS_EXCEPTION_TYPE_DSI: c.dsi = function; break;
                case OS_EXCEPTION_TYPE_ISI: c.isi = function; break;
                case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT: c.externalInterrupt = function; break;
                case OS_EXCEPTION_TYPE_ALIGNMENT: c.alignment = function; break;
                case OS_EXCEPTION_TYPE_PROGRAM: c.program = function; break;
                case OS_EXCEPTION_TYPE_FLOATING_POINT: c.floatingPoint = function; break;
                case OS_EXCEPTION_TYPE_DECREMENTER: c.decrementer = function; break;
                case OS_EXCEPTION_TYPE_SYSTEM_CALL: c.systemCall = function; break;
                case OS_EXCEPTION_TYPE_TRACE: c.trace = function; break;
                case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR: c.performanceMonitor = function; break;
                case OS_EXCEPTION_TYPE_BREAKPOINT: c.breakpoint = function; break;
                case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT: c.systemInterrupt = function; break;
                case OS_EXCEPTION_TYPE_ICI: c.ici = function; break;
                default: break;
            }
        }
    }
}
