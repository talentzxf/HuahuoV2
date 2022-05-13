//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_UTILITY_H
#define HUAHUOENGINE_UTILITY_H
#include <cstdlib>

template<class T>
inline T* Stride(T* p, size_t offset)
{
    return reinterpret_cast<T*>((char*)p + offset);
}

#endif //HUAHUOENGINE_UTILITY_H
