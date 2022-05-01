//
// Created by VincentZhang on 4/29/2022.
//

#include "LogAssert.h"
#include <cstdio>


void DebugStringToFile(const DebugStringToFileData& data)
{
//    PROFILER_AUTO(gProfilerLogString);
//    CLEAR_ALLOC_OWNER;
//
//    LogType logType = LogModeToLogType(data.mode);
//    StackTraceLogType stackTraceLogType = (data.mode & kDontExtractStacktrace) ? kStackTraceLogNone : GetStackTraceLogType(logType);
//    if (HasFlag(data.mode, kStacktraceIsPostprocessed) && stackTraceLogType != kStackTraceLogNone)
//    {
//        DebugStringToFilePostprocessedStacktrace(data);
//        return;
//    }
//
//    DebugStringToFileData processedData = data;
//
//#if PLATFORM_ANDROID && i386
//    printf_console("%s at %s:%d (%d, %d, %d)\n", processedData.message, processedData.file, processedData.line, processedData.mode, processedData.targetInstanceID, processedData.identifier);
//#endif
//
//    core::string stackTrace(kMemTempAlloc);
//    core::string strippedStackTrace(kMemTempAlloc);
//    core::string preprocessedFile(kMemTempAlloc);
//
//    switch (stackTraceLogType)
//    {
//        case kStackTraceLogNone:
//            strippedStackTrace = stackTrace = "";
//            break;
//        case kStackTraceLogScriptOnly:
//            if (gPreprocessor)
//            {
//                preprocessedFile = processedData.file;
//                core::string_ref messageStr = processedData.message;
//
//                LinePathResult result = gPreprocessor(messageStr, strippedStackTrace, stackTrace, preprocessedFile, &processedData.line, &processedData.column, processedData.mode, processedData.targetInstanceID, HasFlag(data.mode, kIsCalledFromManaged));
//
//                preprocessedFile = result.path;
//                processedData.file = preprocessedFile.c_str();
//                processedData.line = result.line;
//            }
//            break;
//        case kStackTraceLogFull:
//            // If called from managed, filename will be "Debug.bindings.h", which is useless information to the user.
//            // In the future, it would be good to get the filename properly like we do in kStackTraceLogScriptOnly above.
//            // As the Full stacktraces are different across all platforms, we do not have a unified way to get the correct filename.
//            if (HasFlag(data.mode, kIsCalledFromManaged))
//            {
//                processedData.file = "";
//                processedData.line = 0;
//            }
//#if STACKTRACE_IMPLEMENTED
//            stackTrace = strippedStackTrace = GetStacktrace();
//#else
//            strippedStackTrace = stackTrace = "Stacktrace is not supported on this platform.";
//#endif
//            break;
//        default:
//            break;
//    }
//
//    processedData.strippedStacktrace = strippedStackTrace.c_str();
//    processedData.stacktrace = stackTrace.c_str();
//    DebugStringToFilePostprocessedStacktrace(processedData);

    printf("%s", data.message);
}