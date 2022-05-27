//
// Created by VincentZhang on 5/26/2022.
//
#include "Format.h"
#include <string>
#include "GfxDevice/GfxDeviceTypes.h"
#include "Utilities/StaticAssert.h"
#include "Utilities/Word.h"

namespace
{
    template<typename T>
    std::string ToString(const T& value)
    {
        return IntToString(value);
    }

    static const FormatDesc s_FormatDescTable[] = //kFormatCount
            {
                    // blockSize, blockX, blockY, blockZ, swizzleR, swizzleG, swizzleB, swizzleA, fallbackFormat, alphaFormat, linearFormat, srgbFormat, textureFormat, renderTextureFormat, colorComponents, alphaComponents, name, flags
                    {0, 0, 0, 0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatNone, kFormatNone, kFormatNone, kFormatNone, kTexFormatNone, kRTFormatCount, 0, 0, "None", 0},          //kFormatNone

                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatR8G8B8A8_SRGB, kFormatR8_UNorm, kFormatR8_SRGB, kTexFormatR8, kRTFormatR8, 1, 0, "R8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},                             //kFormatR8_SRGB
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatR8G8B8A8_SRGB, kFormatR8G8_UNorm, kFormatR8G8_SRGB, kTexFormatRG16, kRTFormatRG16, 2, 0, "R8G8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},                   //kFormatR8G8_SRGB
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatR8G8B8A8_SRGB, kFormatR8G8B8_UNorm, kFormatR8G8B8_SRGB, kTexFormatRGB24, kRTFormatCount, 3, 0, "R8G8B8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},           //kFormatR8G8B8_SRGB
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_SRGB, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_SRGB, kTexFormatRGBA32, kRTFormatARGB32, 3, 1, "R8B8G8A8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},  //kFormatR8G8B8A8_SRGB

                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_UNorm, kFormatR8_UNorm, kFormatR8_SRGB, kTexFormatR8, kRTFormatR8, 1, 0, "R8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                        //kFormatR8_UNorm
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_UNorm, kFormatR8G8_UNorm, kFormatR8G8_SRGB, kTexFormatRG16, kRTFormatRG16, 2, 0, "R8G8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},              //kFormatR8G8_UNorm
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_UNorm, kFormatR8G8B8_UNorm, kFormatR8G8B8_SRGB, kTexFormatRGB24, kRTFormatCount, 3, 0, "R8G8B8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},      //kFormatR8G8B8_UNorm
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_SRGB, kTexFormatRGBA32, kRTFormatARGB32, 3, 1, "R8B8G8A8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},        //kFormatR8G8B8A8_UNorm
                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SNorm, kFormatR8G8B8A8_SNorm, kFormatR8_SNorm, kFormatR8_SNorm, kTexFormatNone, kRTFormatCount, 1, 0, "R8", kFormatPropertyNormBit | kFormatPropertySignedBit},                    //kFormatR8_SNorm
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SNorm, kFormatR8G8B8A8_SNorm, kFormatR8G8_SNorm, kFormatR8G8_SNorm, kTexFormatNone, kRTFormatCount, 2, 0, "R8G8", kFormatPropertyNormBit | kFormatPropertySignedBit},              //kFormatR8G8_SNorm
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SNorm, kFormatR8G8B8A8_SNorm, kFormatR8G8B8_SNorm, kFormatR8G8B8_SNorm, kTexFormatNone, kRTFormatCount, 3, 0, "R8G8B8", kFormatPropertyNormBit | kFormatPropertySignedBit},        //kFormatR8G8B8_SNorm
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR8G8B8A8_SNorm, kFormatR8G8B8A8_SNorm, kFormatR8G8B8A8_SNorm, kTexFormatNone, kRTFormatCount, 3, 1, "R8B8G8A8", kFormatPropertyNormBit | kFormatPropertySignedBit},            //kFormatR8G8B8A8_SNorm
                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UInt, kFormatR8G8B8A8_UInt, kFormatR8_UInt, kFormatR8_UInt, kTexFormatNone, kRTFormatCount, 1, 0, "R8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},                   //kFormatR8_UInt
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UInt, kFormatR8G8B8A8_UInt, kFormatR8G8_UInt, kFormatR8G8_UInt, kTexFormatNone, kRTFormatCount, 2, 0, "R8G8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},             //kFormatR8G8_UInt
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UInt, kFormatR8G8B8A8_UInt, kFormatR8G8B8_UInt, kFormatR8G8B8_UInt, kTexFormatNone, kRTFormatCount, 3, 0, "R8G8B8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},       //kFormatR8G8B8_UInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR8G8B8A8_UInt, kFormatR8G8B8A8_UInt, kFormatR8G8B8A8_UInt, kTexFormatNone, kRTFormatCount, 3, 1, "R8B8G8A8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},          //kFormatR8G8B8A8_UInt
                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SInt, kFormatR8G8B8A8_SInt, kFormatR8_SInt, kFormatR8_SInt, kTexFormatNone, kRTFormatCount, 1, 0, "R8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                     //kFormatR8_SInt
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SInt, kFormatR8G8B8A8_SInt, kFormatR8G8_SInt, kFormatR8G8_SInt, kTexFormatNone, kRTFormatCount, 2, 0, "R8G8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},               //kFormatR8G8_SInt
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SInt, kFormatR8G8B8A8_SInt, kFormatR8G8B8_SInt, kFormatR8G8B8_SInt, kTexFormatNone, kRTFormatCount, 3, 0, "R8G8B8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},         //kFormatR8G8B8_SInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR8G8B8A8_SInt, kFormatR8G8B8A8_SInt, kFormatR8G8B8A8_SInt, kTexFormatNone, kRTFormatCount, 3, 1, "R8B8G8A8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},            //kFormatR8G8B8A8_SInt

                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_UNorm, kFormatR16G16B16A16_UNorm, kFormatR16_UNorm, kFormatR16_UNorm, kTexFormatR16, kRTFormatR16, 1, 0, "R16", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                          //kFormatR16_UNorm
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_UNorm, kFormatR16G16B16A16_UNorm, kFormatR16G16_UNorm, kFormatR16G16_UNorm, kTexFormatRG32, kRTFormatRG32, 2, 0, "R16G16", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},               //kFormatR16G16_UNorm
                    {6, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_UNorm, kFormatR16G16B16A16_UNorm, kFormatR16G16B16_UNorm, kFormatR16G16B16_UNorm, kTexFormatRGB48, kRTFormatCount, 3, 0, "R16G16B16", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},    //kFormatR16G16B16_UNorm
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR16G16B16A16_UNorm, kFormatR16G16B16A16_UNorm, kFormatR16G16B16A16_UNorm, kTexFormatRGBA64, kRTFormatARGB64, 3, 1, "R16G16B16A16", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},       //kFormatR16G16B16A16_UNorm
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SNorm, kFormatR16G16B16A16_SNorm, kFormatR16_SNorm, kFormatR16_SNorm, kTexFormatNone, kRTFormatCount, 1, 0, "R16", kFormatPropertyNormBit | kFormatPropertySignedBit},                         //kFormatR16_SNorm
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SNorm, kFormatR16G16B16A16_SNorm, kFormatR16G16_SNorm, kFormatR16G16_SNorm, kTexFormatNone, kRTFormatCount, 2, 0, "R16G16", kFormatPropertyNormBit | kFormatPropertySignedBit},                //kFormatR16G16_SNorm
                    {6, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_SNorm, kFormatR16G16B16A16_SNorm, kFormatR16G16B16_SNorm, kFormatR16G16B16_SNorm, kTexFormatNone, kRTFormatCount, 3, 0, "R16G16B16", kFormatPropertyNormBit | kFormatPropertySignedBit},       //kFormatR16G16B16_SNorm
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR16G16B16A16_SNorm, kFormatR16G16B16A16_SNorm, kFormatR16G16B16A16_SNorm, kTexFormatNone, kRTFormatCount, 3, 1, "R16G16B16A16", kFormatPropertyNormBit | kFormatPropertySignedBit},            //kFormatR16G16B16A16_SNorm
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_UInt, kFormatR16G16B16A16_UInt, kFormatR16_UInt, kFormatR16_UInt, kTexFormatNone, kRTFormatCount, 1, 0, "R16", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},                        //kFormatR16_UInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_UInt, kFormatR16G16B16A16_UInt, kFormatR16G16_UInt, kFormatR16G16_UInt, kTexFormatNone, kRTFormatCount, 2, 0, "R16G16", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},               //kFormatR16G16_UInt
                    {6, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_UInt, kFormatR16G16B16A16_UInt, kFormatR16G16B16_UInt, kFormatR16G16B16_UInt, kTexFormatNone, kRTFormatCount, 3, 0, "R16G16B16", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},      //kFormatR16G16B16_UInt
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR16G16B16A16_UInt, kFormatR16G16B16A16_UInt, kFormatR16G16B16A16_UInt, kTexFormatNone, kRTFormatRGBAUShort, 3, 1, "R16G16B16A16", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},     //kFormatR16G16B16A16_UInt
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SInt, kFormatR16G16B16A16_SInt, kFormatR16_SInt, kFormatR16_SInt, kTexFormatNone, kRTFormatCount, 1, 0, "R16", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                          //kFormatR16_SInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SInt, kFormatR16G16B16A16_SInt, kFormatR16G16_SInt, kFormatR16G16_SInt, kTexFormatNone, kRTFormatCount, 2, 0, "R16G16", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                 //kFormatR16G16_SInt
                    {6, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_SInt, kFormatR16G16B16A16_SInt, kFormatR16G16B16_SInt, kFormatR16G16B16_SInt, kTexFormatNone, kRTFormatCount, 3, 0, "R16G16B16", kFormatPropertyIntegerBit | kFormatPropertySignedBit},        //kFormatR16G16B16_SInt
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR16G16B16A16_SInt, kFormatR16G16B16A16_SInt, kFormatR16G16B16A16_SInt, kTexFormatNone, kRTFormatCount, 3, 1, "R16G16B16A16", kFormatPropertyIntegerBit | kFormatPropertySignedBit},            //kFormatR16G16B16A16_SInt

                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_UInt, kFormatR32G32B32A32_UInt, kFormatR32_UInt, kFormatR32_UInt, kTexFormatNone, kRTFormatCount, 1, 0, "R32", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},                     //kFormatR32_UInt
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_UInt, kFormatR32G32B32A32_UInt, kFormatR32G32_UInt, kFormatR32G32_UInt, kTexFormatNone, kRTFormatCount, 2, 0, "R32G32", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},            //kFormatR32G32_UInt
                    {12, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR32G32B32A32_UInt, kFormatR32G32B32A32_UInt, kFormatR32G32B32_UInt, kFormatR32G32B32_UInt, kTexFormatNone, kRTFormatCount, 3, 0, "R32G32B32", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},  //kFormatR32G32B32_UInt
                    {16, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR32G32B32A32_UInt, kFormatR32G32B32A32_UInt, kFormatR32G32B32A32_UInt, kTexFormatNone, kRTFormatCount, 3, 1, "R32G32B32A32", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},      //kFormatR32G32B32A32_UInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_SInt, kFormatR32G32B32A32_SInt, kFormatR32_SInt, kFormatR32_SInt, kTexFormatNone, kRTFormatRInt, 1, 0, "R32", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                        //kFormatR32_SInt
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_SInt, kFormatR32G32B32A32_SInt, kFormatR32G32_SInt, kFormatR32G32_SInt, kTexFormatNone, kRTFormatRGInt, 2, 0, "R32G32", kFormatPropertyIntegerBit | kFormatPropertySignedBit},              //kFormatR32G32_SInt
                    {12, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR32G32B32A32_SInt, kFormatR32G32B32A32_SInt, kFormatR32G32B32_SInt, kFormatR32G32B32_SInt, kTexFormatNone, kRTFormatCount, 3, 0, "R32G32B32", kFormatPropertyIntegerBit | kFormatPropertySignedBit},    //kFormatR32G32B32_SInt
                    {16, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR32G32B32A32_SInt, kFormatR32G32B32A32_SInt, kFormatR32G32B32A32_SInt, kTexFormatNone, kRTFormatARGBInt, 3, 1, "R32G32B32A32", kFormatPropertyIntegerBit | kFormatPropertySignedBit},      //kFormatR32G32B32A32_SInt

                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SFloat, kFormatR16G16B16A16_SFloat, kFormatR16_SFloat, kFormatR16_SFloat, kTexFormatRHalf, kRTFormatRHalf, 1, 0, "RHalf", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},                               //kFormatR16_SFloat
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR16G16B16A16_SFloat, kFormatR16G16B16A16_SFloat, kFormatR16G16_SFloat, kFormatR16G16_SFloat, kTexFormatRGHalf, kRTFormatRGHalf, 2, 0, "RGHalf", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},                      //kFormatR16G16_SFloat
                    {6, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_SFloat, kFormatR16G16B16A16_SFloat, kFormatR16G16B16_SFloat, kFormatR16G16B16_SFloat, kTexFormatNone, kRTFormatCount, 3, 0, "RGBHalf", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},                  //kFormatR16G16B16_SFloat
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR32G32B32A32_SFloat, kFormatR16G16B16A16_SFloat, kFormatR16G16B16A16_SFloat, kFormatR16G16B16A16_SFloat, kTexFormatRGBAHalf, kRTFormatARGBHalf, 3, 1, "RGBAHalf", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},    //kFormatR16G16B16A16_SFloat
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_SFloat, kFormatR32G32B32A32_SFloat, kFormatR32_SFloat, kFormatR32_SFloat, kTexFormatRFloat, kRTFormatRFloat, 1, 0, "RFloat", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},                            //kFormatR32_SFloat
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR32G32B32A32_SFloat, kFormatR32G32B32A32_SFloat, kFormatR32G32_SFloat, kFormatR32G32_SFloat, kTexFormatRGFloat, kRTFormatRGFloat, 2, 0, "RGFloat", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},                   //kFormatR32G32_SFloat
                    {12, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR32G32B32A32_SFloat, kFormatR32G32B32A32_SFloat, kFormatR32G32B32_SFloat, kFormatR32G32B32_SFloat, kTexFormatRGBFloat, kRTFormatCount, 3, 0, "RGBFloat", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},            //kFormatR32G32B32_SFloat
                    {16, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatR32G32B32A32_SFloat, kFormatR32G32B32A32_SFloat, kFormatR32G32B32A32_SFloat, kTexFormatRGBAFloat, kRTFormatARGBFloat, 3, 1, "RGBAFloat", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},               //kFormatR32G32B32A32_SFloat

                    {1, 1, 1, 1, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzleR, kFormatR8G8B8A8_UNorm, kFormatR8G8B8A8_UNorm, kFormatL8_UNorm, kFormatL8_UNorm, kTexFormatNone, kRTFormatCount, 1, 0, "L8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},  //kFormatL8_UNorm
                    {1, 1, 1, 1, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzleR, kFormatR8G8B8A8_UNorm, kFormatA8_UNorm, kFormatA8_UNorm, kFormatA8_UNorm, kTexFormatAlpha8, kRTFormatCount, 0, 1, "A8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit}, //kFormatA8_UNorm
                    {2, 1, 1, 1, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzleR, kFormatA8_UNorm, kFormatA16_UNorm, kFormatA16_UNorm, kFormatA16_UNorm, kTexFormatNone, kRTFormatCount, 0, 1, "A16", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},     //kFormatA16_UNorm

                    {3, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR8G8B8_SRGB, kFormatB8G8R8A8_SRGB, kFormatB8G8R8_UNorm, kFormatB8G8R8_SRGB, kTexFormatBGR24, kRTFormatCount, 3, 0, "B8G8R8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},             //kFormatB8G8R8_SRGB
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatB8G8R8A8_SRGB, kFormatB8G8R8A8_UNorm, kFormatB8G8R8A8_SRGB, kTexFormatBGRA32, kRTFormatBGRA32, 3, 1, "B8G8R8A8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},   //kFormatB8G8R8A8_SRGB
                    {3, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR8G8B8_UNorm, kFormatB8G8R8A8_UNorm, kFormatB8G8R8_UNorm, kFormatB8G8R8_SRGB, kTexFormatBGR24, kRTFormatCount, 3, 0, "B8G8R8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                    //kFormatB8G8R8_UNorm
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatB8G8R8A8_UNorm, kFormatB8G8R8A8_UNorm, kFormatB8G8R8A8_SRGB, kTexFormatBGRA32, kRTFormatBGRA32, 3, 1, "B8G8R8A8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                          //kFormatB8G8R8A8_UNorm
                    {3, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR8G8B8_SNorm, kFormatB8G8R8A8_SNorm, kFormatB8G8R8_SNorm, kFormatB8G8R8_SNorm, kTexFormatNone, kRTFormatCount, 3, 0, "B8G8R8", kFormatPropertyNormBit | kFormatPropertySignedBit},                                      //kFormatB8G8R8_SNorm
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR8G8B8A8_SNorm, kFormatB8G8R8A8_SNorm, kFormatB8G8R8A8_SNorm, kFormatB8G8R8A8_SNorm, kTexFormatNone, kRTFormatCount, 3, 1, "B8G8R8A8", kFormatPropertyNormBit | kFormatPropertySignedBit},                              //kFormatB8G8R8A8_SNorm
                    {3, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR8G8B8_UInt, kFormatB8G8R8A8_UInt, kFormatB8G8R8_UInt, kFormatB8G8R8_UInt, kTexFormatNone, kRTFormatCount, 3, 0, "B8G8R8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},                                     //kFormatB8G8R8_UInt
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR8G8B8A8_UInt, kFormatB8G8R8A8_UInt, kFormatB8G8R8A8_UInt, kFormatB8G8R8A8_UInt, kTexFormatNone, kRTFormatCount, 3, 1, "B8G8R8A8", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit},                             //kFormatB8G8R8A8_UInt
                    {3, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR8G8B8_SInt, kFormatB8G8R8A8_SInt, kFormatB8G8R8_SInt, kFormatB8G8R8_SInt, kTexFormatNone, kRTFormatCount, 3, 0, "B8G8R8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                                       //kFormatB8G8R8_SInt
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR8G8B8A8_SInt, kFormatB8G8R8A8_SInt, kFormatB8G8R8_SInt, kFormatB8G8R8_SInt, kTexFormatNone, kRTFormatCount, 3, 1, "B8G8R8A8", kFormatPropertyIntegerBit | kFormatPropertySignedBit},                                   //kFormatB8G8R8A8_SInt

                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatR4G4B4A4_UNormPack16, kFormatR4G4B4A4_UNormPack16, kFormatR4G4B4A4_UNormPack16, kTexFormatRGBA4444, kRTFormatCount, 3, 1, "RGBA4", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},              //kFormatR4G4B4A4_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR4G4B4A4_UNormPack16, kFormatB4G4R4A4_UNormPack16, kFormatB4G4R4A4_UNormPack16, kFormatB4G4R4A4_UNormPack16, kTexFormatARGB4444, kRTFormatARGB4444, 3, 1, "BGRA4", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},     //kFormatB4G4R4A4_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8_UNorm, kFormatR4G4B4A4_UNormPack16, kFormatR5G6B5_UNormPack16, kFormatR5G6B5_UNormPack16, kTexFormatNone, kRTFormatCount, 3, 0, "R5G6B5", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},                       //kFormatR5G6B5_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR5G6B5_UNormPack16, kFormatB4G4R4A4_UNormPack16, kFormatB5G6R5_UNormPack16, kFormatB5G6R5_UNormPack16, kTexFormatRGB565, kRTFormatRGB565, 3, 0, "B5G6B5", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},              //kFormatB5G6R5_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatR4G4B4A4_UNormPack16, kFormatR5G5B5A1_UNormPack16, kFormatR5G5B5A1_UNormPack16, kTexFormatNone, kRTFormatCount, 3, 1, "RGB5A1", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},                 //kFormatR5G5B5A1_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR5G5B5A1_UNormPack16, kFormatB4G4R4A4_UNormPack16, kFormatB5G5R5A1_UNormPack16, kFormatB5G5R5A1_UNormPack16, kTexFormatNone, kRTFormatARGB1555, 3, 1, "BGR5A1", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},        //kFormatB5G5R5A1_UNormPack16
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatB5G5R5A1_UNormPack16, kFormatR4G4B4A4_UNormPack16, kFormatA1R5G5B5_UNormPack16, kFormatA1R5G5B5_UNormPack16, kTexFormatNone, kRTFormatCount, 3, 1, "A1RGB5", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},           //kFormatA1R5G5B5_UNormPack16

                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16_SFloat, kFormatR16G16B16A16_SFloat, kFormatE5B9G9R9_UFloatPack32, kFormatE5B9G9R9_UFloatPack32, kTexFormatRGB9e5Float, kRTFormatCount, 3, 0, "RGB9E5", kFormatPropertyIEEE754Bit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},        //kFormatE5B9G9R9_UFloatPack32
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16_SFloat, kFormatR16G16B16A16_SFloat, kFormatB10G11R11_UFloatPack32, kFormatB10G11R11_UFloatPack32, kTexFormatNone, kRTFormatR11G11B10Float, 3, 0, "RG11B10", kFormatPropertyIEEE754Bit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit},   //kFormatB10G11R11_UFloatPack32

                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR16G16B16A16_UNorm, kFormatA2B10G10R10_UNormPack32, kFormatA2B10G10R10_UNormPack32, kFormatA2B10G10R10_UNormPack32, kTexFormatNone, kRTFormatA2R10G10B10, 3, 1, "RGB10A2", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit}, //kFormatA2B10G10R10_UNormPack32
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR16G16B16A16_UInt, kFormatA2B10G10R10_UIntPack32, kFormatA2B10G10R10_UIntPack32, kFormatA2B10G10R10_UIntPack32, kTexFormatNone, kRTFormatCount, 3, 1, "RGB10A2", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},        //kFormatA2B10G10R10_UIntPack32
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR16G16B16A16_SInt, kFormatA2B10G10R10_SIntPack32, kFormatA2B10G10R10_SIntPack32, kFormatA2B10G10R10_SIntPack32, kTexFormatNone, kRTFormatCount, 3, 1, "RGB10A2", kFormatPropertyIntegerBit | kFormatPropertySignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},          //kFormatA2B10G10R10_SIntPack32

                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatA2B10G10R10_UNormPack32, kFormatA2R10G10B10_UNormPack32, kFormatA2R10G10B10_UNormPack32, kFormatA2R10G10B10_UNormPack32, kTexFormatNone, kRTFormatA2R10G10B10, 3, 1, "BGR10A2", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},        //kFormatA2R10G10B10_UNormPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatA2B10G10R10_UIntPack32, kFormatA2R10G10B10_UIntPack32, kFormatA2R10G10B10_UIntPack32, kFormatA2R10G10B10_UIntPack32, kTexFormatNone, kRTFormatCount, 3, 1, "BGR10A2", kFormatPropertyIntegerBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},               //kFormatA2R10G10B10_UIntPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatA2B10G10R10_SIntPack32, kFormatA2R10G10B10_SIntPack32, kFormatA2R10G10B10_SIntPack32, kFormatA2R10G10B10_SIntPack32, kTexFormatNone, kRTFormatCount, 3, 1, "BGR10A2", kFormatPropertyIntegerBit | kFormatPropertySignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit},                 //kFormatA2R10G10B10_SIntPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR16G16B16A16_UNorm, kFormatA2R10G10B10_XRSRGBPack32, kFormatA2R10G10B10_XRSRGBPack32, kFormatA2R10G10B10_XRSRGBPack32, kTexFormatNone, kRTFormatCount, 3, 1, "BGR10A2XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertySRGBBit | kFormatPropertyAlphaTestBit | kFormatPropertyXRBit},   //kFormatA2R10G10B10_XRSRGBPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR16G16B16A16_UNorm, kFormatA2R10G10B10_XRUNormPack32, kFormatA2R10G10B10_XRUNormPack32, kFormatA2R10G10B10_XRUNormPack32, kTexFormatNone, kRTFormatCount, 3, 1, "BGR10A2XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyAlphaTestBit | kFormatPropertyXRBit},                         //kFormatA2R10G10B10_XRUNormPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR16G16B16A16_UNorm, kFormatA10R10G10B10_XRSRGBPack32, kFormatR10G10B10_XRSRGBPack32, kFormatR10G10B10_XRSRGBPack32, kTexFormatNone, kRTFormatCount, 3, 0, "BGR10XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertySRGBBit | kFormatPropertyXRBit},        //kFormatR10G10B10_XRSRGBPack32
                    {4, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzle1, kFormatR16G16B16A16_UNorm, kFormatA10R10G10B10_XRUNormPack32, kFormatR10G10B10_XRUNormPack32, kFormatR10G10B10_XRUNormPack32, kTexFormatNone, kRTFormatBGR10_XR, 3, 0, "BGR10XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyXRBit},                           //kFormatR10G10B10_XRUNormPack32
                    {8, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR16G16B16A16_UNorm, kFormatA10R10G10B10_XRSRGBPack32, kFormatA10R10G10B10_XRSRGBPack32, kFormatA10R10G10B10_XRSRGBPack32, kTexFormatNone, kRTFormatCount, 3, 1, "BGRA10XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertySRGBBit | kFormatPropertyXRBit}, //kFormatA10R10G10B10_XRSRGBPack32
                    {8, 1, 1, 1, kFormatSwizzleB, kFormatSwizzleG, kFormatSwizzleR, kFormatSwizzleA, kFormatR16G16B16A16_UNorm, kFormatA10R10G10B10_XRUNormPack32, kFormatA10R10G10B10_XRUNormPack32, kFormatA10R10G10B10_XRUNormPack32, kTexFormatNone, kRTFormatBGRA10_XR, 3, 1, "BGRA10XR", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyXRBit},                   //kFormatA10R10G10B10_XRUNormPack32

                    {4, 1, 1, 1, kFormatSwizzleA, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatR8G8B8A8_SRGB, kFormatA8R8G8B8_SRGB, kFormatA8R8G8B8_UNorm, kFormatA8R8G8B8_SRGB, kTexFormatARGB32, kRTFormatBGRA32, 3, 1, "A8R8G8B8", kFormatPropertyNormBit | kFormatPropertySRGBBit | kFormatPropertyUnsignedBit},       //kFormatA8R8G8B8_SRGB
                    {4, 1, 1, 1, kFormatSwizzleA, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatR8G8B8A8_UNorm, kFormatA8R8G8B8_UNorm, kFormatA8R8G8B8_UNorm, kFormatA8R8G8B8_SRGB, kTexFormatARGB32, kRTFormatBGRA32, 3, 1, "A8R8G8B8", kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                              //kFormatA8R8G8B8_UNorm
                    {16, 1, 1, 1, kFormatSwizzleA, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatR32G32B32A32_SFloat, kFormatA32R32G32B32_SFloat, kFormatA32R32G32B32_SFloat, kFormatA32R32G32B32_SFloat, kTexFormatARGBFloat, kRTFormatCount, 3, 1, "ARGB Float", kFormatPropertyIEEE754Bit | kFormatPropertySignedBit},   //kFormatA32R32G32B32_SFloat

                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatD16_UNorm, kFormatD16_UNorm, kTexFormatNone, kRTFormatCount, 1, 0, "D16 UNorm", kFormatPropertyPackedBit | kFormatPropertyDepthBit | kFormatPropertyNormBit},                                                                 //kFormatD16_UNorm
                    {3, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatD24_UNorm, kFormatD24_UNorm, kTexFormatNone, kRTFormatCount, 1, 0, "D24 UNorm", kFormatPropertyPackedBit | kFormatPropertyDepthBit | kFormatPropertyNormBit},                                                                 //kFormatD24_UNorm
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatD24_UNorm_S8_UInt, kFormatD24_UNorm_S8_UInt, kTexFormatNone, kRTFormatCount, 2, 0, "D24 UNorm S8 UInt", kFormatPropertyPackedBit | kFormatPropertyDepthBit | kFormatPropertyStencilBit | kFormatPropertyNormBit},             //kFormatD24_UNorm_S8_UInt
                    {4, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatD32_SFloat, kFormatD32_SFloat, kTexFormatNone, kRTFormatCount, 1, 0, "D32 SFloat", kFormatPropertyPackedBit | kFormatPropertyDepthBit | kFormatPropertyIEEE754Bit},                                                           //kFormatD32_SFloat
                    {8, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatD32_SFloat_S8_Uint, kFormatD32_SFloat_S8_Uint, kTexFormatNone, kRTFormatCount, 2, 0, "D32 SFloat S8 UInt", kFormatPropertyPackedBit | kFormatPropertyDepthBit | kFormatPropertyStencilBit | kFormatPropertyIEEE754Bit},       //kFormatD32_SFloat_S8_Uint
                    {1, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatNone, kFormatNone, kFormatS8_Uint, kFormatS8_Uint, kTexFormatNone, kRTFormatCount, 1, 0, "S8 UInt", kFormatPropertyPackedBit | kFormatPropertyStencilBit | kFormatPropertyIntegerBit},                                                                  //kFormatS8_Uint

                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatRGBA_DXT5_SRGB, kFormatRGBA_DXT1_UNorm, kFormatRGBA_DXT1_SRGB, kTexFormatDXT1, kRTFormatCount, 3, 0, "DXT1", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyAlphaTestBit},        //kFormatRGBA_DXT1_SRGB
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatRGBA_DXT5_UNorm, kFormatRGBA_DXT1_UNorm, kFormatRGBA_DXT1_SRGB, kTexFormatDXT1, kRTFormatCount, 3, 0, "DXT1", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyAlphaTestBit},                               //kFormatRGBA_DXT1_UNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_DXT3_SRGB, kFormatRGBA_DXT3_UNorm, kFormatRGBA_DXT3_SRGB, kTexFormatDXT3, kRTFormatCount, 3, 1, "DXT3", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},       //kFormatRGBA_DXT3_SRGB
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_DXT3_UNorm, kFormatRGBA_DXT3_UNorm, kFormatRGBA_DXT3_SRGB, kTexFormatDXT3, kRTFormatCount, 3, 1, "DXT3", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                              //kFormatRGBA_DXT3_UNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_DXT5_SRGB, kFormatRGBA_DXT5_UNorm, kFormatRGBA_DXT5_SRGB, kTexFormatDXT5, kRTFormatCount, 3, 1, "DXT5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},       //kFormatRGBA_DXT5_SRGB
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_DXT5_UNorm, kFormatRGBA_DXT5_UNorm, kFormatRGBA_DXT5_SRGB, kTexFormatDXT5, kRTFormatCount, 3, 1, "DXT5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                              //kFormatRGBA_DXT5_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatNone, kFormatR_BC4_UNorm, kFormatR_BC4_UNorm, kTexFormatBC4, kRTFormatCount, 1, 0, "BC4", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                                   //kFormatR_BC4_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SNorm, kFormatNone, kFormatR_BC4_SNorm, kFormatR_BC4_SNorm, kTexFormatNone, kRTFormatCount, 1, 0, "BC4", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertySignedBit},                                                    //kFormatR_BC4_SNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatNone, kFormatRG_BC5_UNorm, kFormatRG_BC5_UNorm, kTexFormatBC5, kRTFormatCount, 2, 0, "BC5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                                //kFormatRG_BC5_UNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8B8A8_SNorm, kFormatNone, kFormatRG_BC5_SNorm, kFormatRG_BC5_SNorm, kTexFormatNone, kRTFormatCount, 2, 0, "BC5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertySignedBit},                                                 //kFormatRG_BC5_SNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_SFloat, kFormatNone, kFormatRGB_BC6H_UFloat, kFormatRGB_BC6H_UFloat, kTexFormatBC6H, kRTFormatCount, 3, 0, "BC6H", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit},                                //kFormatRGB_BC6H_UFloat
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR16G16B16A16_SFloat, kFormatNone, kFormatRGB_BC6H_SFloat, kFormatRGB_BC6H_SFloat, kTexFormatNone, kRTFormatCount, 3, 0, "BC6H", kFormatPropertyCompressedBit | kFormatPropertySignedBit | kFormatPropertyIEEE754Bit},                                  //kFormatRGB_BC6H_SFloat
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_BC7_SRGB, kFormatRGBA_BC7_UNorm, kFormatRGBA_BC7_SRGB, kTexFormatBC7, kRTFormatCount, 3, 1, "BC7", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},            //kFormatRGBA_BC7_SRGB
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_BC7_UNorm, kFormatRGBA_BC7_UNorm, kFormatRGBA_BC7_SRGB, kTexFormatBC7, kRTFormatCount, 3, 1, "BC7", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                   //kFormatRGBA_BC7_UNorm

                    {8, 8, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatRGBA_PVRTC_2Bpp_SRGB, kFormatRGB_PVRTC_2Bpp_UNorm, kFormatRGB_PVRTC_2Bpp_SRGB, kTexFormatPVRTC_RGB2, kRTFormatCount, 3, 0, "PVRTC 2BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},     //kFormatRGB_PVRTC_2Bpp_SRGB
                    {8, 8, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatRGBA_PVRTC_2Bpp_UNorm, kFormatRGB_PVRTC_2Bpp_UNorm, kFormatRGB_PVRTC_2Bpp_SRGB, kTexFormatPVRTC_RGB2, kRTFormatCount, 3, 0, "PVRTC 2BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},                            //kFormatRGB_PVRTC_2Bpp_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatRGBA_PVRTC_4Bpp_SRGB, kFormatRGB_PVRTC_4Bpp_UNorm, kFormatRGB_PVRTC_4Bpp_SRGB, kTexFormatPVRTC_RGB4, kRTFormatCount, 3, 0, "PVRTC 4BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},      //kFormatRGB_PVRTC_4Bpp_SRGB
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatRGBA_PVRTC_4Bpp_UNorm, kFormatRGB_PVRTC_4Bpp_UNorm, kFormatRGB_PVRTC_4Bpp_SRGB, kTexFormatPVRTC_RGB4, kRTFormatCount, 3, 0, "PVRTC 4BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},                             //kFormatRGB_PVRTC_4Bpp_UNorm
                    {8, 8, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_PVRTC_2Bpp_SRGB, kFormatRGBA_PVRTC_2Bpp_UNorm, kFormatRGBA_PVRTC_2Bpp_SRGB, kTexFormatPVRTC_RGBA2, kRTFormatCount, 3, 1, "PVRTC 2BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},  //kFormatRGBA_PVRTC_2Bpp_SRGB
                    {8, 8, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_PVRTC_2Bpp_UNorm, kFormatRGBA_PVRTC_2Bpp_UNorm, kFormatRGBA_PVRTC_2Bpp_SRGB, kTexFormatPVRTC_RGBA2, kRTFormatCount, 3, 1, "PVRTC 2BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},                         //kFormatRGBA_PVRTC_2Bpp_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_PVRTC_4Bpp_SRGB, kFormatRGBA_PVRTC_4Bpp_UNorm, kFormatRGBA_PVRTC_4Bpp_SRGB, kTexFormatPVRTC_RGBA4, kRTFormatCount, 3, 1, "PVRTC 4BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},   //kFormatRGBA_PVRTC_4Bpp_SRGB
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_PVRTC_4Bpp_UNorm, kFormatRGBA_PVRTC_4Bpp_UNorm, kFormatRGBA_PVRTC_4Bpp_SRGB, kTexFormatPVRTC_RGBA4, kRTFormatCount, 3, 1, "PVRTC 4BPP", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyBlockSizeIsMinTextureMipSizeBit},                          //kFormatRGBA_PVRTC_4Bpp_UNorm

                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatRGBA_ETC2_UNorm, kFormatRGB_ETC_UNorm, kFormatRGB_ETC_UNorm, kTexFormatETC_RGB4, kRTFormatCount, 3, 0, "ETC", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                                  //kFormatRGB_ETC_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_SRGB, kFormatRGBA_ETC2_SRGB, kFormatRGB_ETC2_UNorm, kFormatRGB_ETC2_SRGB, kTexFormatETC2_RGB, kRTFormatCount, 3, 0, "ETC2", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},                         //kFormatRGB_ETC2_SRGB
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzle1, kFormatR8G8B8A8_UNorm, kFormatRGBA_ETC2_UNorm, kFormatRGB_ETC2_UNorm, kFormatRGB_ETC2_SRGB, kTexFormatETC2_RGB, kRTFormatCount, 3, 0, "ETC2", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                                //kFormatRGB_ETC2_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ETC2_SRGB, kFormatRGB_A1_ETC2_UNorm, kFormatRGB_A1_ETC2_SRGB, kTexFormatETC2_RGBA1, kRTFormatCount, 3, 1, "ETC2 punchthrough", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit | kFormatPropertyAlphaTestBit},    //kFormatRGB_A1_ETC2_SRGB
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ETC2_UNorm, kFormatRGB_A1_ETC2_UNorm, kFormatRGB_A1_ETC2_SRGB, kTexFormatETC2_RGBA1, kRTFormatCount, 3, 1, "ETC2 punchthrough", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyAlphaTestBit},                           //kFormatRGB_A1_ETC2_UNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ETC2_SRGB, kFormatRGBA_ETC2_UNorm, kFormatRGBA_ETC2_SRGB, kTexFormatETC2_RGBA8, kRTFormatCount, 3, 1, "ETC2", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},                    //kFormatRGBA_ETC2_SRGB
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ETC2_UNorm, kFormatRGBA_ETC2_UNorm, kFormatRGBA_ETC2_SRGB, kTexFormatETC2_RGBA8, kRTFormatCount, 3, 1, "ETC2", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                           //kFormatRGBA_ETC2_UNorm

                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8_UNorm, kFormatNone, kFormatR_EAC_UNorm, kFormatR_EAC_UNorm, kTexFormatEAC_R, kRTFormatCount, 1, 0, "EAC", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},              //kFormatR_EAC_UNorm
                    {8, 4, 4, 1, kFormatSwizzleR, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8_SNorm, kFormatNone, kFormatR_EAC_SNorm, kFormatR_EAC_SNorm, kTexFormatEAC_R_SIGNED, kRTFormatCount, 1, 0, "EAC", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertySignedBit},       //kFormatR_EAC_SNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8_UNorm, kFormatNone, kFormatRG_EAC_UNorm, kFormatRG_EAC_UNorm, kTexFormatEAC_RG, kRTFormatCount, 2, 0, "EAC", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},        //kFormatRG_EAC_UNorm
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzle0, kFormatSwizzle1, kFormatR8G8_SNorm, kFormatNone, kFormatRG_EAC_SNorm, kFormatRG_EAC_SNorm, kTexFormatEAC_RG_SIGNED, kRTFormatCount, 2, 0, "EAC", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertySignedBit},   //kFormatRG_EAC_SNorm

                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC4X4_SRGB, kFormatRGBA_ASTC4X4_UNorm, kFormatRGBA_ASTC4X4_SRGB, kTexFormatASTC_4x4, kRTFormatCount, 3, 1, "ASTC4X4", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},               //kFormatRGBA_ASTC4X4_SRGB
                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC4X4_UNorm, kFormatRGBA_ASTC4X4_UNorm, kFormatRGBA_ASTC4X4_SRGB, kTexFormatASTC_4x4, kRTFormatCount, 3, 1, "ASTC4X4", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                      //kFormatRGBA_ASTC4X4_UNorm
                    {16, 5, 5, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC5X5_SRGB, kFormatRGBA_ASTC5X5_UNorm, kFormatRGBA_ASTC5X5_SRGB, kTexFormatASTC_5x5, kRTFormatCount, 3, 1, "ASTC5X5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},               //kFormatRGBA_ASTC5X5_SRGB
                    {16, 5, 5, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC5X5_UNorm, kFormatRGBA_ASTC5X5_UNorm, kFormatRGBA_ASTC5X5_SRGB, kTexFormatASTC_5x5, kRTFormatCount, 3, 1, "ASTC5X5", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                      //kFormatRGBA_ASTC5X5_UNorm
                    {16, 6, 6, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC6X6_SRGB, kFormatRGBA_ASTC6X6_UNorm, kFormatRGBA_ASTC6X6_SRGB, kTexFormatASTC_6x6, kRTFormatCount, 3, 1, "ASTC6X6", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},               //kFormatRGBA_ASTC6X6_SRGB
                    {16, 6, 6, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC6X6_UNorm, kFormatRGBA_ASTC6X6_UNorm, kFormatRGBA_ASTC6X6_SRGB, kTexFormatASTC_6x6, kRTFormatCount, 3, 1, "ASTC6X6", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                      //kFormatRGBA_ASTC6X6_UNorm
                    {16, 8, 8, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC8X8_SRGB, kFormatRGBA_ASTC8X8_UNorm, kFormatRGBA_ASTC8X8_SRGB, kTexFormatASTC_8x8, kRTFormatCount, 3, 1, "ASTC8X8", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},               //kFormatRGBA_ASTC8X8_SRGB
                    {16, 8, 8, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC8X8_UNorm, kFormatRGBA_ASTC8X8_UNorm, kFormatRGBA_ASTC8X8_SRGB, kTexFormatASTC_8x8, kRTFormatCount, 3, 1, "ASTC8X8", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                                      //kFormatRGBA_ASTC8X8_UNorm
                    {16, 10, 10, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC10X10_SRGB, kFormatRGBA_ASTC10X10_UNorm, kFormatRGBA_ASTC10X10_SRGB, kTexFormatASTC_10x10, kRTFormatCount, 3, 1, "ASTC10X10", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},   //kFormatRGBA_ASTC10X10_SRGB
                    {16, 10, 10, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC10X10_UNorm, kFormatRGBA_ASTC10X10_UNorm, kFormatRGBA_ASTC10X10_SRGB, kTexFormatASTC_10x10, kRTFormatCount, 3, 1, "ASTC10X10", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                          //kFormatRGBA_ASTC10X10_UNorm
                    {16, 12, 12, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_SRGB, kFormatRGBA_ASTC12X12_SRGB, kFormatRGBA_ASTC12X12_UNorm, kFormatRGBA_ASTC12X12_SRGB, kTexFormatASTC_12x12, kRTFormatCount, 3, 1, "ASTC12X12", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertySRGBBit},   //kFormatRGBA_ASTC12X12_SRGB
                    {16, 12, 12, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR8G8B8A8_UNorm, kFormatRGBA_ASTC12X12_UNorm, kFormatRGBA_ASTC12X12_UNorm, kFormatRGBA_ASTC12X12_SRGB, kTexFormatASTC_12x12, kRTFormatCount, 3, 1, "ASTC12X12", kFormatPropertyCompressedBit | kFormatPropertyNormBit | kFormatPropertyUnsignedBit},                          //kFormatRGBA_ASTC12X12_UNorm

                    {2, 2, 2, 2, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatNone, kFormatNone, kFormatYUV2, kFormatYUV2, kTexFormatYUY2, kRTFormatCount, 3, 0, "YUV", 0},          //kFormatYUV2,

                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR4G4B4A4_UNormPack16, kFormatNone, kFormatDepthAuto, kFormatDepthAuto, kTexFormatRGBA4444, kRTFormatDepth, 3, 1, "Depth", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyDepthBit},        //kFormatDepthAuto, Auto formats are only directly used with UNITY_WEBGL and interpreted as RGBA4
                    {2, 1, 1, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatR4G4B4A4_UNormPack16, kFormatNone, kFormatShadowAuto, kFormatShadowAuto, kTexFormatRGBA4444, kRTFormatShadowMap, 3, 1, "Shadow", kFormatPropertyNormBit | kFormatPropertyUnsignedBit | kFormatPropertyPackedBit | kFormatPropertyDepthBit},  //kFormatShadowAuto, Auto formats are only directly used with UNITY_WEBGL and interpreted as RGBA4

                    {2, 2, 2, 2, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle0, kFormatSwizzle1, kFormatNone, kFormatNone, kFormatVideoAuto, kFormatVideoAuto, kTexFormatNone, kRTFormatVideo, 3, 0, "Video", 0},         //kFormatVideoAuto,

                    {16, 4, 4, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC4X4_UFloat, kFormatRGBA_ASTC4X4_UFloat, kFormatRGBA_ASTC4X4_UFloat, kTexFormatASTC_HDR_4x4, kRTFormatCount, 3, 1, "ASTC4X4", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit},       //kFormatRGBA_ASTC4X4_UFloat
                    {16, 5, 5, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC5X5_UFloat, kFormatRGBA_ASTC5X5_UFloat, kFormatRGBA_ASTC5X5_UFloat, kTexFormatASTC_HDR_5x5, kRTFormatCount, 3, 1, "ASTC5X5", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit},       //kFormatRGBA_ASTC5X5_UFloat
                    {16, 6, 6, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC6X6_UFloat, kFormatRGBA_ASTC6X6_UFloat, kFormatRGBA_ASTC6X6_UFloat, kTexFormatASTC_HDR_6x6, kRTFormatCount, 3, 1, "ASTC6X6", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit},       //kFormatRGBA_ASTC6X6_UFloat
                    {16, 8, 8, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC8X8_UFloat, kFormatRGBA_ASTC8X8_UFloat, kFormatRGBA_ASTC8X8_UFloat, kTexFormatASTC_HDR_8x8, kRTFormatCount, 3, 1, "ASTC8X8", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit},       //kFormatRGBA_ASTC8X8_UFloat
                    {16, 10, 10, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC10X10_UFloat, kFormatRGBA_ASTC10X10_UFloat, kFormatRGBA_ASTC10X10_UFloat, kTexFormatASTC_HDR_10x10, kRTFormatCount, 3, 1, "ASTC10X10", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit}, //kFormatRGBA_ASTC10X10_UFloat
                    {16, 12, 12, 1, kFormatSwizzleR, kFormatSwizzleG, kFormatSwizzleB, kFormatSwizzleA, kFormatE5B9G9R9_UFloatPack32, kFormatRGBA_ASTC12X12_UFloat, kFormatRGBA_ASTC12X12_UFloat, kFormatRGBA_ASTC12X12_UFloat, kTexFormatASTC_HDR_12x12, kRTFormatCount, 3, 1, "ASTC12X12", kFormatPropertyCompressedBit | kFormatPropertyUnsignedBit | kFormatPropertyIEEE754Bit}, //kFormatRGBA_ASTC12X12_UFloat
            };
    CompileTimeAssertArraySize(s_FormatDescTable, kGraphicsFormatCount);
} // namespace

bool IsValidFormat(GraphicsFormat format)
{
    return format >= kFormatFirst && format <= kFormatLast;
}

UInt32 GetBlockSize(GraphicsFormat format)
{
    return GetDesc(format).blockSize;
}

bool IsAlphaOnlyFormat(GraphicsFormat format)
{
    return GetAlphaComponentCount(format) == 1 && GetColorComponentCount(format) == 0;
}

bool IsAlphaTestFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyAlphaTestBit) != 0;
}

