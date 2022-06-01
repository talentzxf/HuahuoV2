#pragma once

#include "Color.h"

enum ColorSpace
{
    kUninitializedColorSpace = -1,
    kGammaColorSpace = 0,
    kLinearColorSpace,
    kColorSpaceCount,
    kCurrentColorSpace
};
// BIND_MANAGED_TYPE_NAME(ColorSpace, UnityEngine_ColorSpace);

// Returns true if the distance between f0 and f1 is smaller than epsilon
bool CompareApproximately(ColorRGBAf f0, ColorRGBAf f1, float epsilon = 0.000001F);

ColorSpace  GetActiveColorSpace();

#define LINEAR_TO_GAMMA_POW 0.45454545454545F
#define GAMMA_TO_LINEAR_POW 2.2F

// http://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt
// http://www.opengl.org/registry/specs/EXT/texture_sRGB_decode.txt
// {  cs / 12.92,                 cs <= 0.04045 }
// {  ((cs + 0.055)/1.055)^2.4,   cs >  0.04045 }
// NOTE: sRGB extensions above define range [0..1) only
// For the values [1..+inf] we use gamma=2.2 as an approximation

inline float GammaToLinearSpace(float value)
{
    if (value <= 0.04045F)
        return value / 12.92F;
    else if (value < 1.0F)
        return pow((value + 0.055F) / 1.055F, 2.4F);
    else if (value == 1.0F)
        return 1.0f;
    else
        return pow(value, GAMMA_TO_LINEAR_POW);
}

// http://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt
// http://www.opengl.org/registry/specs/EXT/texture_sRGB_decode.txt
// {  0.0,                          0         <= cl
// {  12.92 * c,                    0         <  cl < 0.0031308
// {  1.055 * cl^0.41666 - 0.055,   0.0031308 <= cl < 1
// NOTE: sRGB extensions above define range [0..1) only
// For the values [1..+inf] we use gamma=2.2 as an approximation

inline float LinearToGammaSpace(float value)
{
    if (value <= 0.0F)
        return 0.0F;
    else if (value <= 0.0031308F)
        return 12.92F * value;
    else if (value < 1.0F)
        return 1.055F * powf(value, 0.4166667F) - 0.055F;
    else if (value == 1.0F)
        return 1.0f;
    else
        return powf(value, LINEAR_TO_GAMMA_POW);
}

inline ColorRGBAf GammaToLinearSpace(const ColorRGBAf& value)
{
    return ColorRGBAf(GammaToLinearSpace(value.r), GammaToLinearSpace(value.g), GammaToLinearSpace(value.b), value.a);
}

inline ColorRGBAf LinearToGammaSpace(const ColorRGBAf& value)
{
    return ColorRGBAf(LinearToGammaSpace(value.r), LinearToGammaSpace(value.g), LinearToGammaSpace(value.b), value.a);
}

//@TODO: Rename to GammaToActiveSpace
inline float GammaToActiveColorSpace(float value)
{
    if (GetActiveColorSpace() == kLinearColorSpace)
        return GammaToLinearSpace(value);
    else
        return value;
}

//@TODO: Rename to GammaToActiveSpace
inline ColorRGBAf GammaToActiveColorSpace(const ColorRGBAf& value)
{
    if (GetActiveColorSpace() == kLinearColorSpace)
        return GammaToLinearSpace(value);
    else
        return value;
}

//@TODO: Rename to LinearToActiveSpace
inline float LinearToActiveColorSpace(float value)
{
    if (GetActiveColorSpace() == kGammaColorSpace)
        return LinearToGammaSpace(value);
    else
        return value;
}

//@TODO: Rename to LinearToActiveSpace
inline ColorRGBAf LinearToActiveColorSpace(const ColorRGBAf& value)
{
    if (GetActiveColorSpace() == kGammaColorSpace)
        return LinearToGammaSpace(value);
    else
        return value;
}

//@TODO: Rename to ActiveToGammaSpace
inline ColorRGBAf ActiveToGammaColorSpace(const ColorRGBAf& value)
{
    if (GetActiveColorSpace() == kLinearColorSpace)
        return LinearToGammaSpace(value);
    else
        return value;
}

inline float ActiveToGammaColorSpace(float value)
{
    if (GetActiveColorSpace() == kLinearColorSpace)
        return LinearToGammaSpace(value);
    else
        return value;
}

ColorRGBAf CorrelatedColorTemperatureToRGB(float kelvin);

// BIND_MANAGED_TYPE_NAME(ColorSpace, UnityEngine_ColorSpace);
