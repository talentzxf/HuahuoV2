#pragma once

#include <cstdlib>

#define CompileTimeAssertArraySize(array, size) static_assert(ARRAY_SIZE(array) == (size), "Wrong number of elements in array " #array)
#define CompileTimeAssert static_assert

/// Usage Example:
/// int a;
/// double b;
/// AssertVariableType(a, int); // OK
/// AssertVariableType(b, int); // Error during compile time
/// AssertIfVariableType(a, int); // Error during compile time
/// AssertIfVariableType(b, int); // OK
#define AssertVariableType(Variable, Type)      static_assert(IsSameType<Type,decltype(Variable)>::result, #Variable" is not of type "#Type)
#define AssertIfVariableType(Variable, Type)    static_assert(!IsSameType<Type,decltype(Variable)>::result, #Variable" is of type "#Type)

/// Validate type size
/// Usage:
///
/// struct MyStruct {
///    int i;
/// };
/// CompileTimeAssertTypeSize(MyStruct, 4);
#define CompileTimeAssertTypeSize(Type, ExpectedSize) ::static_assert_detail::CheckSize<Type, ExpectedSize> _validateType##Type;

namespace static_assert_detail
{
    template<typename ToCheck, size_t ExpectedSize, size_t RealSize = sizeof(ToCheck)>
    struct CheckSize
    {
        static_assert(RealSize == ExpectedSize, "The type has wrong size");
    };
}
