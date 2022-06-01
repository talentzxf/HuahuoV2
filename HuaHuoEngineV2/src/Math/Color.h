#pragma once

#include "Serialize/SerializeUtility.h"
#include "Serialize/SerializationMetaFlags.h"
#include <algorithm>
#include "Utilities/Utility.h"
#include "FloatConversion.h"
#include "Serialize/SwapEndianBytes.h"
//#include "Runtime/Scripting/BindingsDefs.h"

class Vector4f;

class ColorRGBAf
{
public:
    float   r, g, b, a;

    // DEFINE_GET_TYPESTRING_IS_ANIMATION_CHANNEL(ColorRGBA)

    ColorRGBAf() {}

    ColorRGBAf(float inR, float inG, float inB, float inA = 1.0F) : r(inR), g(inG), b(inB), a(inA) {}
    explicit ColorRGBAf(const float* c) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {}

    template<class TransferFunction>
    void Transfer(TransferFunction& transfer);

    void Set(float inR, float inG, float inB, float inA) {r = inR; g = inG; b = inB; a = inA; }

    void SetHex(UInt32 hex)
    {
        Set(float(hex >> 24) / 255.0f,
            float((hex >> 16) & 255) / 255.0f,
            float((hex >> 8) & 255) / 255.0f,
            float(hex & 255) / 255.0f);
    }

    void SetHSV(float H, float S, float V);
    void CalculateHSV(float& H, float& S, float& V);

    UInt32 GetHex() const
    {
        UInt32 hex = (NormalizedToByte(r) << 24) | (NormalizedToByte(g) << 16) | (NormalizedToByte(b) << 8) | NormalizedToByte(a);
        return hex;
    }

    ColorRGBAf Clamp01() const
    {
        return ColorRGBAf(clamp01(r), clamp01(g), clamp01(b), clamp01(a));
    }

    float AverageRGB() const {return (r + g + b) * (1.0F / 3.0F); }
    float GreyScaleValue() const { return r * 0.30f + g * 0.59f  + b * 0.11f; }

    ColorRGBAf& operator=(const ColorRGBAf& in) { Set(in.r, in.g, in.b, in.a); return *this; }

    bool Equals(const ColorRGBAf& inRGB) const
    {
        return (r == inRGB.r && g == inRGB.g && b == inRGB.b && a == inRGB.a);
    }

    bool NotEquals(const ColorRGBAf& inRGB) const
    {
        return (r != inRGB.r || g != inRGB.g || b != inRGB.b || a != inRGB.a);
    }

    float* GetPtr()                { return &r; }
    const float* GetPtr() const    { return &r; }

    Vector4f& GetVector4f()                {return *reinterpret_cast<Vector4f*>(&r); }
    const Vector4f& GetVector4f() const    {return *reinterpret_cast<const Vector4f*>(&r); }

    ColorRGBAf& operator+=(const ColorRGBAf &inRGBA)
    {
        r += inRGBA.r; g += inRGBA.g; b += inRGBA.b; a += inRGBA.a;
        return *this;
    }

    ColorRGBAf& operator-=(const ColorRGBAf &inRGBA)
    {
        r -= inRGBA.r; g -= inRGBA.g; b -= inRGBA.b; a -= inRGBA.a;
        return *this;
    }

    ColorRGBAf& operator*=(const ColorRGBAf &inRGBA)
    {
        r *= inRGBA.r; g *= inRGBA.g; b *= inRGBA.b; a *= inRGBA.a;
        return *this;
    }

    inline ColorRGBAf& operator*=(float inScale)
    {
        r *= inScale; g *= inScale; b *= inScale; a *= inScale;
        return *this;
    }

private:
    // intentionally undefined
    bool operator==(const ColorRGBAf& inRGB) const;
    bool operator!=(const ColorRGBAf& inRGB) const;
};

// BIND_MANAGED_TYPE_NAME(ColorRGBAf, UnityEngine_Color);

