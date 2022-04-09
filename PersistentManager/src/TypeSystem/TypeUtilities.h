//
// Created by VincentZhang on 4/6/2022.
//

#ifndef PERSISTENTMANAGER_TYPEUTILITIES_H
#define PERSISTENTMANAGER_TYPEUTILITIES_H
//////////////////////////////////////////////////////////////////////////
/// returns one value or another depending on if the given types are the same or not
template<class T, class U, int equal_value, int unequal_value>
struct SelectOnTypeEquality { enum { result = unequal_value }; };

template<class T, int equal_value, int unequal_value>
struct SelectOnTypeEquality<T, T, equal_value, unequal_value> { enum { result = equal_value }; };

// Types to represent predicate decisions during templating.
// Have different sizeof() so that can be used to make compile-time decisions as well.
struct TrueType
{
    bool b[4];
    static const bool value = true;
};
struct FalseType
{
    bool b[8];
    static const bool value = false;
};

// Use these to make compile-time time decisions about expressions. Use DeconstructFunctionType::ReturnType
// to extract the return value for further templating.
template<typename T>
FalseType IsTrueType(const T&);
TrueType IsTrueType(const TrueType&);
template<typename T>
FalseType IsFalseType(const T&);
TrueType IsFalseType(const FalseType&);

template<typename T1, typename T2>
struct IsSameType
{
    typedef FalseType ResultType;
    static const bool result = false;
};

template<typename T>
struct IsSameType<T, T>
{
    typedef TrueType ResultType;
    static const bool result = true;
};

template<typename T>
struct ToPointerType
{
    typedef T* ResultType;
};
template<typename T>
struct ToPointerType<T*>
{
    typedef T* ResultType;
};
template<typename T>
struct ToPointerType<const T*>
{
    typedef const T* ResultType;
};

//////////////////////////////////////////////////////////////////////////
/// std::conditional equivalent
template<bool B, class T, class F>
struct Conditional { typedef T type; };

template<class T, class F>
struct Conditional<false, T, F> { typedef F type; };

//////////////////////////////////////////////////////////////////////////
/// std::enable_if equivalent
template<bool B, class T = void>
struct EnableIf {};

template<class T>
struct EnableIf<true, T> { typedef T type; };

#endif //PERSISTENTMANAGER_TYPEUTILITIES_H
