#include <format>

#include "Impl.hpp"
#include "coreinit/debug.h"
#include "Coreinit/memorymap.h"
#include "whb/log.h"

namespace Exception
{
    std::string SystemReset::name() { return "SystemReset"; }
    OSExceptionType SystemReset::type() { return OSExceptionType::OS_EXCEPTION_TYPE_SYSTEM_RESET; }

    std::string MachineCheck::name() { return "MachineCheck"; }
    OSExceptionType MachineCheck::type() { return OSExceptionType::OS_EXCEPTION_TYPE_MACHINE_CHECK; }

    std::string DSI::name() { return "DSI"; }
    OSExceptionType DSI::type() { return OSExceptionType::OS_EXCEPTION_TYPE_DSI; }

    void DSI::panic(std::string name, OSContext* context)
    {
        if(context)
        {
            uint32_t codeAddress = context->srr0;
            uint32_t dataAddress = context->dar;
            std::string message = std::format("{0} Exception occurred\n", name);
            if(OSIsAddressValid(codeAddress))
            {
                char symbol[1024];
                OSGetSymbolName(codeAddress, symbol, sizeof(symbol));
                message += std::format("Code: {0} Symbol: {1}\n", codeAddress, symbol);
            }
            else
            {
                message += std::format("Code: {0}\n", codeAddress);
            }

            if(OSIsAddressValid(dataAddress))
            {
                char symbol[1024];
                OSGetSymbolName(dataAddress, symbol, sizeof(symbol));
                message += std::format("Code: {0} Symbol: {1}\n", dataAddress, symbol);
            }
            else
            {
                message += std::format("Code: {0}\n", dataAddress);
            }
            WHBLogPrintf("%s", message.c_str());
            OSFatal(message.c_str());
        }
        else
        {
            std::string message = std::format("{0} Exception occurred", name);
        }
    }
    
    bool DSI::handle(OSContext* context)
    {
        // DABR による例外でなければクラッシュ
        if ((context->dsisr & (1 << 22)) == 0)
        {
            // メッセージを表示してクラッシュ
            panic("DSI", context);
            return false;
        }
        else
        {
            // 通知の準備
            queue.push_back(*context);

            // 復帰処理
            context->srr0 += 4;
            return true;
        }
    }

    std::string ISI::name() { return "ISI"; }
    OSExceptionType ISI::type() { return OSExceptionType::OS_EXCEPTION_TYPE_ISI; }

    void ISI::panic(std::string name, OSContext* context)
    {
        if(context)
        {
            uint32_t codeAddress = context->srr0;
            std::string message = std::format("{0} Exception occurred\n", name);
            if(OSIsAddressValid(codeAddress))
            {
                char symbol[1024];
                OSGetSymbolName(codeAddress, symbol, sizeof(symbol));
                message += std::format("Code: {0} Symbol: {1}\n", codeAddress, symbol);
            }
            else
            {
                message += std::format("Code: {0}\n", codeAddress);
            }
            WHBLogPrintf("%s", message.c_str());
            OSFatal(message.c_str());
        }
    }

    std::string ExternalInterrupt::name() { return "ExternalInterrupt"; }
    OSExceptionType ExternalInterrupt::type() { return OSExceptionType::OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT; }

    std::string Alignment::name() { return "Alignment"; }
    OSExceptionType Alignment::type() { return OSExceptionType::OS_EXCEPTION_TYPE_ALIGNMENT; }

    void Alignment::panic(std::string name, OSContext* context)
    {
        if(context)
        {
            uint32_t codeAddress = context->srr0;
            uint32_t dataAddress = context->dar;
            std::string message = std::format("{0} Exception occurred\n", name);
            if(OSIsAddressValid(codeAddress))
            {
                char symbol[1024];
                OSGetSymbolName(codeAddress, symbol, sizeof(symbol));
                message += std::format("Code: {0} Symbol: {1}\n", codeAddress, symbol);
            }
            else
            {
                message += std::format("Code: {0}\n", codeAddress);
            }

            if(OSIsAddressValid(dataAddress))
            {
                char symbol[1024];
                OSGetSymbolName(dataAddress, symbol, sizeof(symbol));
                message += std::format("Code: {0} Symbol: {1}\n", dataAddress, symbol);
            }
            else
            {
                message += std::format("Code: {0}\n", dataAddress);
            }
            WHBLogPrintf("%s", message.c_str());
            OSFatal(message.c_str());
        }
        else
        {
            std::string message = std::format("{0} Exception occurred", name);
        }
    }

    std::string Program::name() { return "Program"; }
    OSExceptionType Program::type() { return OS_EXCEPTION_TYPE_PROGRAM; }

    std::string FloatingPoint::name() { return "FloatingPoint"; }
    OSExceptionType FloatingPoint::type() { return OS_EXCEPTION_TYPE_FLOATING_POINT; }

    std::string Decrementer::name() { return "Decrementer"; }
    OSExceptionType Decrementer::type() { return OS_EXCEPTION_TYPE_DECREMENTER; }

    std::string SystemCall::name() { return "SystemCall"; }
    OSExceptionType SystemCall::type() { return OS_EXCEPTION_TYPE_SYSTEM_CALL; }

    std::string Trace::name() { return "Trace"; }
    OSExceptionType Trace::type() { return OS_EXCEPTION_TYPE_TRACE; }

    std::string PerformanceMonitor::name() { return "PerformanceMonitor"; }
    OSExceptionType PerformanceMonitor::type() { return OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR; }

    std::string Breakpoint::name() { return "Breakpoint"; }
    OSExceptionType Breakpoint::type() { return OS_EXCEPTION_TYPE_BREAKPOINT; }

    std::string SystemInterrupt::name() { return "SystemInterrupt"; }
    OSExceptionType SystemInterrupt::type() { return OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT; }

    std::string ICI::name() { return "ICI"; }
    OSExceptionType ICI::type() { return OS_EXCEPTION_TYPE_ICI; }
}