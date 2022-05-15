#pragma once

#include "Runtime/Core/Containers/String.h"
#include "Runtime/Core/Containers/StringRef.h"
#include <stringapiset.h>

// It must be noted that conversion functions are made inline for a reason.
// This reason being that std::string is highjacked in STLAllocator.h and
// it actually is core::string(kMemSTL) when we build the Editor or Standalone.
// This leads to various linking problems if the functions are implemented in
// WinUnicode.cpp, because the interface is used in both Editor/Standalone
// and CrashHandler, where std::string is "normal".

/*

// Interface functions

void ConvertUTF8ToWideString( const char* utf8, std::wstring& wide );
void ConvertUTF8ToWideString( const std::string& utf8, std::wstring& wide );
void ConvertWideToUTF8String( const wchar_t* wide, std::string& utf8 );
void ConvertWideToUTF8String( const std::wstring& wide, std::string& utf8 );

void ConvertToDefaultAnsi( const std::wstring& wide, std::string& ansi );
std::string ConvertUnityToDefaultAnsi( const std::string& utf8 );

std::string WideToUtf8(const wchar_t* wide);
std::string WideToUtf8(const std::wstring& wide);
std::wstring Utf8ToWide(const char* utf8);
std::wstring Utf8ToWide(const std::string& utf8);

*/

// Old and dangerous interface.
void UTF8ToWide(const char* utf8, wchar_t* outBuffer, int outBufferSize);

template<size_t TSize>
inline void UTF8ToWide(const char* utf8, wchar_t(&outBuffer)[TSize])
{ UTF8ToWide(utf8, outBuffer, TSize); }
template<size_t TSize>
inline void UTF8ToWide(const std::string& utf8, wchar_t(&outBuffer)[TSize])
{ UTF8ToWide(utf8.c_str(), outBuffer); }
template<size_t TSize>
inline void UTF8ToWide(const core::string& utf8, wchar_t(&outBuffer)[TSize])
{ UTF8ToWide(utf8.c_str(), outBuffer); }
template<size_t TSize>
inline void UTF8ToWide(core::string_ref utf8, wchar_t(&outBuffer)[TSize])
{ UTF8ToWide(TempString(utf8).c_str(), outBuffer); }

// Implementation details for ConvertUTF8ToWideString/ConvertWideToUTF8String
namespace utf_detail
{
    struct multi_byte_to_wide
    {
        int get_length(const char* utf8, std::size_t length) const
        {
            return ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(length), NULL, 0);
        }

        template<typename WideString>
        void convert(const char* utf8, std::size_t length, WideString& dest) const
        {
            // Convert the given narrow string to wchar_t format
            ::MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(length), &dest.front(),
                static_cast<int>(dest.size()));
        }
    };

    struct wide_to_multi_byte
    {
        explicit wide_to_multi_byte(UINT codepage)
            : m_CodePage(codepage)
        {}

        int get_length(const wchar_t* wide, std::size_t length) const
        {
            return ::WideCharToMultiByte(m_CodePage, 0, wide, static_cast<int>(length), NULL, 0, NULL, NULL);
        }

        template<typename NarrowString>
        void convert(const wchar_t* wide, std::size_t length, NarrowString& dest) const
        {
            // Convert the given wide string to char format
            ::WideCharToMultiByte(m_CodePage, 0, wide, static_cast<int>(length), &dest[0],
                static_cast<int>(dest.size()), NULL, NULL);
        }

    private:
        UINT m_CodePage;
    };

    // The reason for the length parameter is that we do not want to pass -1 to MultiByteToWideChar function.
    // If cbMultiByte parameter is -1, the function processes the entire input string, including the terminating null character.
    // Therefore, the resulting Unicode string has a terminating null character, and the length returned by the function
    // includes this character.
    // Since we output the converted string of characters to std::wstring directly,
    // we do not want to embed null characters in the string.
    template<typename SourceString, typename DestString, typename Converter>
    inline void ChangeStringFormat(const SourceString& src, std::size_t length, DestString& dest, Converter func)
    {
        if (0u == length) // source string is empty
        {
            return dest.clear();
        }

        // required size
        const int nChars = func.get_length(src, length);
        if (nChars <= 0) // destination string is empty
        {
            return dest.clear();
        }
        else
        {
            // properly resize the buffer
            dest.resize(nChars);
            return func.convert(src, length, dest);
        }
    }
} // namespace detail