inline ColorRGBAf operator+(const ColorRGBAf& inC0, const ColorRGBAf& inC1)
{
    return ColorRGBAf(inC0.r + inC1.r, inC0.g + inC1.g, inC0.b + inC1.b, inC0.a + inC1.a);
}

inline ColorRGBAf operator-(const ColorRGBAf& inC0, const ColorRGBAf& inC1)
{
    return ColorRGBAf(inC0.r - inC1.r, inC0.g - inC1.g, inC0.b - inC1.b, inC0.a - inC1.a);
}

inline ColorRGBAf operator*(const ColorRGBAf& inC0, const ColorRGBAf& inC1)
{
    return ColorRGBAf(inC0.r * inC1.r, inC0.g * inC1.g, inC0.b * inC1.b, inC0.a * inC1.a);
}

inline ColorRGBAf operator*(float inScale, const ColorRGBAf& inC0)
{
    return ColorRGBAf(inC0.r * inScale, inC0.g * inScale, inC0.b * inScale, inC0.a * inScale);
}

inline ColorRGBAf operator*(const ColorRGBAf& inC0, float inScale)
{
    return ColorRGBAf(inC0.r * inScale, inC0.g * inScale, inC0.b * inScale, inC0.a * inScale);
}

inline ColorRGBAf Lerp(const ColorRGBAf& c0, const ColorRGBAf& c1, float t)
{
    return (1.0f - t) * c0 + t * c1;
}

//-----------------------------------------------------------------------------

// Only in use for ColorRGBAf <-> ColorRGBA64 conversions
class alignas(8)ColorRGBA64
{
public:
    UInt16   r, g, b, a;

    ColorRGBA64() {}

    ColorRGBA64(UInt16 inR, UInt16 inG, UInt16 inB, UInt16 inA) { r = inR; g = inG; b = inB; a = inA; }
    ColorRGBA64(UInt64 c) { *(UInt64*)this = c; }
    void Set(UInt16 inR, UInt16 inG, UInt16 inB, UInt16 inA) { r = inR; g = inG; b = inB; a = inA; }

    ColorRGBA64(const ColorRGBAf& c) { Set(c); }

    operator ColorRGBAf() const
    {
        return ColorRGBAf(UnsignedWordToNormalized(r), UnsignedWordToNormalized(g), UnsignedWordToNormalized(b), UnsignedWordToNormalized(a));
    }

    UInt64 AsUInt64() const { return *(UInt64*)this; }

    void operator=(const ColorRGBAf& c)
    {
        Set(c);
    }

    void Set(const ColorRGBAf& c)
    {
        r = NormalizedToUnsignedWord(c.r);
        g = NormalizedToUnsignedWord(c.g);
        b = NormalizedToUnsignedWord(c.b);
        a = NormalizedToUnsignedWord(c.a);
    }

    void SetRGBA64(const ColorRGBA64& c)
    {
        r = c.r;
        g = c.g;
        b = c.b;
        a = c.a;
    }
};

//-----------------------------------------------------------------------------


// Data-only struct for icalls
// C++ classes below a certain size (8bytes?) are passed incorrectly for icalls
struct ColorRGBA32Icall
{
public:
    UInt8 r, g, b, a;
};

//BIND_MANAGED_TYPE_NAME(ColorRGBA32Icall, UnityEngine_Color32)

inline void ColorRGBAf::SetHSV(float H, float S, float V)
{
    float C = V * S;
    float X = C * (1 - Abs(fmodf(H / 60.0f, 2.0f) - 1));

    if (H < 60)
        Set(C, X, 0, 1);
    else if (H < 120)
        Set(X, C, 0, 1);
    else if (H < 180)
        Set(0, C, X, 1);
    else if (H < 240)
        Set(0, X, C, 1);
    else if (H < 300)
        Set(X, 0, C, 1);
    else
        Set(C, 0, X, 1);

    float m = V - C;
    r += m;
    g += m;
    b += m;
}

