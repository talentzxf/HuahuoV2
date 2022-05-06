#pragma once

// By default we build static libs
#ifndef BUILDING_DYNAMICLIB
#define BUILDING_DYNAMICLIB 0
#endif

#ifndef BUILDING_COREMODULE
#define BUILDING_COREMODULE 1
#endif

#if BUILDING_DYNAMICLIB

#if PLATFORM_WIN && PLATFORM_STANDALONE && !PLATFORM_WINRT

#if BUILDING_COREMODULE
#define EXPORT_COREMODULE __declspec(dllexport)
#else
#define EXPORT_COREMODULE __declspec(dllimport)
#endif

#define EXPORT_MODULE __declspec(dllexport)

#elif PLATFORM_OSX && PLATFORM_STANDALONE && 0

#if BUILDING_COREMODULE
#define EXPORT_COREMODULE __attribute__((visibility("default")))
#else
#define EXPORT_COREMODULE
#endif

#define EXPORT_MODULE __attribute__((visibility("default")))

#else

#define EXPORT_COREMODULE
#define EXPORT_MODULE

#endif

#else

#define EXPORT_COREMODULE
#define EXPORT_MODULE

#endif // BUILDING_DYNAMICLIB
