#pragma once

// Error state builder helper.
// Include platform specific info providers to add With%NativeError%() capabilities.
//
// Usage example:
// errorState |= RaiseError(errorCode) | WithStaticString("error details") | With...(...);

#include "Include/C/Baselib_ErrorState.h"
#include "Source/Baselib_ErrorState_Utils.h"
#include <type_traits>

namespace detail
{
    struct ErrorStateBuilder
    {
        Baselib_ErrorState errorState;

        ErrorStateBuilder()
            : errorState(Baselib_ErrorState_Create())
        {
        }

        ErrorStateBuilder(Baselib_ErrorCode errorCode, Baselib_SourceLocation sourceLocation)
            : errorState(Baselib_ErrorState_Create())
        {
            errorState.code = errorCode;
            // potentially can be improved with C++20 std::source_location
            errorState.sourceLocation = sourceLocation;
        }

        // CRTP to avoid virtual methods
        template<typename T>
        struct InfoProvider
        {
            // Fill will only be called if ErrorStateBuilder already has an error state risen.
            inline void Fill(Baselib_ErrorState& errorState) const
            {
                static_cast<T*>(this)->Fill(errorState);
            }
        };

        template<typename T>
        inline ErrorStateBuilder& WithInfoProvider(const T& provider)
        {
            static_assert(std::is_base_of<InfoProvider<T>, T>::value, "provider must derive from InfoProvider.");
            if (Baselib_ErrorState_ErrorRaised(&errorState))
                provider.Fill(errorState);
            return *this;
        }

        inline void TryMergeWith(Baselib_ErrorState* externalErrorState) const
        {
            if (!Baselib_ErrorState_ErrorRaised(externalErrorState) && Baselib_ErrorState_ErrorRaised(&errorState))
                *externalErrorState = errorState;
        }
    };
}

template<typename T>
static inline ::detail::ErrorStateBuilder operator|(::detail::ErrorStateBuilder errorStateHelper, const T& provider)
{
    return errorStateHelper.WithInfoProvider(provider);
}

// | has precedence over |= hence why errorState |= RaiseError(...) | WithStaticString(...) compiles.
static inline Baselib_ErrorState& operator|=(Baselib_ErrorState& errorState, const detail::ErrorStateBuilder& errorStateHelper)
{
    errorStateHelper.TryMergeWith(&errorState);
    return errorState;
}

static inline Baselib_ErrorState* operator|=(Baselib_ErrorState* errorState, const detail::ErrorStateBuilder& errorStateHelper)
{
    errorStateHelper.TryMergeWith(errorState);
    return errorState;
}

struct WithStaticString : ::detail::ErrorStateBuilder::InfoProvider<WithStaticString>
{
    const char* str;

    WithStaticString(const char* setStr)
        : str(setStr)
    {
    }

    inline void Fill(Baselib_ErrorState& state) const
    {
        state.extraInformationType = Baselib_ErrorState_ExtraInformationType_StaticString;
        state.extraInformation = (uint64_t)str;
    }
};

struct WithFormattedString : ::detail::ErrorStateBuilder::InfoProvider<WithFormattedString>
{
    uint64_t generationCounter;

    WithFormattedString(const char* format, ...)
        : generationCounter(0)
    {
        va_list args;
        va_start(args, format);
        generationCounter = Baselib_ErrorState_FormatAndStoreExtraInformation(format, args);
        va_end(args);
    }

    inline void Fill(Baselib_ErrorState& state) const
    {
        state.extraInformationType = Baselib_ErrorState_ExtraInformationType_GenerationCounter;
        state.extraInformation = (uint64_t)generationCounter;
    }
};

#define RaiseError(errorCode) ::detail::ErrorStateBuilder((errorCode), BASELIB_SOURCELOCATION)