inline void RGBToHSVHelper(float offset, float dominantcolor, float colorone, float colortwo, float& H, float& S, float& V)
{
    V = dominantcolor;
    //we need to find out which is the minimum color
    if (V != 0)
    {
        //we check which color is smallest
        float smallest = 0;
        if (colorone > colortwo)
            smallest = colortwo;
        else
            smallest = colorone;

        float diff = V - smallest;

        //if the two values are not the same, we compute the like this
        if (diff != 0)
        {
            //S = max-min/max
            S = diff / V;
            //H = hue is offset by X, and is the difference between the two smallest colors
            H = offset + ((colorone - colortwo) / diff);
        }
        else
        {
            //S = 0 when the difference is zero
            S = 0;
            //H = 4 + (R-G) hue is offset by 4 when blue, and is the difference between the two smallest colors
            H = offset + (colorone - colortwo);
        }

        H /= 6;

        //conversion values
        if (H < 0)
            H += 1.0f;
    }
    else
    {
        S = 0;
        H = 0;
    }
}

inline void ColorRGBAf::CalculateHSV(float& H, float& S, float& V)
{
    // when blue is highest valued
    if ((b > g) && (b > r))
        RGBToHSVHelper(4.0f, b, r, g, H, S, V);
    //when green is highest valued
    else if (g > r)
        RGBToHSVHelper(2.0f, g, b, r, H, S, V);
    //when red is highest valued
    else
        RGBToHSVHelper(0.0f, r, g, b, H, S, V);
}

class alignas(4)ColorRGBA32
{
public:

    UInt8   r, g, b, a;

    //DEFINE_GET_TYPESTRING_IS_ANIMATION_CHANNEL(ColorRGBA)

    ColorRGBA32()                              {}

