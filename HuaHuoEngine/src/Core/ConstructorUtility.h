//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_CONSTRUCTORUTILITY_H
#define HUAHUOENGINE_CONSTRUCTORUTILITY_H

#include "Memory/AllocatorLabels.h"
#include "Internal/PlatformEnvironment.h"
#include <utility>

#if defined(_MSC_VER)
#define HAS_TRIVIAL_DESTRUCTOR(T) (__has_trivial_destructor(T) || !__is_class(T))
#define HAS_TRIVIAL_CONSTRUCTOR(T) (__has_trivial_constructor(T) || !__is_class(T))
#define SUPPORTS_HAS_TRIVIAL_DESTRUCTOR 1
#define SUPPORTS_HAS_TRIVIAL_CONSTRUCTOR 1
// most compilers support __has_trivial_destructor
#else
#define HAS_TRIVIAL_DESTRUCTOR(T) (__has_trivial_destructor(T))
#define HAS_TRIVIAL_CONSTRUCTOR(T) (__has_trivial_constructor(T))
#define SUPPORTS_HAS_TRIVIAL_DESTRUCTOR 1
#define SUPPORTS_HAS_TRIVIAL_CONSTRUCTOR 1
#endif


// The default placement new constructor actually performs null checks.
// The cost of this vs direct function calls can be roughly 20% on MacBookPro.
// For this reason we have a special placement new override which doesn't support null and seems to get the same performance
// as just assigning a value directly.
// See ArrayPerformanceTests.cpp

// non-noexcept allocator
enum non_null_placement_t { non_null_placement };
inline void* operator new(size_t, void* ptr, non_null_placement_t) { COMPILER_BUILTIN_ASSUME(ptr != NULL); return ptr; }
inline void operator delete(void*, void*, non_null_placement_t) {}


// Internal usage only. (bool signifies HAS_TRIVIAL_DESTRUCTOR)
template<bool>
struct destruct_internal
{
    template<typename T>
    static inline void destruct(T *mem)
    {
        mem->~T();
    }
};

template<>
struct destruct_internal<true>
{
    template<typename T>
    static inline void destruct(T *mem) {}
};

template<typename T> inline
void copy_construct(T* mem, const T& orig)
{
    ::new((void*)mem, non_null_placement) T(orig);
}

template<typename T> inline
void copy_construct_n(T* mem, size_t n, const T& orig)
{
    for (size_t i = 0; i != n; i++)
        ::new((void*)(mem + i), non_null_placement) T(orig);
}

template<typename T> inline
void copy_construct_array(T* mem, size_t n, const T* src)
{
    if (HAS_TRIVIAL_DESTRUCTOR(T))
    {
        memcpy((void*)mem, src, n * sizeof(T));
    }
    else
    {
        for (size_t i = 0; i != n; i++)
            ::new((void*)(mem + i), non_null_placement) T(src[i]);
    }
}

template<typename T> inline
void default_construct_n(T* mem, size_t n)
{
    for (size_t i = 0; i != n; i++)
        ::new((void*)(mem + i), non_null_placement) T();
}

template<typename T> inline
void default_construct(T* mem)
{
    ::new((void*)mem, non_null_placement) T();
}

template<typename T> inline
void destruct(T* mem)
{
    destruct_internal<HAS_TRIVIAL_DESTRUCTOR(T)>::destruct(mem);
}

template<typename T> inline
void destruct_n(T* mem, size_t n)
{
    if (!HAS_TRIVIAL_DESTRUCTOR(T))
    {
        for (size_t i = 0; i != n; i++)
            destruct_internal<HAS_TRIVIAL_DESTRUCTOR(T)>::destruct(mem + i);
    }
}

template<typename T, bool supportLabels>
struct AllocatorTraitsImpl
{
    inline static T* Construct(void* data, MemLabelRef)
    {
        return ::new((void*)data, non_null_placement) T();
    }

    inline static T* Construct(void* data, MemLabelRef, const T& other)
    {
        return ::new((void*)data, non_null_placement) T(other);
    }

    template<typename ... Args> inline
    static T* Construct(void* data, MemLabelRef, Args&& ... args)
    {
        return ::new((void*)data, non_null_placement) T(std::forward<Args>(args)...);
    }
};

template<typename T>
struct AllocatorTraitsImpl<T, true>
{
    inline static T* Construct(void* data, MemLabelRef label)
    {
        return ::new(data, non_null_placement) T(label);
    }

    inline static T* Construct(void* data, MemLabelRef label, const T& other)
    {
        return ::new(data, non_null_placement) T(other, label);
    }

    template<typename ... Args> inline
    static T* Construct(void* data, MemLabelRef label, Args&& ... args)
    {
        return ::new(data, non_null_placement) T(std::forward<Args>(args)..., label);
    }
};

