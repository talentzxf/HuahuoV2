// Utility functions and types for working with static types.
#pragma once

#if ENABLE_CPP_RTTI
#include <typeinfo>
#endif

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


template<typename From, typename To>
struct IsConvertible
{
private:
    static TrueType helper(To);
    static FalseType helper(...);
    static From get_from();
public:
    static bool const value = (sizeof(TrueType) == sizeof(IsConvertible::helper(get_from())));
};

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

template<typename Expected, typename Actual>
inline bool IsOfType(Actual&)
{
    return IsSameType<Actual, Expected>::result;
}

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


////////////////////////////////////////////////////////////////////////////
///// IsBaseOf - std::is_base_of equivalent
//#include "External/boost/is_base_of.inc.h"


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

//////////////////////////////////////////////////////////////////////////
/// std::is_polymorphic sort of equivalent but runtime instead of static (requires typeid() and thus RTTI).

// There's basically two approaches pre-C++11:
//  1. Derive from type, add virtual method, and then see if both have the same sizeof().
//  2. Exploit the rule that typeid() does not evaluate its expression if it's not of polymorphic type.
// The first works purely at compile time but does not work with types that don't have default constructors.
// The second relies on expression evaluation although the involved expressions are compile-time constant
// and can thus still be optimized away.
#if ENABLE_CPP_RTTI
// Clang will invariably throw an unused warning for the typeid() expression below.
#if defined(__clang__) // MSVC doesn't like unknown pragmas.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif
template<typename T>
inline bool IsPolymorphic(const T& t)
{
    bool result = false;
    typeid(result = true, t); // If 't' is not polymorphic, the entire expression isn't evaluated, so result=true won't get evaluated.
    return result;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

//////////////////////////////////////////////////////////////////////////
/// returns one value or another depending on if the given types are the same or not
template<class T, class U, int equal_value, int unequal_value>
struct SelectOnTypeEquality { enum { result = unequal_value }; };

template<class T, int equal_value, int unequal_value>
struct SelectOnTypeEquality<T, T, equal_value, unequal_value> { enum { result = equal_value }; };

//////////////////////////////////////////////////////////////////////////
/// Empty type
struct NullType {};

//////////////////////////////////////////////////////////////////////////
/// Default value generation
template<typename T>
struct DefaultValue
{
    static T Get() { return T(); }
};

template<>
struct DefaultValue<void>
{
    static void Get() {}
};

namespace core
{
    template<typename TType>
    struct remove_reference { typedef TType type; };

    template<typename TType>
    struct remove_reference<TType&> { typedef TType type; };

    template<typename TType>
    struct add_reference{ typedef typename remove_reference<TType>::type& type; };

    template<>
    struct add_reference<void> { typedef void type; };

    template<>
    struct add_reference<const void> { typedef const void type; };

    template<>
    struct add_reference<volatile void> { typedef volatile void type; };

    template<>
    struct add_reference<const volatile void> {typedef const volatile void type; };

    template<typename TType>
    struct add_const { typedef const TType type; };

    // Know Issues: Currently requires some kind of compiler support, otherwise unions are identified as classes.
    template<class TType>
    class is_class
    {
        template<typename U>
        static TrueType test(int U::*);

        template<typename U>
        static FalseType test(...);

    public:
        static const bool value = sizeof(test<TType>(0)) == sizeof(TrueType);
    };

    template<class TType>
    class is_empty
    {
        struct nonempty {char x; };
        struct test :
            Conditional<is_class<TType>::value, TType, nonempty>::type
        {char y; };
    public:
        static const bool value = sizeof(nonempty) == sizeof(test);
    };

    template<class T> struct is_const : FalseType {};
    template<class T> struct is_const<const T> : TrueType {};
}
