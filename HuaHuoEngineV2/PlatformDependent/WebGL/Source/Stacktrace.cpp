#include "UnityPrefix.h"
#include "JSBridge.h"

core::string GetStacktrace(int skipFrames)
{
    // Skip stacktrace functions.
    skipFrames += 4;

    #define kMaxWebGLStackTraceSize (8*1024)
    char buffer[kMaxWebGLStackTraceSize];
    printf_console("PlatformGetStacktrace\n");
    JS_Log_StackTrace(buffer, kMaxWebGLStackTraceSize);


    char *start = buffer;
    for (char *ch = buffer; ch < buffer + kMaxWebGLStackTraceSize && skipFrames > 0; ch++)
    {
        if (*ch == '\n')
        {
            skipFrames--;
            start = ch + 1;
        }
    }
    return start;
}

UInt32 GetStacktracetNativeOption(void**trace, int bufferSize, int startFrame, bool fastNativeOnly)
{
    trace[0] = 0;
    return 0;
}

int GetProfilerStacktrace(UInt64* frame, int maxFrameCount)
{
    return 0;
}