template<typename T>
struct ConstructorSupportLabel
{
    struct ImplicitLabel { operator MemLabelRef() { return kMemDefault;} };

    struct dummie {};

    // Detect if there is template<typename T> C::C(const T&)
    template<typename U> static std::true_type  deduceT(decltype(U(dummie()))*);
    template<typename U> static std::false_type deduceT(...);

    // Detect if there is C::C(MemLabelRef) or C::C(const MemLabelRef&) or explicit C::C(MemLabelRef)
    template<typename U> static std::true_type  deduce(decltype(U(ImplicitLabel()))*);
    template<typename U> static std::false_type deduce(...);

    // Detect if there is C::C(const C&, MemLabelRef) or C::C(const C&, const MemLabelRef&) or explicit C::C(const C&, MemLabelRef)
    template<typename U> static std::true_type  deduceCopy(decltype(new U(*(const T*)0, ImplicitLabel())));
    template<typename U> static std::false_type deduceCopy(...);

    // Detect if there is template<typename T> C::C(const T&, MemLabelRef)
    template<typename U> static std::true_type  deduceCopyT1(decltype(new U(dummie(), ImplicitLabel())));
    template<typename U> static std::false_type deduceCopyT1(...);

    // Detect if there is template<typename T> C::C(const C&, const T&)
    template<typename U> static std::true_type  deduceCopyT2(decltype(new U(*(const T*)0, dummie())));
    template<typename U> static std::false_type deduceCopyT2(...);

    static constexpr bool result = std::is_same<decltype(deduce<T>(0)), std::true_type>::value
                                   && !std::is_same<decltype(deduceT<T>(0)), std::true_type>::value;
    static constexpr bool resultCopy = std::is_same<decltype(deduceCopy<T>(0)), std::true_type>::value
                                       && !std::is_same<decltype(deduceCopyT1<T>(0)), std::true_type>::value
                                       && !std::is_same<decltype(deduceCopyT2<T>(0)), std::true_type>::value;
};

template<typename T>
struct ConstructorSupportLabel<T&>
{
    static constexpr bool result = false;
    static constexpr bool resultCopy = false;
};

template<typename T>
struct ConstructorSupportLabel<T*>
{
    static constexpr bool result = false;
    static constexpr bool resultCopy = false;
};


template<typename T>
using AllocatorTraits = AllocatorTraitsImpl<T, ConstructorSupportLabel<T>::result>;

template<typename T>
struct AutoLabelConstructor
{
    using value_type = typename std::remove_const<T>::type;
    using AllocatorTrait = AllocatorTraitsImpl<value_type, ConstructorSupportLabel<value_type>::result>;

    static inline value_type* construct(void* mem, MemLabelRef label)
    {
        return AllocatorTrait::template Construct(mem, label);
    }

    static value_type* construct_n(void* mem, size_t n, MemLabelRef label)
    {
        value_type* entries = reinterpret_cast<value_type*>(mem);

        for (size_t i = 0; i != n; i++)
            AllocatorTrait::template Construct(reinterpret_cast<void*>(entries + i), label);

        return entries;
    }

    template<typename ... Args> inline
    static value_type* construct_args(void* mem, MemLabelRef label, Args&& ... args)
    {
        return AllocatorTrait::template Construct<Args...>(mem, label, std::forward<Args>(args)...);
    }

    template<typename ... Args>
    static value_type* construct_n_args(void* mem, size_t n, MemLabelRef label, Args&& ... args)
    {
        value_type* entries = reinterpret_cast<value_type*>(mem);

        for (size_t i = 0; i != n; i++)
            AllocatorTrait::template Construct<Args...>(reinterpret_cast<void*>(entries + i), label, std::forward<Args>(args)...);

        return entries;
    }

    template<typename ITER>
    static value_type* construct_array(void* mem, size_t n, ITER src, MemLabelRef label)
    {
        value_type* entries = reinterpret_cast<value_type*>(mem);

        for (size_t i = 0; i != n; i++, ++src)
            AllocatorTrait::template Construct(reinterpret_cast<void*>(entries + i), label, *src);

        return entries;
    }

    static value_type* construct_array(void* mem, size_t n, const value_type* src, MemLabelRef label)
    {
        value_type* entries = reinterpret_cast<value_type*>(mem);

        if (HAS_TRIVIAL_DESTRUCTOR(T))
        {
            memcpy(mem, src, n * sizeof(T));
        }
        else
        {
            for (size_t i = 0; i != n; i++)
                AllocatorTrait::template Construct(reinterpret_cast<void*>(entries + i), label, src[i]);
        }

        return entries;
    }
};

#endif //HUAHUOENGINE_CONSTRUCTORUTILITY_H
