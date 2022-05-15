#include "UnityPrefix.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Logging/LogSystem.h"
#include "PlatformDependent/WebGL/Source/JSBridge.h"

void WebGLPrintfConsolev(LogType logType, const char* log, va_list list)
{
    char buffer[1024 * 8] = { 0 };
    vsnprintf(buffer, 1024 * 8, log, list);
    JS_Log_Dump(buffer, logType);
}
