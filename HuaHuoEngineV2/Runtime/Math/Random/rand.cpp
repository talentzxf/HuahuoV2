#include "Math/Random/rand.h"
#include "Utilities/HashStringFunctions.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Memory/MemoryMacros.h"
#include "Utilities/Hash128.h"
#include "HuaHuoEngineConfig.h"
#include <chrono>

#if PLATFORM_WIN
    #include "windows.h"
    #define PLAT_GETPID() GetCurrentProcessId()
#elif UNITY_POSIX
    #include <unistd.h>
    #define PLAT_GETPID() getpid()
#else
    #define PLAT_GETPID() (0)
#endif

Rand* gScriptingRand = nullptr;

static void InitScriptingRand(void*)
{
    gScriptingRand = HUAHUO_NEW(Rand, kMemUtility)(Rand::GetUniqueGenerator());
}

static void CleanupScriptingRand(void*)
{
    HUAHUO_DELETE(gScriptingRand, kMemUtility);
}

static RegisterRuntimeInitializeAndCleanup gScriptingRandInitAndCleanup(InitScriptingRand, CleanupScriptingRand);

Rand& GetScriptingRand()
{
    return *gScriptingRand;
}

void Rand::RandomizeState()
{
    // Most platforms have a way to get "real" random numbers

//    if (GetSystemEntropy((unsigned char*)&m_State, sizeof(m_State)))
//        return;

    // If this one doesn't, attempt to generate a unique set of terms for every call

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

    static volatile int salt = 1;
    UInt64 terms[] =
    {
        // Start with the absolute time
        static_cast<UInt64>(value.count()),

        // Also sample the high resolution timer, because the clock might have low precision
        static_cast<UInt64>(duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()),

        // Avoid collisions between different builds
        ComputeStringHash128(HHE_VERSION_STR).PackToUInt64(),

//        // Avoid collisions between threads in this process
//        (UInt64)AtomicIncrement(&salt),
        (UInt64)salt,

        // Avoid collisions between processes on this device
        (UInt64)PLAT_GETPID(),

//    #if !UNITY_EXTERNAL_TOOL
//        // Avoid collisions between devices
//        ComputeStringHash128(systeminfo::GetDeviceUniqueIdentifier()).PackToUInt64(),
//    #endif
    };

    // Hash all the terms to initialize our internal state

    Hash128 hash = ComputeHash128(terms, sizeof(terms));

    static_assert(sizeof(m_State) == sizeof(hash), "RandState expected to be 128 bits");
    memcpy(&m_State, &hash, sizeof(m_State));
}
