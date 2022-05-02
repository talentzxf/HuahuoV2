//
// Created by VincentZhang on 4/1/2022.
//

#ifndef HUAHUOENGINE_BASETYPES_H
#define HUAHUOENGINE_BASETYPES_H

//==============================================================================
//
//  Core
//
//==============================================================================
//
//  basic typedefs
//
//==============================================================================
//
//  externals:


//==============================================================================
//
//  publics:

// FIXME: Why are we redefining these here?
#if PLATFORM_LINUX
#include <stdint.h>
typedef int64_t             SInt64;
typedef uint64_t            UInt64;
#else
typedef signed long long    SInt64;
typedef unsigned long long  UInt64;
#endif

typedef signed char         SInt8;
typedef signed short        SInt16;
typedef signed int          SInt32;

typedef unsigned char       UInt8;
typedef unsigned short      UInt16;
typedef unsigned int        UInt32;


#ifdef _UNWRAP_DLL

#if defined(_WIN32) || defined(__WIN32__)
        #define UNWRAP_API extern "C" __declspec(dllexport)
        #define UNWRAP_API_STRUCT(Name) extern "C" struct __declspec(dllexport) Name
    #elif defined(__APPLE_CC__) || defined(linux) || defined(__linux__)
        #define UNWRAP_API extern "C" __attribute__((visibility("default")))
        #define UNWRAP_API_STRUCT(Name) extern "C" struct Name
    #endif

#elif !defined(_UNWRAP_LIB)

#if defined(_WIN32) || defined(__WIN32__)
#define UNWRAP_API extern "C" __declspec(dllimport)
#define UNWRAP_API_STRUCT(Name) extern "C" struct __declspec(dllimport) Name
#elif defined(__APPLE_CC__) || defined(linux) || defined(__linux__)
#define UNWRAP_API extern "C"
        #define UNWRAP_API_STRUCT(Name) extern "C" struct Name
#endif

#endif

#ifndef UNWRAP_API
#define UNWRAP_API
#endif

typedef SInt32 PersistentTypeID;
typedef UInt32 RuntimeTypeIndex;


//==============================================================================

#endif //HUAHUOENGINE_BASETYPES_H
