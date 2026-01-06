#include "Patch.hpp"
#include "Callback.hpp"
#include "Impl.hpp"

namespace Exception
{
    void Patch::apply()
    {
        Callback::set<SystemReset>();
        Callback::set<MachineCheck>();
        Callback::set<DSI>();
        Callback::set<ISI>();
        Callback::set<ExternalInterrupt>();
        Callback::set<Alignment>();
        Callback::set<Program>();
        Callback::set<FloatingPoint>();
        Callback::set<Decrementer>();
        Callback::set<SystemCall>();
        Callback::set<Trace>();
        Callback::set<PerformanceMonitor>();
        Callback::set<Breakpoint>();
        Callback::set<SystemInterrupt>();
        Callback::set<ICI>();
    }
}