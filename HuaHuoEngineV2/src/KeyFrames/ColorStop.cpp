//
// Created by VincentZhang on 2022-11-15.
//

#include "ColorStop.h"

float ColorStopEntry::GetValue() const {
    return value;
}

void ColorStopEntry::SetValue(float value) {
    ColorStopEntry::value = value;
}

const ColorRGBAf *ColorStopEntry::GetColor() const {
    return &color;
}

void ColorStopEntry::SetColor(ColorRGBAf *color) {
    ColorStopEntry::color = *color;
}

ColorStopArray Lerp(ColorStopArray &k0, ColorStopArray &k1, float t) {
    ColorStopArray resultStopArray;
    if(k0.GetColorStopCount() != k1.GetColorStopCount())
    {
        Assert("Lerp colorstop size mismatch!!");
        return resultStopArray;
    }

    resultStopArray.Lerp(k0, k1,t);
    return resultStopArray;
}

ColorStopEntry LerpColorEntry(ColorStopEntry k1, ColorStopEntry k2, float ratio){
    ColorStopEntry resultEntry;
    resultEntry.SetIndex(k1.GetIndex());
    resultEntry.SetValue(Lerp(k1.GetValue(), k2.GetValue(), ratio));
    ColorRGBAf k1Color = *k1.GetColor();
    ColorRGBAf k2Color = *k2.GetColor();
    ColorRGBAf resultColor = Lerp(k1Color, k2Color, ratio);
    resultEntry.SetColor(&resultColor);

    return resultEntry;
}

void ColorStopArray::Lerp(ColorStopArray &k0, ColorStopArray &k1, float ratio) {
    for(int colorStopIndex = 0 ; colorStopIndex < k0.GetColorStopCount(); colorStopIndex++){
        ColorStopEntry k0ColorStop = *k0.GetColorStop(colorStopIndex);
        ColorStopEntry k1ColorStop = *k1.GetColorStop(colorStopIndex);

        ColorStopEntry entry = LerpColorEntry( k0ColorStop, k1ColorStop, ratio);
        m_ColorStops[colorStopIndex] = entry;

        m_usedIndexes.insert(colorStopIndex);
        nextColorStopId = std::max(nextColorStopId, colorStopIndex + 1);
    }
}