bool IsCompressedFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyCompressedBit) != 0;
}

bool IsPackedFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyPackedBit) != 0;
}

UInt32 GetColorComponentCount(GraphicsFormat format)
{
    return GetDesc(format).colorComponents;
}

bool Is16BitPackedFormat(GraphicsFormat format)
{
    return GetColorComponentCount(format) >= 3 && GetBlockSize(format) == 2 && format < kFormatYUV2;
}

UInt32 GetAlphaComponentCount(GraphicsFormat format)
{
    return GetDesc(format).alphaComponents;
}

const FormatDesc& GetDesc(GraphicsFormat format)
{
    return s_FormatDescTable[IsValidFormat(format) ? format : kFormatNone];
}

static const char* s_SwizzleNameTable[] =
        {
                "R",    // kFormatSwizzleR
                "G",    // kFormatSwizzleG
                "B",    // kFormatSwizzleB
                "A",    // kFormatSwizzleA
                "",     // kFormatSwizzle0
                ""      // kFormatSwizzle1
        };
CompileTimeAssertArraySize(s_SwizzleNameTable, kFormatSwizzleMax + 1);

bool IsStencilFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyStencilBit) != 0;
}

bool IsDepthFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyDepthBit) != 0;
}

bool IsSRGBFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertySRGBBit) != 0;
}

