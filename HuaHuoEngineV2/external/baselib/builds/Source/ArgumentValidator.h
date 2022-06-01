#pragma once

// Argument validator based on error state builder.
//
// For strong type arguments, implement IsValid templated function, return true.
// Because some arguments types are weakly typed, we use helper wrappers to make them strong typed.
//
// Usage examples:
// errorState |= Validate(strongTypedArgument);
// errorState |= Validate(AsPointer(weaklyTypedPointer));
// errorState |= RaiseInvalidArgument(argumentName);

#include "ErrorStateBuilder.h"

#ifndef BASELIB_ENABLE_INVALID_ARGUMENT_NAME
    #ifdef NDEBUG
        #define BASELIB_ENABLE_INVALID_ARGUMENT_NAME 0
    #else
        #define BASELIB_ENABLE_INVALID_ARGUMENT_NAME 1
    #endif
#endif

// Return true if argument is valid
template<typename T>
static inline bool IsValid(const T& value);

BASELIB_CPP_INTERFACE
{
    extern const char* Baselib_StrippedArgumentName;
}

namespace detail
{
    template<typename T>
    static inline ErrorStateBuilder Validate(const T& value, const char* argumentName, Baselib_SourceLocation sourceLocation)
    {
        if (!IsValid(value))
            return ErrorStateBuilder(Baselib_ErrorCode_InvalidArgument, sourceLocation) | WithStaticString(argumentName);
        return ErrorStateBuilder();
    }
}

// small helper to strong type a pointer
struct AsPointer
{
    const void* ptr;
    AsPointer(const void* setPtr)
        : ptr(setPtr)
    {
    }
};

static inline bool IsValid(const AsPointer& value)
{
    return value.ptr != nullptr;
}

#if BASELIB_ENABLE_INVALID_ARGUMENT_NAME
    #define Validate(argument) ::detail::Validate((argument), PP_STRINGIZE(argument), BASELIB_SOURCELOCATION)
    #define RaiseInvalidArgument(argument) RaiseError(Baselib_ErrorCode_InvalidArgument) | WithStaticString(PP_STRINGIZE(argument))
#else
    #define Validate(argument) ::detail::Validate((argument), Baselib_StrippedArgumentName, BASELIB_SOURCELOCATION)
    #define RaiseInvalidArgument(argument) RaiseError(Baselib_ErrorCode_InvalidArgument) | WithStaticString(Baselib_StrippedArgumentName)
#endif
