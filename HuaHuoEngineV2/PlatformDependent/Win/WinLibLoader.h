#pragma once

#include <windef.h>
#include <libloaderapi.h>
#include "Runtime/Utilities/StaticAssert.h"
#include "Runtime/Logging/LogAssert.h"

namespace winlibloader
{
namespace impl
{
    // Managing functions within std::tuple<Fns...>
    // This works by using variadic template argument expansion and compile-time recursion to generate function calls
    template<size_t N>
    struct Functions
    {
        template<typename ... FnTypes>
        static void Load(HMODULE lib, std::tuple<FnTypes...>& fns, const std::array<const char*, sizeof...(FnTypes)>& fnNames)
        {
            // std::tuple_element_t extracts the element type at a compile-time index
            using function_type = std::tuple_element_t<N - 1, std::tuple<FnTypes...> >;

            std::get<N - 1>(fns) = reinterpret_cast<function_type>(GetProcAddress(lib, fnNames[N - 1]));
            Functions<N - 1>::Load(lib, fns, fnNames);
        }
    };

    // This struct halts compile-time recursion using struct selection
    template<>
    struct Functions<0>
    {
        template<typename ... FnTypes>
        static void Load(HMODULE lib, std::tuple<FnTypes...>& fns, const std::array<const char*, sizeof...(FnTypes)>& fnNames) {}
    };
    // ----------------------------------------------------

    // Automatically select LoadLibrary version (LoadLibraryA or LoadLibraryW) based on character type
    HMODULE LoadDll(const char* name);
    HMODULE LoadDll(const wchar_t* name);
    // ----------------------------------------------------
}

    // Core library functionality
    template<typename Dll, typename ... FnTypes>
    class Lib
    {
        CompileTimeAssert(sizeof...(FnTypes) > 0,
            "The number of `FnTypes` for the `typename ... FnTypes` template parameter must be more than zero for `Lib<typename Dll, typename ... FnTypes>`");
        CompileTimeAssert(sizeof...(FnTypes) == Dll::FnNames.size(),
            "The number of `FnNames` defined within `typename Dll::FnNames` does not match the number of types defined as the `typename ... FnTypes` template parameter for `Lib<typename Dll, typename ... FnTypes>`");
    public:
        Lib(const Lib&) = delete;
        Lib(Lib&&) = delete;
        Lib& operator=(Lib&&) = delete;
        Lib& operator=(const Lib&) = delete;

        Lib()
            : m_Lib(nullptr), m_Functions({})
        {
        }

        ~Lib()
        {
            Free();
        }

        bool Load()
        {
            if (!IsLoaded())
            {
                m_Lib = impl::LoadDll(Dll::DllName);
                if (IsLoaded())
                {
                    // Expands to loading all of the functions in Dll::FnNames
                    impl::Functions<sizeof...(FnTypes)>::Load(m_Lib, m_Functions, Dll::FnNames);
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        void Free()
        {
            if (IsLoaded())
            {
                FreeLibrary(m_Lib);
                m_Lib = nullptr;
                m_Functions = {};
            }
        }

        bool IsLoaded() const noexcept
        {
            return (m_Lib != nullptr);
        }

        template<typename Dll::Fn fn>
        bool IsFunctionLoaded() const noexcept
        {
            return std::get<static_cast<size_t>(fn)>(m_Functions) != nullptr;
        }

        template<typename Dll::Fn fn, class ... Args>
        decltype(auto)Invoke(Args && ... args) const
        {
            Assert(std::get<static_cast<size_t>(fn)>(m_Functions) != nullptr);
            return std::get<static_cast<size_t>(fn)>(m_Functions)(std::forward<Args>(args)...);
        }

    private:
        HMODULE m_Lib;

        std::tuple<FnTypes...> m_Functions;
    };
    // ----------------------------------------------------
}
