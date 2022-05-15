#pragma once

#include "Runtime/Core/Containers/String.h"
#include "Runtime/Utilities/StringTraits.h"
#include "WinUnicode.h"


#include <algorithm>

#if !UNITY_EXTERNAL_TOOL

// These functions are only used in Editor/Standalone, so they do not need to be
// built for crashhandler apps.

#if UNITY_EDITOR || PLATFORM_STANDALONE
bool GetWideShortPathName(const core::wstring& wideUnityPath, core::wstring& wideShortPath);

core::string ToShortPathName(core::string_ref unityPath);
#endif

#if UNITY_EDITOR
void ConvertUnityPathNameToShort(const core::string& utf8, core::string& outBuffer);
#endif

#endif // !UNITY_EXTERNAL_TOOL

template<typename TString>
inline void ConvertSeparatorsToWindows(TString& path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
}

// Implementation details for ConvertWindowsPathName
namespace detail
{
    template<typename TDestString>
    inline void ConvertWindowsPathNameImpl(const wchar_t* widePath, TDestString& outPath)
    {
        ConvertWideToUTF8String(widePath, outPath);
        std::replace(outPath.begin(), outPath.end(), '\\', '/'); // Convert separators to unity
    }
} // namespace detail

template<typename TSourceString, typename TDestString>
inline void ConvertWindowsPathName(const TSourceString& widePath, TDestString& outPath)
{
    return detail::ConvertWindowsPathNameImpl(StringTraits::AsConstTChars(widePath), outPath);
}

// This overload is a minor optimization, i.e., we don't copy wide buffer to a std::wstring,
// and simply go straight to conversion routines
template<typename TDestString>
inline void ConvertWindowsPathName(const wchar_t* widePath, TDestString& outPath)
{
    return detail::ConvertWindowsPathNameImpl(widePath, outPath);
}

// Implementation details for ConvertUnityPathName
namespace detail
{
    template<typename TSourceString, typename TDestString>
    inline void ConvertUnityPathNameImpl(const TSourceString& utf8, TDestString& widePath)
    {
        ConvertUTF8ToWideString(utf8, widePath);
        std::replace(widePath.begin(), widePath.end(), L'/', L'\\'); // Convert separators to Windows
    }

    template<typename TDestString>
    inline void ConvertUnityPathNameImpl(const char* utf8, TDestString& widePath)
    {
        ConvertUTF8ToWideString(utf8, widePath);
        std::replace(widePath.begin(), widePath.end(), L'/', L'\\'); // Convert separators to Windows
    }
} // namespace detail

template<typename TSourceString, typename TDestString>
inline void ConvertUnityPathName(const TSourceString& utf8, TDestString& widePath)
{
    return detail::ConvertUnityPathNameImpl(utf8, widePath);
}

// This overload is a minor optimization, i.e., we don't copy utf8 buffer to a std::string,
// and simply go straight to conversion routines
template<typename TDestString>
inline void ConvertUnityPathName(const char* utf8, TDestString& widePath)
{
    return detail::ConvertUnityPathNameImpl(utf8, widePath);
}

// Old and unsafe API
const int kDefaultPathBufferSize = MAX_PATH * 4;
void ConvertUnityPathName(const char* utf8, wchar_t* outBuffer, int outBufferSize);
void ConvertWindowsPathName(const wchar_t* widePath, char* outBuffer, int outBufferSize);
