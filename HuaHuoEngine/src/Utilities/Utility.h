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

template<class T>
inline T clamp01(const T& t)
{
    if (t < 0)
        return 0;
    else if (t > 1)
        return 1;
    else
        return t;
}

template<>
inline float clamp01<float>(const float& t)
{
    if (t < 0.0F)
        return 0.0F;
    else if (t > 1.0F)
        return 1.0F;
    else
        return t;
}

#endif //HUAHUOENGINE_UTILITY_H
