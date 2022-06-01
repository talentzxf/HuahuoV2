#include "ColorSpaceConversion.h"

bool CompareApproximately(ColorRGBAf f0, ColorRGBAf f1, float epsilon)
{
    return CompareApproximately(f0.r, f1.r, epsilon) &&
        CompareApproximately(f0.g, f1.g, epsilon) &&
        CompareApproximately(f0.b, f1.b, epsilon) &&
        CompareApproximately(f0.a, f1.a, epsilon);
}

ColorSpace GetActiveColorSpace()
{
//    __FAKEABLE_FUNCTION__(GetActiveColorSpace, ());
//
//    if (GetPlayerSettingsPtr())
//        return GetPlayerSettings().GetColorSpace();
//    else
        return kUninitializedColorSpace;
}

// Given a correlated color temperature (in Kelvin), estimate the RGB equivalent. Curve fit error is max 0.008.
ColorRGBAf CorrelatedColorTemperatureToRGB(float temperature)
{
    float r, g, b;

    // Temperature must fall between 1000 and 40000 degrees
    // The fitting require to divide kelvin by 1000 (allow more precision)
    float kelvin = clamp(temperature, 1000.f, 40000.f) / 1000.0f;
    float kelvin2 = kelvin * kelvin;

    // Using 6570 as a pivot is an approximation, pivot point for red is around 6580 and for blue and green around 6560.
    // Calculate each color in turn (Note, clamp is not really necessary as all value belongs to [0..1] but can help for extremum).
    // Red
    r = kelvin < 6.570f ? 1.0f : clamp((1.35651f + 0.216422f * kelvin + 0.000633715f * kelvin2) / (-3.24223f + 0.918711f * kelvin), 0.0f, 1.0f);
    // Green
    g = kelvin < 6.570f ?
        clamp((-399.809f + 414.271f * kelvin + 111.543f * kelvin2) / (2779.24f + 164.143f * kelvin + 84.7356f * kelvin2), 0.0f, 1.0f) :
        clamp((1370.38f + 734.616f * kelvin + 0.689955f * kelvin2) / (-4625.69f + 1699.87f * kelvin), 0.0f, 1.0f);
    //Blue
    b = kelvin > 6.570f ? 1.0f : clamp((348.963f - 523.53f * kelvin + 183.62f * kelvin2) / (2848.82f - 214.52f * kelvin + 78.8614f * kelvin2), 0.0f, 1.0f);

    return ColorRGBAf(r, g, b, 1.f);
}
