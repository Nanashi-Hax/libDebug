#pragma once

#include <string>

#include <coreinit/context.h>
#include <coreinit/debug.h>
#include <coreinit/exception.h>

namespace Exception
{
    class Base
    {
    public:
        virtual std::string name() = 0;

        virtual void panic(std::string name, OSContext* context);
        virtual bool handle(OSContext* context);
    };
}