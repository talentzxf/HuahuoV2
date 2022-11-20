//
// Created by VincentZhang on 2022-11-15.
//

#include "ColorStop.h"
#include "Math/Color.h"

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

ColorRGBAf ColorStopArray::LerpColor(float value) {
    if (GetColorStopCount() == 0) { // Currently no color stop, return black.
        return ColorRGBAf(0.0f, 0.0f, 0.0f, 1.0f);
    }

    if (GetColorStopCount() == 1) { // Currently only one color stop, return the color of this color stop.
        return *GetColorStop(0)->GetColor();
    }

    // If the value is leq than the first value. Return the first color.
    if(value <= m_valueIndexPairs.begin()->first){
        int beginColorStopIndex = m_valueIndexPairs.begin()->second;
        return *m_ColorStops[beginColorStopIndex].GetColor();
    }

    // If the value is leq than the last value. Return the last color.
    auto lastValueIndexPair = m_valueIndexPairs[m_valueIndexPairs.size() - 1];
    if(value >= lastValueIndexPair.first) {
        int lastColorStopIndex = lastValueIndexPair.second;
        return *m_ColorStops[lastColorStopIndex].GetColor();
    }

    // TODO: Change to binary search.
    auto prevPair = m_valueIndexPairs.begin();
    auto curPair = m_valueIndexPairs.begin()+1;

    while(curPair != m_valueIndexPairs.end()){
        // if value is between the prevPair and the curPair, we found it.
        if(prevPair->first < value && curPair->first >= value){
            break;
        }

        curPair++;
        prevPair++;
    }

    float prevValue = m_ColorStops[prevPair->second].GetValue();
    float curValue = m_ColorStops[curPair->second].GetValue();

    const ColorRGBAf& prevColor = *m_ColorStops[prevPair->second].GetColor();
    const ColorRGBAf& curColor = *m_ColorStops[curPair->second].GetColor();

    if(prevValue == curValue){ // TODO: Is it possible?
        return prevColor;
    }

    float ratio = (value - prevValue) / (curValue - prevValue);

    return ::Lerp(prevColor, curColor, ratio);
}