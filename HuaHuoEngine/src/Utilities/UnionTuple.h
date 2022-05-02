#pragma once

#include <cstring>
#include "Annotations.h"
#include "baselib/include/IntegerDefinitions.h"
#include "StaticAssert.h"

//////////////////////////////////////////////////////////////////////////
/// Union with two types
template<typename A, typename B>
union UnionTuple
{
    typedef A first_type;
    typedef B second_type;

    A first;
    B second;
};
#if !defined(_SCE_GNMP_COMMON_H) // bit_cast is duplicated by this sdk code

template<class Target, class Source>
UNITY_FORCEINLINE Target bit_cast(const Source& a)
{
    // See also C++ proposal of function with similar semantics and the same name:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0476r2.html

    // TODO: forbid casting of pointers via this. Without additional compiler
    // support this definitely breaks rules of strict aliasing.

    // Using union to convert between bit representations is the only way that
    // is somewhat portable between compilers. Note that by the letter of the
    // C++ standard even this is undefined behavior, because we're reading from
    // the union member which we did not write to during the last write. However,
    // the compilers provide support for this pattern as an extension.

    CompileTimeAssert(sizeof(Target) == sizeof(Source),
        "Aliasing type with a type of different size is not allowed");

    UnionTuple<Source, Target> storage;
    storage.first = a;
    return storage.second;
}

#endif
// Just like bit_cast, except that cast to larger type is allowed
template<class Target, class Source>
UNITY_FORCEINLINE Target bit_cast_to_maybe_larger(const Source& a)
{
    CompileTimeAssert(sizeof(Target) >= sizeof(Source),
        "Aliasing type with a type of different size is not allowed");

    Target target{}; // make sure we default-initialize, as source may not overwrite whole value
    std::memcpy(&target, &a, sizeof(Source));
    return target;
}

// Just like bit_cast, except that cast to smaller type is allowed
template<class Target, class Source>
UNITY_FORCEINLINE Target bit_cast_to_maybe_smaller(const Source& a)
{
    CompileTimeAssert(sizeof(Target) <= sizeof(Source),
        "Aliasing type with a type of different size is not allowed");

    Target target{}; // make sure we default-initialize, as source may not overwrite whole value
    std::memcpy(&target, &a, sizeof(Target));
    return target;
}

// Use this metafunction together with UnionTuple<A, B>, for type B. It returns a type of same size
// that can be safely loaded into registers while data is in endianess-swapped form.
template<class T>
struct BitSafeTypeFor
{
    typedef T type;
};

template<>
struct BitSafeTypeFor<float>
{
    typedef UInt32 type;
};

template<>
struct BitSafeTypeFor<double>
{
    typedef UInt64 type;
};
