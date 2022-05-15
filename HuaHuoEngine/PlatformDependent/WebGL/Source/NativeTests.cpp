#include "UnityPrefix.h"

#if ENABLE_UNIT_TESTS

#include "Runtime/Network/PlayerCommunicator/PlayerConnection.h"
#include <emscripten.h>
#include "Runtime/Logging/LogSystem.h"

static void TestReportLoop(void)
{
    PlayerConnection::Get().Poll();
}

void WebGLReportResults()
{
    InstallPlayerConnectionLogging(true);
    emscripten_set_main_loop(TestReportLoop, 0, 1);
}

#endif
