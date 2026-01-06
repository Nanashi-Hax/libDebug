#include <string>
#include <format>

#include <coreinit/context.h>
#include <coreinit/debug.h>

#include "Exception/Base.hpp"

namespace Exception
{
    void Base::panic(std::string name, OSContext* context)
    {
        std::string message = std::format("{0} Exception occurred", name);
        OSFatal(message.c_str());
    }

    bool Base::handle(OSContext* context)
    {
        panic(name(), context);
        return false;
    }
}