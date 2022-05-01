#pragma once
#include <string>
#include "Annotations.h"
#include "TypeUtilities.h"

namespace core
{
    template<class T>
    class basic_string_ref;

    template<class T, class TStorage>
    class basic_string;

    template<int kMemLabelId, typename TChar>
    class string_with_label;
}

template<class T>
struct string_traits
{
    enum { is_null_terminated = false, is_supported = false };
};

template<typename TChar, typename TTraits, typename TAlloc>
struct string_traits<std::basic_string<TChar, TTraits, TAlloc> >
{
    enum { is_null_terminated = true, is_supported = true };

    typedef TChar value_type;
    typedef std::basic_string<TChar, TTraits, TAlloc> type;

    static const value_type*        get_data(const type& a) { return a.c_str(); }
    static typename type::size_type get_size(const type& a) { return a.size(); }
};

template<int kMemLabelId>
struct string_traits<core::string_with_label<kMemLabelId, char> >
{
    enum { is_null_terminated = true, is_supported = true };

    typedef char value_type;
    typedef core::string_with_label<kMemLabelId, char> type;

    static const value_type*        get_data(const type& a) { return a.c_str(); }
    static typename type::size_type get_size(const type& a) { return a.size(); }
};

template<int kMemLabelId>
struct string_traits<core::string_with_label<kMemLabelId, wchar_t> >
{
    enum { is_null_terminated = true, is_supported = true };

    typedef wchar_t value_type;
    typedef core::string_with_label<kMemLabelId, wchar_t> type;

    static const value_type*        get_data(const type& a) { return a.c_str(); }
    static typename type::size_type get_size(const type& a) { return a.size(); }
};

template<class TChar, class TStorage>
struct string_traits<core::basic_string<TChar, TStorage> >
{
    enum { is_null_terminated = true, is_supported = true  };

    typedef TChar value_type;
    typedef core::basic_string<TChar, TStorage> type;

    static const value_type*        get_data(const type& a) { return a.c_str(); }
    static typename type::size_type get_size(const type& a) { return a.size(); }
};

template<class TChar>
struct string_traits<core::basic_string_ref<TChar> >
{
    enum { is_null_terminated = false, is_supported = true };

    typedef TChar value_type;
    typedef core::basic_string_ref<TChar> type;

    static const value_type*        get_data(const type& a) { return a.data(); }
    static typename type::size_type get_size(const type& a) { return a.size(); }
};

template<>
struct string_traits<const char*>
{
    enum { is_null_terminated = true, is_supported = true };

    typedef char value_type;

    // force-inlined to make sure the length of strings known at compile time can be deduced
    UNITY_FORCEINLINE static const value_type* get_data(const char* a) { return a; }
    UNITY_FORCEINLINE static size_t get_size(const char* a) { return std::char_traits<char>::length(a); }
    static size_t                   calculate_size(const char* a, size_t max_length)
    {
        for (size_t i = 0; i < max_length; ++i)
        {
            if (a[i] == '\0') return i;
        }
        return max_length;
    }
};

template<size_t I>
struct string_traits<char[I]>
{
    enum { is_null_terminated = false, is_supported = true };

    typedef char value_type;

    static const value_type*        get_data(const char(&a)[I]) { return a; }
    static size_t                   get_size(const char(&a)[I]) { return string_traits<const char*>::calculate_size(a, I); }
};

template<size_t I>
struct string_traits<const char[I]>
{
    enum { is_null_terminated = false, is_supported = true };

    typedef char value_type;

    static const value_type*        get_data(const char(&a)[I]) { return a; }
    static size_t                   get_size(const char(&a)[I]) { return string_traits<const char*>::calculate_size(a, I); }
};

template<>
struct string_traits<char*>
{
    enum { is_null_terminated = true, is_supported = true  };

    typedef char value_type;

    static const value_type*        get_data(const char* a) { return a; }
    static size_t                   get_size(const char* a) { return std::char_traits<char>::length(a);  }
};

template<>
struct string_traits<char>
{
    enum { is_null_terminated = false, is_supported = true  };

    typedef char value_type;

    static const value_type*        get_data(const char& a) { return &a; }
    static size_t                   get_size(char a) { return 1; }
};

template<>
struct string_traits<const wchar_t*>
{
    enum { is_null_terminated = true, is_supported = true };

    typedef wchar_t value_type;