template<typename DestString>
inline void ConvertUTF8ToWideString(const char* utf8, DestString& wide)
{
    if (utf8)
        utf_detail::ChangeStringFormat(utf8, strlen(utf8), wide, utf_detail::multi_byte_to_wide());
    else
        wide.clear();
}

template<typename SourceString, typename DestString>
inline void ConvertUTF8ToWideString(const SourceString& utf8, DestString& wide)
{
    return utf_detail::ChangeStringFormat(utf8.data(), utf8.size(), wide, utf_detail::multi_byte_to_wide());
}

template<typename DestString>
inline void ConvertWideToUTF8String(const wchar_t* wide, std::size_t length, DestString& utf8)
{
    if (wide)
        utf_detail::ChangeStringFormat(wide, length, utf8, utf_detail::wide_to_multi_byte(CP_UTF8));
    else
        utf8.clear();
}

inline void ConvertWideToUTF8String(const std::wstring& wide, std::string& utf8)
{
    return utf_detail::ChangeStringFormat(wide.data(), wide.size(), utf8,
        utf_detail::wide_to_multi_byte(CP_UTF8));
}

inline void ConvertWideToUTF8String(const core::wstring& wide, core::string& utf8)
{
    return utf_detail::ChangeStringFormat(wide.data(), wide.size(), utf8,
        utf_detail::wide_to_multi_byte(CP_UTF8));
}

template<typename TDestString>
inline void ConvertWideToUTF8String(const wchar_t* wide, TDestString& utf8)
{
    if (wide)
        ConvertWideToUTF8String(wide, wcslen(wide), utf8);
    else
        utf8.clear();
}

template<typename TSourceString, typename TDestString>
inline void ConvertToDefaultAnsi(const TSourceString& wide, TDestString& ansi)
{
    utf_detail::ChangeStringFormat(wide.data(), wide.size(), ansi,
        utf_detail::wide_to_multi_byte(CP_ACP));
}

inline std::string ConvertUnityToDefaultAnsi(const std::string& utf8)
{
    std::wstring wide;
    ConvertUTF8ToWideString(utf8, wide);

    std::string bufAnsi;
    ConvertToDefaultAnsi(wide, bufAnsi);
    return bufAnsi;
}

// Implementation details for WideToUtf8/Utf8ToWide
namespace utf_detail
{
    inline std::string WideToUtf8Impl(const std::wstring& wide)
    {
        std::string result;
        ConvertWideToUTF8String(wide, result);
        return result;
    }

    inline core::string WideToUtf8Impl(const core::wstring_ref& wide)
    {
        core::string result(kMemTempAlloc);
        ConvertWideToUTF8String(wide, result);
        return result;
    }

    inline std::wstring Utf8ToWideImpl(const std::string& utf8)
    {
        std::wstring result;
        ConvertUTF8ToWideString(utf8, result);
        return result;
    }

    inline core::wstring Utf8ToWideImpl(core::string_ref utf8)
    {
        core::wstring result(kMemTempAlloc);
        ConvertUTF8ToWideString(utf8, result);
        return result;
    }
} // namespace detail

inline core::string WideToUtf8(const wchar_t* wide)
{
    core::wstring wstr(kMemTempAlloc);
    if (wide != NULL)
        wstr.assign(wide);
    return utf_detail::WideToUtf8Impl(wstr);
}

inline std::string WideToUtf8(const std::wstring& wide)
{
    return utf_detail::WideToUtf8Impl(wide);
}

inline core::string WideToUtf8(const core::wstring_ref& wide)
{
    return utf_detail::WideToUtf8Impl(wide);
}

inline core::wstring Utf8ToWide(const char* utf8)
{
    return utf_detail::Utf8ToWideImpl(core::string_ref(utf8));
}

inline std::wstring Utf8ToWide(const std::string& utf8)
{
    return utf_detail::Utf8ToWideImpl(utf8);
}

inline core::wstring Utf8ToWide(const core::string& utf8)
{
    return utf_detail::Utf8ToWideImpl(core::string_ref(utf8));
}

inline core::wstring Utf8ToWide(core::string_ref utf8)
{
    return utf_detail::Utf8ToWideImpl(utf8);
}
