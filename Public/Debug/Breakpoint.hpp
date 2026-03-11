#pragma once

#include <cstdint>
#include <cstring>
#include <coreinit/context.h>

namespace Library::Debug
{
    enum class BreakpointSize : uint32_t
    {
        Invalid = 0,
        Bit8 = 1,
        Bit16 = 2,
        Bit32 = 4,
        Bit64 = 8
    };

    struct RegisterInfo
    {
        uint32_t pc;
        uint32_t dar;
        uint32_t gpr[32];
        double fpr[32];
        uint32_t cr;
        uint32_t lr;
        uint32_t ctr;

        static RegisterInfo fromContext(OSContext* context)
        {
            RegisterInfo info;
            info.pc = context->srr0;
            info.dar = context->dar;
            std::memcpy(&info.gpr, context->gpr, 32);
            std::memcpy(&info.fpr, context->fpr, 32);
            info.cr = context->cr;
            info.lr = context->lr;
            info.ctr = context->ctr;
            return info;
        }
    };
}