bool IsIEEE754Format(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyIEEE754Bit) != 0;
}

bool IsUnsignedFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyUnsignedBit) != 0;
}

bool IsSignedFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertySignedBit) != 0;
}

bool IsNormFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyNormBit) != 0;
}

bool IsUNormFormat(GraphicsFormat format)
{
    return IsNormFormat(format) && IsUnsignedFormat(format);
}

bool IsIntegerFormat(GraphicsFormat format)
{
    return (GetDesc(format).flags & kFormatPropertyIntegerBit) != 0;
}

bool IsSIntFormat(GraphicsFormat format)
{
    return IsIntegerFormat(format) && IsSignedFormat(format);
}

TextureFormat GetTextureFormat(GraphicsFormat format)
{
    return GetDesc(format).textureFormat;
}

RenderTextureFormat GetRenderTextureFormat(GraphicsFormat format)
{
    RenderTextureFormat renderTextureFormat = GetDesc(format).renderTextureFormat;
    return renderTextureFormat;
}

UInt32 GetComponentCount(GraphicsFormat format)
{
    return GetDesc(format).colorComponents + GetDesc(format).alphaComponents;
}
std::string GetFormatString(GraphicsFormat format)
{
    if (format == kFormatNone)
        return "None";
    else if (format == kFormatYUV2 || format == kFormatVideoAuto)
        return "YUV";
    else if (format == kFormatL8_UNorm)
        return "Luminance8 UNorm";

    const FormatDesc& desc = GetDesc(format);

    std::string result;

    if (IsPackedFormat(format))
        result += desc.name;
    else if (IsAlphaOnlyFormat(format))
        result += "Alpha";
    else
    {
        result += s_SwizzleNameTable[desc.swizzleR];
        result += s_SwizzleNameTable[desc.swizzleG];
        result += s_SwizzleNameTable[desc.swizzleB];
        result += s_SwizzleNameTable[desc.swizzleA];
    }

    if (IsCompressedFormat(format))
    {
        result += " Compressed ";
        result += desc.name;
    }
    else if (!IsPackedFormat(format))
    {
        result += ::ToString(static_cast<int>(GetBlockSize(format) * 8 / GetComponentCount(format)));
    }

    if (!IsStencilFormat(format) && !IsDepthFormat(format))
    {
        if (IsSRGBFormat(format))
            result += " sRGB";
        else if (IsIEEE754Format(format) && IsUnsignedFormat(format))
            result += " UFloat";
        else if (IsIEEE754Format(format) && IsSignedFormat(format))
            result += " SFloat";
        else if (IsNormFormat(format) && IsUnsignedFormat(format))
            result += " UNorm";
        else if (IsNormFormat(format) && IsSignedFormat(format))
            result += " SNorm";
        else if (IsIntegerFormat(format) && IsUnsignedFormat(format))
            result += " UInt";
        else if (IsIntegerFormat(format) && IsSignedFormat(format))
            result += " SInt";
    }

    return result;
}

bool HasAlphaChannel(GraphicsFormat format)
{
    return GetAlphaComponentCount(format) != 0;
}
