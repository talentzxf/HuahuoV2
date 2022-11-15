//
// Created by VincentZhang on 2022-11-15.
//

#ifndef HUAHUOENGINEV2_COLORSTOP_H
#define HUAHUOENGINEV2_COLORSTOP_H
#include "Math/Color.h"
#include <vector>

class ColorStopEntry {
public:
    ColorStopEntry(): value(-1.0f){

    }

    ColorStopEntry(float value, float r, float g, float b, float a){
        this->value = value;
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    float GetValue() const;

    void SetValue(float value);

    const ColorRGBAf* GetColor() const;

    void SetColor(ColorRGBAf* color);

    DECLARE_SERIALIZE(ColorStopEntry);
private:
    float value;
    ColorRGBAf color;
};

template <class TransferFunction> void ColorStopEntry::Transfer(TransferFunction &transfer) {
    TRANSFER(value);
    TRANSFER(color);
}

class ColorStopArray{
public:
    const int GetColorStopCount(){
        return m_ColorStops.size();
    }

    ColorStopEntry* GetColorStop(int idx){
        if(idx >= m_ColorStops.size()){
            return NULL;
        }

        return &m_ColorStops[idx];
    }

    void AddEntry(ColorStopEntry& colorStopEntry){
        m_ColorStops.push_back(colorStopEntry);
    }

    void DeleteEntry(int idx){
        m_ColorStops.erase(std::next(m_ColorStops.begin(), idx));
    }

    void UpdateAtIndex(int idx, float value, float r, float g, float b, float a){
        m_ColorStops[idx].SetValue(value);
        ColorRGBAf rgbAf(r, g, b ,a);
        m_ColorStops[idx].SetColor(&rgbAf);
    }

    void Lerp(ColorStopArray& c0, ColorStopArray& c1, float t);

    DECLARE_SERIALIZE(ColorStopArray);
private:
    std::vector<ColorStopEntry> m_ColorStops;
};

template <class TransferFunction> void ColorStopArray::Transfer(TransferFunction &transfer) {
    TRANSFER(m_ColorStops);
}

ColorStopArray Lerp(ColorStopArray& c0, ColorStopArray& c1, float t);

#endif //HUAHUOENGINEV2_COLORSTOP_H
