#pragma once

namespace core
{
    template<typename T>
    class remove_const
    {
    public:
        typedef T type;
    };

    template<typename T>
    class remove_const<const T>
    {
    public:
        typedef T type;
    };
}
