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


template<class T>
inline T clamp(const T&t, const T& t0, const T& t1)
{
    if (t < t0)
        return t0;
    else if (t > t1)
        return t1;
    else
        return t;
}

template<>
inline float clamp(const float&t, const float& t0, const float& t1)
{
    if (t < t0)
        return t0;
    else if (t > t1)
        return t1;
    else
        return t;
}

#endif //HUAHUOENGINE_UTILITY_H
