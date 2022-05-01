//
// Created by VincentZhang on 4/8/2022.
//

#ifndef PERSISTENTMANAGER_LOGASSERT_H
#define PERSISTENTMANAGER_LOGASSERT_H
#include <stdarg.h>
#include "Utilities/Annotations.h"
#include "baselib/include/CoreMacros.h"
#include "Memory/AllocatorLabels.h"
#include "BaseClasses/InstanceID.h"
#include <string>

// __FILE_STRIPPED__ should be used in place of __FILE__ when you only want to embed the source filename for diagnostic
// purposes, and don't want it to take up space in a final release.
// (formerly part of CoreMacros)
#if ENABLE_STRIPPING_SOURCE_FILENAMES
#define __FILE_STRIPPED__ ""
#else
#define __FILE_STRIPPED__ __FILE__
#endif

enum LogMessageFlags
{
    kNoLogMessageFlags                  = 0,
    kError                              = 1 <<  0, // Message describes an error.
    kAssert                             = 1 <<  1, // Message describes an assertion failure.
    kLog                                = 1 <<  2, // Message is a general log message.
    kFatal                              = 1 <<  4, // Message describes a fatal error, and that the program should now exit.
    kAssetImportError                   = 1 <<  6, // Message describes an error generated during asset importing.
    kAssetImportWarning                 = 1 <<  7, // Message describes a warning generated during asset importing.
    kScriptingError                     = 1 <<  8, // Message describes an error produced by script code.
    kScriptingWarning                   = 1 <<  9, // Message describes a warning produced by script code.
    kScriptingLog                       = 1 << 10, // Message describes a general log message produced by script code.
    kScriptCompileError                 = 1 << 11, // Message describes an error produced by the script compiler.
    kScriptCompileWarning               = 1 << 12, // Message describes a warning produced by the script compiler.
    kStickyLog                          = 1 << 13, // Message is 'sticky' and should not be removed when the user manually clears the console window.
    kMayIgnoreLineNumber                = 1 << 14, // The scripting runtime should skip annotating the log callstack with file and line information.
    kReportBug                          = 1 << 15, // When used with kFatal, indicates that the log system should launch the bug reporter.
    kDisplayPreviousErrorInStatusBar    = 1 << 16, // The message before this one should be displayed at the bottom of HuaHuo's main window, unless there are no messages before this one.
    kScriptingException                 = 1 << 17, // Message describes an exception produced by script code.
    kDontExtractStacktrace              = 1 << 18, // Stacktrace extraction should be skipped for this message.
    kScriptingAssertion                 = 1 << 21, // The message describes an assertion failure in script code.
    kStacktraceIsPostprocessed          = 1 << 22, // The stacktrace has already been postprocessed and does not need further processing.
    kIsCalledFromManaged                = 1 << 23, // The message is being called from managed code.
};

/// The type of the log message in the delegate registered with Application.RegisterLogCallback.
///
enum LogType
{
    /// LogType used for Errors.
    kLogTypeError = 0,
    /// LogType used for Asserts. (These indicate an error inside HuaHuo itself.)
    kLogTypeAssert = 1,
    /// LogType used for Warnings.
    kLogTypeWarning = 2,
    /// LogType used for regular log messages.
    kLogTypeLog = 3,
    /// LogType used for Exceptions.
    kLogTypeException = 4,
    /// LogType used for Debug.
    kLogTypeDebug = 5,
    ///
    kLogTypeNumLevels
};


/// The option of the log message in the delegate registered with Application.RegisterLogCallback.
/// Should be kept in sync with LogOption enum in Runtime/Export/BaseClass.cs
///
enum LogOption
{
    /// LogOption used for default behaviour
    kLogOptionNone = 0,
    /// LogOption used for supressing stacktraces from log message
    kLogOptionNoStacktrace = 1,
    ///
    kLogOptionNumLevels
};

#if HuaHuo_USE_PLATFORM_LOGASSERT
#   include "Utilities/PlatformLogAssert.h"
#endif

struct DebugStringToFileData
{
    const char* message;
    const char* scriptingExceptionType;
    const char* strippedStacktrace;
    const char* stacktrace;
    const char* file;
    int line;
    int column;
    LogMessageFlags mode;
    InstanceID targetInstanceID;
    int identifier;
    // LogEntryDoubleClickCallback doubleClickCallback;
    bool invokePostprocessCallbacks;

    DebugStringToFileData()
            : message("")
            , scriptingExceptionType("")
            , strippedStacktrace("")
            , stacktrace("")
            , file("")
            , line(0)
            , column(0)
            , mode(kNoLogMessageFlags)
            , targetInstanceID(InstanceID_None)
            , identifier(0)
            // , doubleClickCallback(NULL)
            , invokePostprocessCallbacks(true)
    {
    }
};

struct CppLogEntry;
typedef void (*LogEntryDoubleClickCallback)(const CppLogEntry&);

EXPORT_COREMODULE void DebugStringToFile(const DebugStringToFileData& data);
TAKES_PRINTF_ARGS(1, 2) std::string Format(const char* format, ...);

