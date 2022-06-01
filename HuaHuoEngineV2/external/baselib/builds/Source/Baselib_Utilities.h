#pragma once

#include <string.h>

namespace detail
{
    template<typename NativeType, typename BaselibHandleType>
    static inline NativeType AsNativeType(BaselibHandleType baselibHandle)
    {
        static_assert(sizeof(BaselibHandleType::handle) >= sizeof(NativeType), "native type does not fit baselib handle");
        NativeType result = {};
        memcpy(&result, &baselibHandle.handle, sizeof(result));
        return result;
    }

    template<typename BaselibHandleType, typename NativeType>
    static inline BaselibHandleType AsBaselibHandle(NativeType nativeType)
    {
        static_assert(sizeof(BaselibHandleType::handle) >= sizeof(NativeType), "native type does not fit baselib handle");
        decltype(BaselibHandleType::handle)result = {};
        memcpy(&result, &nativeType, sizeof(nativeType));
        return BaselibHandleType { result };
    }
}
