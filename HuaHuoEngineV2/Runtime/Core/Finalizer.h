#pragma once
#include <algorithm>

namespace core
{
/*
    Finalizer is a helper class you can use for getting a lambda executed just before exiting a scope.

    Finalizer can be useful, if you want code always to be executed, before returning from a function
    that may have multiple returns. To avoid having to specify the callable type, you can construct
    the Finalizer with MakeFinalizer.

    Example of how to use Finalizer:
    {
        auto goodBye = core::MakeFinalizer([]()
        {
            printf_console("Goodbye World\n");
        });

        printf_console("Hello World\n");
    }
    ---
    This example will print:
    Hello World
    Goodbye World
*/

    template<class FinalCallable>
    class Finalizer
    {
    public:
        Finalizer(FinalCallable&& finalCallable) :
            m_FinalCallable(std::move(finalCallable))
        {}

        ~Finalizer()
        {
            m_FinalCallable();
        }

    private:
        FinalCallable m_FinalCallable;
    };

    template<class FinalCallable>
    Finalizer<FinalCallable> MakeFinalizer(FinalCallable&& finalCallable)
    {
        return Finalizer<FinalCallable>(std::move(finalCallable));
    }
}
