#pragma once
#include "HashFunctions.h"
#include <string>

namespace core
{
    template<class T>
    struct hash;

    // Based on http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
    inline UInt32 hash_combine(UInt32 a, UInt32 b)
    {
        a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
        return a;
    }

namespace detail
{
    template<class T, bool isSize64 = (sizeof(T) == sizeof(UInt64))>
    struct hash_integral;

    template<class T>
    struct hash_integral<T, false>
    {
        UInt32 operator()(T i) const
        {
            return static_cast<UInt32>(ComputeIntHash(static_cast<UInt32>(i)));
        }
    };

    template<class T>
    struct hash_integral<T, true>
    {
        UInt32 operator()(T i) const
        {
            return static_cast<UInt32>(ComputeIntHash(static_cast<UInt32>(i)) ^ ComputeIntHash(static_cast<UInt32>(i >> 32)));
        }
    };
}

    template<> struct hash<char> : detail::hash_integral<char> {};
    template<> struct hash<signed char> : detail::hash_integral<signed char> {};
    template<> struct hash<unsigned char> : detail::hash_integral<unsigned char> {};
    template<> struct hash<short> : detail::hash_integral<short> {};
    template<> struct hash<unsigned short> : detail::hash_integral<unsigned short> {};
    template<> struct hash<int> : detail::hash_integral<int> {};
    template<> struct hash<unsigned int> : detail::hash_integral<unsigned int> {};
    template<> struct hash<long> : detail::hash_integral<long> {};
    template<> struct hash<unsigned long> : detail::hash_integral<unsigned long> {};
    template<> struct hash<long long> : detail::hash_integral<long long> {};
    template<> struct hash<unsigned long long> : detail::hash_integral<unsigned long long> {};
    template<> struct hash<bool> : detail::hash_integral<bool> {};

//    template<>
//    struct hash<std::string>
//    {
//        UInt32 operator()(const std::string& s) const
//        {
//            return ComputeHash32(s.c_str(), s.size());
//        }
//    };

//    template<>
//    struct hash<std::string_ref>
//    {
//        UInt32 operator()(std::string_ref s) const
//        {
//            return ComputeHash32(s.data(), s.size());
//        }
//    };

    template<class T>
    struct hash<T*>
    {
        UInt32 operator()(T* p) const
        {
            return static_cast<UInt32>(ComputePointerHash(p));
        }
    };

//    template<typename T1, typename T2, bool supportsLabels>
//    struct hash<core::pair<T1, T2, supportsLabels> >
//    {
//        UInt32 operator()(const core::pair<T1, T2, supportsLabels>& p) const
//        {
//            return hash_combine(hash<T1>()(p.first), hash<T2>()(p.second));
//        }
//    };
}