template<typename TString>
void DebugStringToFile(const TString& message, const char* file, int line, int column, LogMessageFlags mode, const InstanceID objectInstanceID = InstanceID_None, int identifier = 0, LogEntryDoubleClickCallback doubleClickCallback = NULL)
{
    DebugStringToFileData data;
    // data.message = StringTraits::AsConstTChars(message);
    data.file = file;
    data.line = line;
    data.column = column;
    data.mode = mode;
    data.targetInstanceID = objectInstanceID;
    data.identifier = identifier;
    // data.doubleClickCallback = doubleClickCallback;
    DebugStringToFile(data);
}

void DumpCallstackConsole(const char* prefix, const char* file, int line);
#define DUMP_CALLSTACK(message) DumpCallstackConsole(message, __FILE_STRIPPED__, __LINE__)

void DebugTextLineByLine(const char* text, int maxLineLen = -1);

LogMessageFlags LogTypeOptionsToLogMessageFlags(LogType logType, LogOption logOptions);

#define ErrorIf(x)                          PP_WRAP_CODE(if (x) DebugStringToFile (#x, __FILE_STRIPPED__, __LINE__, -1, kError); ANALYSIS_ASSUME(!(x)))

#define ErrorString(x)                          PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kError))
#define ErrorStringWithoutStacktrace(x)         PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kError))
#define ErrorStringMsg(...)                     PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kError))
#define ErrorStringMsgWithoutStacktrace(...)    PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kError))
#define WarningString(x)                        PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kScriptingWarning))
#define WarningStringWithoutStacktrace(x)       PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kScriptingWarning))
#define WarningStringMsg(...)                   PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kScriptingWarning))
#define WarningStringMsgWithoutStacktrace(...)  PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kScriptingWarning))

/// These errors pass an Object* as the place in which object the error occurred
#define ErrorStringObject(x, o)              PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kError, GetInstanceIDFrom(o)))
#define WarningStringObject(x, o)            PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kScriptingWarning, GetInstanceIDFrom(o)))

#define ErrorStringObjectWithoutStacktrace(x, o) PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kError, GetInstanceIDFrom(o)))
#define WarningStringObjectWithoutStacktrace(x, o)   PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kScriptingWarning, GetInstanceIDFrom(o)))

#define LogStringObject(x, o)               PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kLog, GetInstanceIDFrom(o)))
#define LogString(x)                        PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kLog))
#define LogStringWithoutStacktrace(x)       PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kDontExtractStacktrace | kLog))
#define LogStringMsg(...)                   PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kLog))

// #define FatalErrorString(x)                 PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kError | kFatal | kReportBug))
#define FatalErrorString(x) PP_WRAP_CODE(printf("FatalError:%s", (x).c_str()))
#define FatalErrorIf(x)                     PP_WRAP_CODE(if (x) DebugStringToFile (#x, __FILE_STRIPPED__, __LINE__, -1, kError | kFatal | kReportBug))
#define FatalErrorStringDontReport(x)       PP_WRAP_CODE(DebugStringToFile (x, __FILE_STRIPPED__, __LINE__, -1, kError | kFatal))
#define FatalErrorMsg(...)                  PP_WRAP_CODE(DebugStringToFile (Format(__VA_ARGS__), __FILE_STRIPPED__, __LINE__, -1, kError | kFatal))

/***** MAKE SURE THAT Assert's only compare code to compare,    ****/
/***** Which can safely be not called in non-debug mode         ****/


#ifndef ENABLE_ASSERTIONS
#define ENABLE_ASSERTIONS DEBUGMODE
#endif

#ifndef ENABLE_DEBUG_ASSERTIONS
#define ENABLE_DEBUG_ASSERTIONS DEBUGMODE && !HuaHuo_RELEASE
#endif

#ifndef ASSERT_SHOULD_BREAK
#define ASSERT_SHOULD_BREAK DEBUGMODE && !HuaHuo_RELEASE
#endif

#if DEBUGMODE
#define DEBUGMODE_ONLY(code) code
    #define NON_DEBUGMODE_ONLY(code)
#else
#define DEBUGMODE_ONLY(code)
#define NON_DEBUGMODE_ONLY(code) code
#endif

#if HuaHuo_RELEASE
#define HuaHuo_RELEASE_ONLY(code) code
    #define NON_HuaHuo_RELEASE_ONLY(code)
#else
#define HuaHuo_RELEASE_ONLY(code)
#define NON_HuaHuo_RELEASE_ONLY(code) code
#endif


#define HuaHuo__DEBUGLOG(msg, ...)   DEBUGMODE_ONLY(PP_WRAP_CODE(DebugStringToFile(msg,  __FILE_STRIPPED__, __LINE__, -1, __VA_ARGS__)))

#if ENABLE_NATIVE_TEST_FRAMEWORK
extern bool IsNativeTestExpectingAssertionFailure(const char* msg);
#else
#define IsNativeTestExpectingAssertionFailure(...) false
#endif

#if ENABLE_ASSERTIONS
bool AssertImplementation(InstanceID objID, const char* fileStripped, int line, int column, const char* msg);

