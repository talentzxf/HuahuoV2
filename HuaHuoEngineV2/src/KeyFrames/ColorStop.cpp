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

void ColorStopEntry::SetColor(const ColorRGBAf *color) {
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

ColorStopEntry& Lerp(ColorStopEntry& k1, ColorStopEntry&k2, float ratio){
    ColorStopEntry resultEntry;
    resultEntry.SetValue(Lerp(k1.GetValue(), k2.GetValue(), ratio));
    resultEntry.SetColor(Lerp(*k1.GetColor(), *k2.GetColor(), ratio));

    return resultEntry;
}

void ColorStopArray::Lerp(ColorStopArray &k0, ColorStopArray &k1, float ratio) {
    this->m_ColorStops.resize(k0.m_ColorStops.size());

    for(int colorStopIndex = 0 ; colorStopIndex < k0.GetColorStopCount(); colorStopIndex++){
        ColorStopEntry entry = Lerp( *k0.GetColorStop(colorStopIndex),*k1.GetColorStop(colorStopIndex), ratio);
        m_ColorStops[colorStopIndex] = entry;
    }
}