    static const value_type*        get_data(const wchar_t* a) { return a; }
    static size_t                   get_size(const wchar_t* a) { return std::char_traits<wchar_t>::length(a); }
    static size_t                   calculate_size(const wchar_t* a, size_t max_length)
    {
        for (size_t i = 0; i < max_length; ++i)
        {
            if (a[i] == L'\0') return i;
        }
        return max_length;
    }
};

template<size_t I>
struct string_traits<wchar_t[I]>
{
    enum { is_null_terminated = false, is_supported = true };

    typedef wchar_t value_type;

    static const value_type*        get_data(const wchar_t(&a)[I]) { return a; }
    static size_t                   get_size(const wchar_t(&a)[I]) { return string_traits<const wchar_t*>::calculate_size(a, I); }
};


template<size_t I>
struct string_traits<const wchar_t[I]>
{
    enum { is_null_terminated = false, is_supported = true };

    typedef wchar_t value_type;

    static const value_type*        get_data(const wchar_t(&a)[I]) { return a; }
    static size_t                   get_size(const wchar_t(&a)[I]) { return string_traits<const wchar_t*>::calculate_size(a, I); }
};

template<>
struct string_traits<wchar_t*>
{
    enum { is_null_terminated = true, is_supported = true  };

    typedef wchar_t value_type;

    static const value_type*        get_data(const wchar_t* a) { return a; }
    static size_t                   get_size(const wchar_t* a) { return std::char_traits<wchar_t>::length(a); }
};

template<>
struct string_traits<wchar_t>
{
    enum { is_null_terminated = false, is_supported = true  };

    typedef wchar_t value_type;

    static const value_type*        get_data(const wchar_t& a) { return &a; }
    static size_t                   get_size(wchar_t a) { return 1; }
};

namespace StringTraits
{
    // AsConstTChars

    template<typename TChar, typename TStorage>
    inline const TChar* AsConstTChars(const core::basic_string<TChar, TStorage>& value)
    {
        return value.c_str();
    }

    template<typename TChar, typename TTraits, typename TAlloc>
    inline const TChar* AsConstTChars(const std::basic_string<TChar, TTraits, TAlloc>& value)
    {
        return value.c_str();
    }

    template<typename TChar>
    inline const TChar* AsConstTChars(const TChar* value)
    {
        return value;
    }

    // AsTChars

    // As long as you are not changing the logical size of the string (overwrite null)
    // and you are not making buffer overrun you can safely modify string chars inplace.
    // Also, the string implementation must guarantee contiguous storage of it's internal buffer.
    template<typename TChar, typename TStorage>
    inline TChar* AsTChars(core::basic_string<TChar, TStorage>& value)
    {
        if (value.empty())
        {
            static char s_EmptyString = 0;
            return &s_EmptyString;
        }
        else
        {
            return &*value.begin();
        }
    }

    template<typename TChar, typename TTraits, typename TAlloc>
    inline TChar* AsTChars(std::basic_string<TChar, TTraits, TAlloc>& value)
    {
        if (value.empty())
        {
            static char s_EmptyString = 0;
            return &s_EmptyString;
        }
        else
        {
            return &*value.begin();
        }
    }

    template<typename TChar>
    inline TChar* AsTChars(TChar* value)
    {
        return value;
    }

    // GetLength

    template<typename TChar>
    inline size_t GetLength(const core::basic_string_ref<TChar>& value)
    {
        return value.length();
    }

    template<typename TChar, typename TStorage>
    inline size_t GetLength(const core::basic_string<TChar, TStorage>& value)
    {
        return value.length();
    }

    template<typename TChar, typename TTraits, typename TAlloc>
    inline size_t GetLength(const std::basic_string<TChar, TTraits, TAlloc>& value)
    {
        return value.length();
    }

    template<typename TChar>
    inline size_t GetLength(const TChar* value)
    {
        // $ it's tempting to provide an overload for TChar[size_t], but it will fail on 'char x[256] = "abc";'.
        //   there's no way to differentiate a buffer (with whatever in it) from a string literal.

        return std::char_traits<TChar>::length(value);
    }

    // HasConstantTimeLength: true if a type supports query of string length in O(k)

    template<typename TString>
    struct HasConstantTimeLength : public FalseType {};

    template<typename TChar>
    struct HasConstantTimeLength<core::basic_string_ref<TChar> > : public TrueType {};

    template<typename TChar, typename TStorage>
    struct HasConstantTimeLength<core::basic_string<TChar, TStorage> > : public TrueType {};

    template<typename TChar, typename TTraits, typename TAlloc>
    struct HasConstantTimeLength<std::basic_string<TChar, TTraits, TAlloc> > : public TrueType {};
}
