#pragma once

// DeconstructFunctionType allows you to take a function or method pointer type and take it apart
// and reconstruct it in various ways. This is mostly useful when writing templates that have to
// deal with functions in a generic way.
//
// Examples:
//
// * Take method of class C and turn it into a method of class B
//      DeconstructFunctionType<decltype(&C::SomeMethod)>::MakeMethodOf<B>::ResultType
//
// * Take a function type 'void (*)(int)' and turn it into 'int (*)(int)'
//      DeconstructFunctionType<void (*)(int)>::ReplaceReturnTypeWith<int>::ResultType
//
// * Get the signature (i.e. plain, non-pointer function type) of a function/method pointer type
//      DeconstructFunctionType<void (*)()>::Signature
//
// * Get the return type of a function/method pointer type
//      DeconstructFunctionType<int (*)(int)>::ReturnType
//
// * Get the class type from a method pointer type
//      DeconstructFunctionType<int (C::*)(int) const>::ClassType

template<typename F>
struct DeconstructFunctionType
{
};
#define DECONSTRUCT_FUNCTION_TYPE_IMPL(...) \
    struct DeconstructFunctionType<R (*)(__VA_ARGS__)> \
    { \
        typedef R ReturnType; \
        typedef void ClassType; \
        typedef R Signature(__VA_ARGS__); \
        typedef R (*PointerType)(__VA_ARGS__); \
        template<class C> \
        struct MakeMethodOf \
        { \
            typedef R (C::*ResultType)(__VA_ARGS__); \
        }; \
        template<typename X> \
        struct ReplaceReturnTypeWith \
        { \
            typedef X (*ResultType)(__VA_ARGS__); \
        }; \
        template<typename X> \
        struct PrependArgument \
        { \
            typedef R (*ResultType)(X, ##__VA_ARGS__); \
        }; \
    }

template<typename R>
DECONSTRUCT_FUNCTION_TYPE_IMPL();
template<typename R, typename A1>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1);
template<typename R, typename A1, typename A2>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2);
template<typename R, typename A1, typename A2, typename A3>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3);
template<typename R, typename A1, typename A2, typename A3, typename A4>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6, A7);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6, A7, A8);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6, A7, A8, A9);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
DECONSTRUCT_FUNCTION_TYPE_IMPL(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

#define DECONSTRUCT_METHOD_TYPE_IMPL(constKeyword, ...) \
    struct DeconstructFunctionType<R (C::*)(__VA_ARGS__) constKeyword> \
    { \
        typedef R ReturnType; \
        typedef C ClassType; \
        typedef R Signature(__VA_ARGS__); \
        typedef R (C::*NonConstPointerType)(__VA_ARGS__); \
        typedef R (C::*PointerType)(__VA_ARGS__) constKeyword; \
        template<class X> \
        struct MakeMethodOf \
        { \
            typedef R (X::*ResultType)(__VA_ARGS__) constKeyword; \
        }; \
        template<typename X> \
        struct ReplaceReturnTypeWith \
        { \
            typedef X (C::*ResultType)(__VA_ARGS__) constKeyword; \
        }; \
    }

template<typename C, typename R>
DECONSTRUCT_METHOD_TYPE_IMPL(, );
template<typename C, typename R, typename A1>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1);
template<typename C, typename R, typename A1, typename A2>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2);
template<typename C, typename R, typename A1, typename A2, typename A3>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6, A7);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6, A7, A8);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6, A7, A8, A9);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
DECONSTRUCT_METHOD_TYPE_IMPL(, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

template<typename C, typename R>
DECONSTRUCT_METHOD_TYPE_IMPL(const);
template<typename C, typename R, typename A1>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1);
template<typename C, typename R, typename A1, typename A2>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2);
template<typename C, typename R, typename A1, typename A2, typename A3>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6, A7);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6, A7, A8);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6, A7, A8, A9);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
template<typename C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
DECONSTRUCT_METHOD_TYPE_IMPL(const, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

#undef DECONSTRUCT_FUNCTION_TYPE_IMPL
#undef DECONSTRUCT_METHOD_TYPE_IMPL
