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

const ColorRGBAf* ColorStopEntry::GetColor() const {
    return &color;
}

void ColorStopEntry::SetColor(const ColorRGBAf* color) {
    ColorStopEntry::color = *color;
}
