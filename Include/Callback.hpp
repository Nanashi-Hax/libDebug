#pragma once

#include <coreinit/context.h>
#include <coreinit/exception.h>

namespace Exception
{
    class Callback
    {
    public:
        template<class Exception>
        static BOOL trampoline(OSContext* ctx)
        {
            static Exception instance;
            return static_cast<BOOL>(instance.handle(ctx));
        }

        template<class Exception>
        static void set()
        {
            OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, Exception::type(), &trampoline<Exception>);
        }
    };
}