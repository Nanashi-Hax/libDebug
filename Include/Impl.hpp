#pragma once

#include <deque>

#include <coreinit/context.h>
#include <coreinit/exception.h>

#include "Base.hpp"

namespace Exception
{
    class SystemReset : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class MachineCheck : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class DSI : public Base
    {
    private:
        std::deque<OSContext> queue;

    public:
        std::string name() override;
        static OSExceptionType type();

        void panic(std::string name, OSContext* context) override;
        bool handle(OSContext* context) override;
    };

    class ISI : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();

        void panic(std::string name, OSContext* context) override;
    };

    class ExternalInterrupt : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class Alignment : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();

        void panic(std::string name, OSContext* context) override;
    };

    class Program : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class FloatingPoint : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class Decrementer : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class SystemCall : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class Trace : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class PerformanceMonitor : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class Breakpoint : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class SystemInterrupt : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };

    class ICI : public Base
    {
    public:
        std::string name() override;
        static OSExceptionType type();
    };
}
