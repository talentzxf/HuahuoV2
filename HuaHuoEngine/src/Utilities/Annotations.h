#pragma once

#if defined(_MSC_VER)
    #define UNITY_NOINLINE              __declspec(noinline)
    #define UNITY_INLINE                inline
    #define UNITY_FORCEINLINE           __forceinline
    #define UNITY_EMPTYINLINE           __forceinline
#else
    #define UNITY_NOINLINE              __attribute__((unused, noinline)) // unused is needed to avoid warning when a function is not used
    #define UNITY_INLINE                __attribute__((unused)) inline

    #if defined(__clang__) || defined(__APPLE_CC__) || defined(__APPLE_CPP__)
    #define UNITY_FORCEINLINE           __attribute__((unused, always_inline, nodebug)) inline
    #else
    #define UNITY_FORCEINLINE           __attribute__((unused, always_inline)) inline
    #endif

    #if defined(__clang__) || defined(__APPLE_CC__) || defined(__APPLE_CPP__)
    #define UNITY_EMPTYINLINE       __attribute__((const, always_inline, nodebug)) inline
    #else
    #define UNITY_EMPTYINLINE       __attribute__((const, always_inline)) inline
    #endif
#endif // defined(_MSC_VER)

#if PLATFORM_WIN
#pragma warning(disable:6255) // _alloca
#pragma warning(disable:6211) // leaking due to exception
#include <cassert>
#define OUTPUT_OPTIONAL _Out_opt_
#define DOES_NOT_RETURN __declspec(noreturn)
#define ANALYSIS_ASSUME(x) { __analysis_assume(x); }
#define TAKES_PRINTF_ARGS(n, m)
#define UNUSED_SYMBOL

#elif defined(__GNUC__) || defined(__clang__)

#define OUTPUT_OPTIONAL
#define DOES_NOT_RETURN __attribute__((noreturn))
#define ANALYSIS_ASSUME(x)
#define TAKES_PRINTF_ARGS(m, n) __attribute__((format(printf,m,n)))
#define UNUSED_SYMBOL __attribute__((unused))
#else

#define OUTPUT_OPTIONAL
#define DOES_NOT_RETURN
#define ANALYSIS_ASSUME(x)
#define TAKES_PRINTF_ARGS(n, m)
#define UNUSED_SYMBOL

#endif

// Macros for marking things deprecated. Examples:
//
// Deprecating a function or variable:
//      DEPRECATED("use the string variant instead") void DoStringyThing(core::string str);
//
// It's only needed on the declaration in the header, and it goes BEFORE the declaration.
//
// Deprecating a class or struct:
//      struct DEPRECATED("use math::float3 instead") Vector3f
//      {
//          float x, y, z;
//      };
//
// Again, it's only needed on the declaration, but it goes AFTER the 'class' or 'struct' keyword.
// (Otherwise the compiler confuses it with deprecating a variable).
//
// Deprecating an enum value:
//      enum Platforms
//      {
//          Windows,
//          WebPlayer DEPRECATED_ENUM_VALUE("no longer supported"),
//          Metro DEPRECATED_ENUM_VALUE("use WindowsStore instead") = 4,
//          WindowsStore = 4
//      }
//
// It is a different macro name, and it goes AFTER the enum member, but BEFORE any assigned value for that member.
//
// There is no non-message version of these things, by design - always write something that explains
// what to use instead, if applicable.
//
#if defined(_MSC_VER)
    #define DEPRECATED(msg) __declspec(deprecated(msg))
    #define DEPRECATED_ENUM_VALUE(msg) /* no equivalent for this in MSVC */
#elif defined(__clang__)
    #if __has_extension(attribute_deprecated_with_message)
        #define DEPRECATED(msg) __attribute__((deprecated(msg)))
    #else
        #define DEPRECATED(msg) __attribute__((deprecated))
    #endif

    #if __has_extension(enumerator_attributes)
        #if __has_extension(attribute_deprecated_with_message)
            #define DEPRECATED_ENUM_VALUE(msg) __attribute__((deprecated(msg)))
        #else
            #define DEPRECATED_ENUM_VALUE(msg) __attribute__((deprecated))
        #endif
    #else
        #define DEPRECATED_ENUM_VALUE(msg)
    #endif
#elif defined(__GNUC__)
// Support for messages on the deprecated attribute arrived in GCC 4.5
    #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 5
        #define DEPRECATED(msg) __attribute__((deprecated(msg)))
    #else
        #define DEPRECATED(msg) __attribute__((deprecated))
    #endif
// Support for attributes on enumerators is GCC 6
    #if __GNUC__ >= 6
        #define DEPRECATED_ENUM_VALUE(msg) __attribute__((deprecated(msg)))
    #else
        #define DEPRECATED_ENUM_VALUE(msg)
    #endif
#else
// TODO
    #define DEPRECATED(msg)
#endif
