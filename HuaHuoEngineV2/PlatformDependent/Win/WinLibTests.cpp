#include "UnityPrefix.h"

#if ENABLE_UNIT_TESTS

#include "Runtime/Testing/Testing.h"
#include "WinLib.h"

INTEGRATION_TEST_SUITE(WinLib)
{
    // Helper to compile time loop over library functions
    template<size_t N>
    struct ForEach
    {
        template<typename Lib, typename Enum>
        static void Loop(Lib& lib, bool value)
        {
            CHECK(lib.IsFunctionLoaded<static_cast<Enum>(N - 1)>() == value);
            ForEach<N - 1>::Loop<Lib, Enum>(lib, value);
        }
    };

    template<>
    struct ForEach<0>
    {
        template<typename Lib, typename Enum>
        static void Loop(Lib& lib, bool value) {}
    };
    // ------------------------------------------------------

    // Generic test to load / unload a library (and check function state)
    template<typename LibType, typename Dll>
    void LoadUnloadLibTest(LibType& lib)
    {
        if (lib.IsLoaded())
            lib.Free();

        CHECK(!lib.IsLoaded());

        lib.Load();
        CHECK(lib.IsLoaded());

        ForEach<Dll::FnNames.size()>::Loop<LibType, Dll::Fn>(lib, true);

        lib.Free();
        CHECK(!lib.IsLoaded());

        ForEach<Dll::FnNames.size()>::Loop<LibType, Dll::Fn>(lib, false);
    }

    // ------------------------------------------------------

    TEST(LibUser32_LoadFree)
    {
        using namespace winlib;
        LoadUnloadLibTest<LibUser32, User32Dll>(GetUser32());
    }

    TEST(LibShcore_LoadFree)
    {
        using namespace winlib;
        LoadUnloadLibTest<LibShcore, ShcoreDll>(GetShcore());
    }

    TEST(LibDbghelp_LoadFree)
    {
        using namespace winlib;
        LoadUnloadLibTest<LibDbghelp, DbghelpDll>(GetDbghelp());
    }
}

#endif //!ENABLE_UNIT_TESTS