    ColorRGBA32(UInt8 inR, UInt8 inG, UInt8 inB, UInt8 inA)        { r = inR; g = inG; b = inB; a = inA; }
    ColorRGBA32(UInt32 c)                      { *(UInt32*)this = c; }
    ColorRGBA32(const ColorRGBA32Icall & c) { r = c.r; g = c.g; b = c.b; a = c.a; }
    ColorRGBA32 operator=(const ColorRGBA32& c)   { *(UInt32*)this = *((UInt32*)&c); return *this; }
    ColorRGBA32 operator=(const ColorRGBA32Icall &c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
    void Set(UInt8 inR, UInt8 inG, UInt8 inB, UInt8 inA)       { r = inR; g = inG; b = inB; a = inA; }

    ColorRGBA32(const ColorRGBAf& c) { Set(c); }

    operator ColorRGBAf() const
    {
        return ColorRGBAf(ByteToNormalized(r), ByteToNormalized(g), ByteToNormalized(b), ByteToNormalized(a));
    }

    inline operator ColorRGBA32Icall() const
    {
        ColorRGBA32Icall c;
        c.r = r;
        c.g = g;
        c.b = b;
        c.a = a;
        return c;
    }

    UInt32 AsUInt32() const { return *(UInt32*)this; }

    void operator=(const ColorRGBAf& c)
    {
        Set(c);
    }

    UInt32 GetUInt32() { return *(UInt32*)this; }

    void Set(const ColorRGBAf& c)
    {
        r = NormalizedToByte(c.r);
        g = NormalizedToByte(c.g);
        b = NormalizedToByte(c.b);
        a = NormalizedToByte(c.a);
    }

    void SetRGBA32(const ColorRGBA32& c)
    {
        r = c.r;
        g = c.g;
        b = c.b;
        a = c.a;
    }

    template<class TransferFunction>
    void Transfer(TransferFunction& transfer)
    {
        transfer.SetVersion(2);
        UInt32 &c = *reinterpret_cast<UInt32*>(this);

        transfer.Transfer(c, "rgba", kHideInEditorMask);

        // When transferring colors we shouldn't swap bytes.
        // UInt32 already convert endianess by default so we convert it two times to keep it the same :)
        if (transfer.ConvertEndianess() && transfer.IsReading())
            SwapEndianBytes(c);
    }

    UInt8& operator[](long i) { DebugAssertMsg(i >= 0 && i <= 3, "Out of bounds color channel access"); return GetPtr()[i]; }
    const UInt8& operator[](long i) const { DebugAssertMsg(i >= 0 && i <= 3, "Out of bounds color channel access"); return GetPtr()[i]; }

    bool operator==(const ColorRGBA32& inRGB) const
    {
        return (r == inRGB.r && g == inRGB.g && b == inRGB.b && a == inRGB.a) ? true : false;
    }

    bool operator!=(const ColorRGBA32& inRGB) const
    {
        return (r != inRGB.r || g != inRGB.g || b != inRGB.b || a != inRGB.a) ? true : false;
    }

    inline bool operator==(const ColorRGBA32Icall &c) const
    {
        return r == c.r && g == c.g && b == c.b && a == c.a;
    }

    inline bool operator!=(const ColorRGBA32Icall &c) const
    {
        return r != c.r || g != c.g || b != c.b || a != c.a;
    }

    UInt8* GetPtr()        {return &r; }
    const UInt8* GetPtr() const {return &r; }

    inline ColorRGBA32 operator*(int scale) const
    {
        //Assert  (scale >= 0 && scale <= 255);
        scale += 1;
        const UInt32& u = reinterpret_cast<const UInt32&>(*this);
        UInt32 lsb = (((u & 0x00ff00ff) * scale) >> 8) & 0x00ff00ff;
        UInt32 msb = (((u & 0xff00ff00) >> 8) * scale) & 0xff00ff00;
        lsb |= msb;
        return ColorRGBA32(lsb);
    }

    inline void operator*=(const ColorRGBA32& inC1)
    {
#if 0
        r = (r * inC1.r) / 255;
        g = (g * inC1.g) / 255;
        b = (b * inC1.b) / 255;
        a = (a * inC1.a) / 255;
#else // This is much faster, but doesn't guarantee 100% matching result (basically color values van vary 1/255 but not at ends, check out unit test in cpp file).

        UInt32& u = reinterpret_cast<UInt32&>(*this);
        const UInt32& v = reinterpret_cast<const UInt32&>(inC1);
        UInt32 result = (((u & 0x000000ff) * ((v & 0x000000ff) + 1)) >> 8) & 0x000000ff;
        result |= (((u & 0x0000ff00) >> 8) * (((v & 0x0000ff00) >> 8) + 1)) & 0x0000ff00;
        result |= (((u & 0x00ff0000) * (((v & 0x00ff0000) >> 16) + 1)) >> 8) & 0x00ff0000;
        result |= (((u & 0xff000000) >> 8) * (((v & 0xff000000) >> 24) + 1)) & 0xff000000;
        u = result;
#endif
    }

    inline ColorRGBA32 SwizzleToBGRA() const { return ColorRGBA32(b, g, r, a); }
    inline ColorRGBA32 SwizzleToBGR() const { return ColorRGBA32(b, g, r, 255); }
    inline ColorRGBA32 SwizzleToARGB() const { return ColorRGBA32(a, r, g, b); }
    inline ColorRGBA32 UnswizzleARGB() const { return ColorRGBA32(g, b, a, r); }
};


struct OpColorRGBA32ToUInt32
{
    typedef UInt32 result_type;
    UInt32 operator()(ColorRGBA32 const& arg) const { return arg.AsUInt32(); }
};

inline ColorRGBA32 operator+(const ColorRGBA32& inC0, const ColorRGBA32& inC1)
{
    return ColorRGBA32(std::min<int>(inC0.r + inC1.r, 255),
        std::min<int>(inC0.g + inC1.g, 255),
        std::min<int>(inC0.b + inC1.b, 255),
        std::min<int>(inC0.a + inC1.a, 255));
}

inline ColorRGBA32 operator*(const ColorRGBA32& inC0, const ColorRGBA32& inC1)
{
#if 0
    return ColorRGBA32((inC0.r * inC1.r) / 255,
        (inC0.g * inC1.g) / 255,
        (inC0.b * inC1.b) / 255,
        (inC0.a * inC1.a) / 255);
#else
    // This is much faster, but doesn't guarantee 100% matching result (basically color values van vary 1/255 but not at ends, check out unit test in cpp file).
    const UInt32& u = reinterpret_cast<const UInt32&>(inC0);
    const UInt32& v = reinterpret_cast<const UInt32&>(inC1);
    UInt32 result = (((u & 0x000000ff) * ((v & 0x000000ff) + 1)) >> 8) & 0x000000ff;
    result |= (((u & 0x0000ff00) >> 8) * (((v & 0x0000ff00) >> 8) + 1)) & 0x0000ff00;
    result |= (((u & 0x00ff0000) * (((v & 0x00ff0000) >> 16) + 1)) >> 8) & 0x00ff0000;
    result |= (((u & 0xff000000) >> 8) * (((v & 0xff000000) >> 24) + 1)) & 0xff000000;
    return ColorRGBA32(result);
#endif
}

inline bool operator==(const ColorRGBA32Icall& ci, const ColorRGBA32 &c) { return ci.r == c.r && ci.g == c.g && ci.b == c.b && ci.a == c.a; }
inline bool operator!=(const ColorRGBA32Icall& ci, const ColorRGBA32 &c) { return ci.r != c.r || ci.g != c.g || ci.b != c.b || ci.a != c.a; }

inline ColorRGBA32 Lerp(const ColorRGBA32& c0, const ColorRGBA32& c1, int scale)
{
    //Assert (scale >= 0 && scale <= 255);
    const UInt32& u0 = reinterpret_cast<const UInt32&>(c0);
    const UInt32& u1 = reinterpret_cast<const UInt32&>(c1);
    UInt32 vx = u0 & 0x00ff00ff;
    UInt32 rb = (vx + ((((u1 & 0x00ff00ff) - vx) * scale) >> 8)) & 0x00ff00ff;
    vx = u0 & 0xff00ff00;
    return ColorRGBA32(rb | ((vx + ((((u1 >> 8) & 0x00ff00ff) - (vx >> 8)) * scale)) & 0xff00ff00));
}

template<class TransferFunction>
void ColorRGBAf::Transfer(TransferFunction& transfer)
{
    transfer.AddMetaFlag(kTransferUsingFlowMappingStyle);
    transfer.Transfer(r, "r", kHideInEditorMask);
    transfer.Transfer(g, "g", kHideInEditorMask);
    transfer.Transfer(b, "b", kHideInEditorMask);
    transfer.Transfer(a, "a", kHideInEditorMask);
}

#if ENABLE_UNIT_TESTS
#include "Runtime/Testing/TestingForwardDecls.h"
namespace UnitTest
{
    template<> inline bool AreClose(ColorRGBAf const& expected, ColorRGBAf const& actual, float const& tolerance)
    {
        if (Abs(expected.r - actual.r) > tolerance)
            return false;
        if (Abs(expected.g - actual.g) > tolerance)
            return false;
        if (Abs(expected.b - actual.b) > tolerance)
            return false;
        if (Abs(expected.a - actual.a) > tolerance)
            return false;
        return true;
    }

    template<> inline bool AreClose(ColorRGBA32 const& expected, ColorRGBA32 const& actual, int const& tolerance)
    {
        if (Abs(expected.r - actual.r) > tolerance)
            return false;
        if (Abs(expected.g - actual.g) > tolerance)
            return false;
        if (Abs(expected.b - actual.b) > tolerance)
            return false;
        if (Abs(expected.a - actual.a) > tolerance)
            return false;
        return true;
    }
}
#endif
