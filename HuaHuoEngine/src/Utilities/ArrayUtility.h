//
// Created by VincentZhang on 4/11/2022.
//

#ifndef HUAHUOENGINE_ARRAYUTILITY_H
#define HUAHUOENGINE_ARRAYUTILITY_H
// ARRAY_SIZE: Element count of a static array

#undef ARRAY_SIZE

#if defined(__GNUC__) && __cplusplus < 201103L
#   define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#else
#   include <cstdlib> // size_t
#   include <stddef.h>  // size_t on Vita

template<typename T, size_t N>
char(&ARRAY_SIZE_REQUIRES_ARRAY_ARGUMENT(T(&)[N]))[N];

#   define ARRAY_SIZE(x) sizeof(ARRAY_SIZE_REQUIRES_ARRAY_ARGUMENT(x))
#endif

#endif //HUAHUOENGINE_ARRAYUTILITY_H