#   define HuaHuo__ASSERT_IMPL(test, objID, msg) PP_WRAP_CODE(                                       \
    if (!(test))                                                                                    \
    {                                                                                               \
        if (AssertImplementation(objID, __FILE_STRIPPED__, __LINE__, -1, StringTraits::AsConstTChars(msg)))                              \
        {                                                                                           \
            NON_HuaHuo_RELEASE_ONLY(DEBUG_BREAK);                                                    \
        }                                                                                           \
        ANALYSIS_ASSUME(test);                                                                      \
    })
#else
#   define HuaHuo__ASSERT_IMPL(test, objID, msg) UNUSED(test)
#endif

// The macros taking ... params here all expect either nothing, or a single argument which is a context object for the assertion
// The exception is AssertFormatMsg which does not support taking a context object - its ... are the values to put into the format string
// If you want to use a format string AND have a context object, you need to use AssertMsg and call Format() yourself.
#define Assert(test)                    HuaHuo__ASSERT_IMPL(test, InstanceID_None, "Assertion failed on expression: '" #test "'")
#define AssertMsg(test, msg, ...)       HuaHuo__ASSERT_IMPL(test, GetInstanceIDFrom(__VA_ARGS__), msg)
#define AssertFormatMsg(test, msg, ...) HuaHuo__ASSERT_IMPL(test, InstanceID_None, Format(msg, __VA_ARGS__))
#define AssertString(msg, ...)          HuaHuo__ASSERT_IMPL(false, GetInstanceIDFrom(__VA_ARGS__), msg) // TODO: Rename to AssertFailure

#define ScriptWarning(msg, ...)         HuaHuo__DEBUGLOG(msg, kScriptingWarning, GetInstanceIDFrom(__VA_ARGS__))

#if ENABLE_ASSERTIONS
#   define AssertFiniteParameter(param, functionName, obj)    PP_WRAP_CODE(AssertMsg(IsFinite(param), Format("Invalid parameter %s in %s because it was infinity or NaN", #param, functionName), obj))
// Assert on recursive call.
// For ex., if you expect function not to be called recursively at all, pass 1.
//          if you expect function to be called recursively once, but not more - pass 2.
#   define AssertOnRecursiveCall(RecursiveCallLimit, ...) PP_WRAP_CODE(static unsigned int s_RecursiveCallCount = 0; RecursionLimiter limiter(&s_RecursiveCallCount); AssertMsg (s_RecursiveCallCount <= RecursiveCallLimit, __VA_ARGS__))
#else
#   define AssertFiniteParameter(param, functionName, obj)    PP_WRAP_CODE(UNUSED(param);UNUSED(functionName);UNUSED(obj))
#   define AssertOnRecursiveCall(RecursiveCallLimit, ...)    PP_EMPTY_STATEMENT
#endif


// There's a known conflict between Apple/HuaHuo DebugAssert(), Apple's variant is deprecated but
// it gets declared in CoreServices.framework -> bundled CarbonCore.framework/Debugging.h header
// where it's guarded by __DEBUGGING__, just avoid getting it declared entirely since it shouldn't
// get used by any code anymore
#if defined(__APPLE__)
#define __DEBUGGING__
#endif

#if ENABLE_DEBUG_ASSERTIONS
#define DebugAssert(x)                       Assert(x)
    #define DebugAssertMsg(x, msg)               AssertMsg(x, msg)
    #define DebugAssertFormatMsg(x, msg, ...)    AssertFormatMsg(x, msg, __VA_ARGS__)
#else
#define DebugAssert(test)                    UNUSED(test)
#define DebugAssertMsg(test, msg)            UNUSED(test)
#define DebugAssertFormatMsg(test, msg, ...) UNUSED(test)
#endif

// The VERIFY() macro is similar to Assert(), but always evaluates its expression, even when assertions are disabled. Use it when
// your expression has side-effects that you want to make sure are still evaluated in release builds.
// The macro takes an optional message and format string parameters, e.g.
//    VERIFY(Something(), "Call to something failed on object ID %i", objectID);

#if ENABLE_ASSERTIONS
#define VERIFY(EXPR_, ...) PP_EVAL(PP_DEFER2(PP_IF_ELSE)(PP_VARG_IS_NONEMPTY(__VA_ARGS__))(AssertMsg(EXPR_, Format(__VA_ARGS__)))(Assert(EXPR_)))
#else
#define VERIFY(EXPR_, ...) PP_WRAP_CODE(EXPR_)
#endif

void printf_consolev(LogType logType, const char* log, va_list list, bool flushOnWrite = true);

#if MASTER_BUILD
#define HuaHuo_TRACE(...)
#else
#define HuaHuo_TRACE(...)                printf_console(__VA_ARGS__)
#endif

#if HuaHuo_EDITOR
/// When logging an error it can be passed an identifier. The identifier can be used to remove errors from the console later on.
/// Eg. when reimporting an asset all errors generated for that asset should be removed before importing!
void RemoveErrorWithIdentifierFromConsole(int identifier);
void EnableExtendedLogging();
#endif

#endif //PERSISTENTMANAGER_LOGASSERT_H
