#include <cstdint>
#include <string>
#include <format>

#include <coreinit/debug.h>
#include <coreinit/memorymap.h>

#include "Breakpoint.hpp"
#include "Syscall.hpp"

namespace Library::Debug
{
    void BreakpointManager::SetDataBreakpoint(uint32_t address, bool read, bool write)
    {
        uint32_t enabled = true;
        uint32_t r = static_cast<uint32_t>(read);
        uint32_t w = static_cast<uint32_t>(write);
        uint32_t value = address | (enabled << 2 | w << 1 | r << 0);
        dabr.store(value, std::memory_order_relaxed);
        dsiHead.store(0, std::memory_order_relaxed);
        dsiTail.store(0, std::memory_order_relaxed);
    }

    void BreakpointManager::UnsetDataBreakpoint()
    {
        dabr.store(0, std::memory_order_relaxed);
        dsiHead.store(0, std::memory_order_relaxed);
        dsiTail.store(0, std::memory_order_relaxed);
    }

    constexpr inline uint32_t OP_LWZ  = 32;
    constexpr inline uint32_t OP_LWZU = 33;
    constexpr inline uint32_t OP_LBZ  = 34;
    constexpr inline uint32_t OP_LBZU = 35;
    constexpr inline uint32_t OP_STW  = 36;
    constexpr inline uint32_t OP_STWU = 37;
    constexpr inline uint32_t OP_STB  = 38;
    constexpr inline uint32_t OP_STBU = 39;
    constexpr inline uint32_t OP_LHZ  = 40;
    constexpr inline uint32_t OP_LHZU = 41;
    constexpr inline uint32_t OP_LHA  = 42;
    constexpr inline uint32_t OP_LHAU = 43;
    constexpr inline uint32_t OP_STH  = 44;
    constexpr inline uint32_t OP_STHU = 45;
    constexpr inline uint32_t OP_LMW  = 46;
    constexpr inline uint32_t OP_STMW = 47;

    OSSwitchThreadCallbackFn OSSwitchThreadCallbackDefault = reinterpret_cast<OSSwitchThreadCallbackFn>(0x0103C4B4);

    void BreakpointManager::Initialize()
    {
        SetSwitchThreadCallback(SwitchThreadHandler);
        isInitialized = true;
    }

    void BreakpointManager::Shutdown()
    {
        isInitialized = false;
        SetSwitchThreadCallback(OSSwitchThreadCallbackDefault);
    }

    bool BreakpointManager::IsInitialized()
    {
        return isInitialized;
    }

    void BreakpointManager::SetIABR(uint32_t value)
    {
        ::SetIABR(value);
    }

    void BreakpointManager::SetDABR(uint32_t value)
    {
        ::SetDABR(value);
    }

    void BreakpointManager::ExcecuteInstruction(OSContext* context, uint32_t instruction)
    {
        if (!context) return;

        uint32_t op   = (instruction >> 26) & 0x3F;
        uint32_t rD   = (instruction >> 21) & 0x1F;
        uint32_t rA   = (instruction >> 16) & 0x1F;
        int16_t simm  = static_cast<int16_t>(instruction & 0xFFFF);

        uint32_t ea;
        if (rA == 0) ea = (int32_t)simm;
        else ea = context->gpr[rA] + (int32_t)simm;

        if (!OSIsAddressValid(ea)) return;

        switch (op)
        {
            case OP_LWZ:
            case OP_LWZU:
            {
                uint32_t value = *(uint32_t*)ea;
                context->gpr[rD] = value;

                if (op == OP_LWZU) context->gpr[rA] = ea;

                return;
            }

            case OP_LBZ:
            case OP_LBZU:
            {
                uint32_t value = *(uint8_t*)ea;
                context->gpr[rD] = value;

                if (op == OP_LBZU) context->gpr[rA] = ea;

                return;
            }

            case OP_LHZ:
            case OP_LHZU:
            {
                uint32_t value = *(uint16_t*)ea;
                context->gpr[rD] = value;

                if (op == OP_LHZU) context->gpr[rA] = ea;

                return;
            }

            case OP_STW:
            case OP_STWU:
            {
                uint32_t value = context->gpr[rD];
                *(uint32_t*)ea = value;

                if (op == OP_STWU) context->gpr[rA] = ea;

                return;
            }

            case OP_STB:
            case OP_STBU:
            {
                uint32_t value = context->gpr[rD];
                *(uint8_t*)ea = value;

                if (op == OP_STBU) context->gpr[rA] = ea;

                return;
            }

            case OP_STH:
            case OP_STHU:
            {
                uint32_t value = context->gpr[rD];
                *(uint16_t*)ea = value;

                if (op == OP_STHU) context->gpr[rA] = ea;

                return;
            }

            default:
            {
                std::string msg = std::format("Instruction: {:08X}", instruction);
                OSFatal(msg.c_str());
                return;
            }
        }
    }

    BOOL BreakpointManager::DSIHandler(OSContext* context)
    {
        if ((context->dsisr & (1 << 22)) == 0)
        {
            OSFatal("DSI");
            return FALSE;
        }

        if (!context) return FALSE;
        
        // 非ブロッキングで push する
        uint32_t t = dsiTail.load(std::memory_order_relaxed);
        uint32_t next = (t + 1) & DAR_MASK;
        uint32_t h = dsiHead.load(std::memory_order_acquire);
        
        if (next == h)
        {
            // バッファ満杯 -> ドロップ（安全策）。上書きしたければ darHead を進める実装に変える。
            // 何もせずに戻す（必要なら統計カウンタを増やすくらい）
        } else {
            dsiBuf[t] =
            {
                context->dar,
                context->srr0
            };
            dsiTail.store(next, std::memory_order_release);
        }
    
        SetDABR(0);
        ExcecuteInstruction(context, *(uint32_t*)context->srr0);
        SetDABR(dabr.load());

        context->srr0 += 4;
        return TRUE;
    }

    void BreakpointManager::SwitchThreadHandler(OSThread* thread, OSThreadQueue*)
    {
        if (!thread) return;

        uint32_t desired = dabr.load(std::memory_order_relaxed);
        uint32_t addr = reinterpret_cast<uint32_t>(thread);

        if (lastThreadAddr.load() == addr && lastDabrSet.load() == desired) return;

        lastThreadAddr.store(addr);
        lastDabrSet.store(desired);

        SetDSICallback(DSIHandler);
        SetDABR(desired);
    }
}