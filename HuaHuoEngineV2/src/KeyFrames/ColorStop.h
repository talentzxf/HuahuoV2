//
// Created by VincentZhang on 2022-11-15.
//

#ifndef HUAHUOENGINEV2_COLORSTOP_H
#define HUAHUOENGINEV2_COLORSTOP_H

#include "Math/Color.h"
#include <map>
#include <set>
#include <vector>
#include <utility>

class ColorStopEntry {
public:
    ColorStopEntry() : identifier(-1), value(-1.0f) {

    }

    ColorStopEntry(int identifier, float value, float r, float g, float b, float a) {
        this->identifier = identifier;
        this->value = value;
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    int GetIdentifier() {
        return identifier;
    }

    void SetIdentifier(int identifier) {
        this->identifier = identifier;
    }

    float GetValue() const;

    void SetValue(float value);

    const ColorRGBAf *GetColor() const;

    void SetColor(ColorRGBAf *color);

    DECLARE_SERIALIZE(ColorStopEntry);
private:
    int identifier;
    float value;
    ColorRGBAf color;
};

template<class TransferFunction>
void ColorStopEntry::Transfer(TransferFunction &transfer) {
    TRANSFER(identifier);
    TRANSFER(value);
    TRANSFER(color);
}

class ColorStopArray {
public:
    ColorStopArray() :
            nextColorStopIdentifier(0)
    {

    }

    const int GetColorStopCount() {
        return m_ColorStops.size();
    }

    ColorStopEntry *GetColorStop(int idx) { // This is index. Not identifier of the color stop entry. Might be different from the identifier of the colorStopEntry.
        if (idx >= m_ColorStops.size()) {
            return NULL;
        }

        int realIdentifier = *std::next(m_usedIndentifiers.begin(), idx);
        return &m_ColorStops[realIdentifier];
    }

    void AddEntry(ColorStopEntry& colorStopEntry) {
        colorStopEntry.SetIdentifier(nextColorStopIdentifier++);
        m_ColorStops.insert( std::pair<int, ColorStopEntry>(colorStopEntry.GetIdentifier(),  colorStopEntry));
        m_usedIndentifiers.insert(colorStopEntry.GetIdentifier());

        m_valueIndentifierPairs.push_back(ValueIdentifierPair(colorStopEntry.GetValue(), colorStopEntry.GetIdentifier()));

        SortValueIdentifierPair();
    }

    void DeleteEntry(int idx) {
        m_ColorStops.erase(idx);
        m_usedIndentifiers.erase(idx);

        std::erase_if(m_valueIndentifierPairs, [idx](ValueIdentifierPair pair){
            if(pair.second == idx)
                return true;
            return false;
        });
    }

    void UpdateAtIdentifier(int identifier, float value, float r, float g, float b, float a) {
        m_ColorStops[identifier].SetValue(value);
        for(int i = 0; i < this->m_valueIndentifierPairs.size(); i++){
            auto pair = m_valueIndentifierPairs[i];
            if(pair.second == identifier){
                m_valueIndentifierPairs[i].first = value;
                break;
            }
        }

        ColorRGBAf rgbAf(r, g, b, a);
        m_ColorStops[identifier].SetColor(&rgbAf);

        SortValueIdentifierPair();
    }

    void Lerp(ColorStopArray &c0, ColorStopArray &c1, float t);

    ColorRGBAf LerpColor(float value);

    DECLARE_SERIALIZE(ColorStopArray);
private:
        void SortValueIdentifierPair(){
            std::sort(m_valueIndentifierPairs.begin(), m_valueIndentifierPairs.end(), [](ValueIdentifierPair x1, ValueIdentifierPair x2){
                return x1.first < x2.first;
            });
        }
private:
    typedef std::pair<float, int> ValueIdentifierPair;

    std::map<int, ColorStopEntry> m_ColorStops; // From identifier->ColorStopEntry (not index!!!).
    std::set<int> m_usedIndentifiers;

    // Always keep this from min->max based on value;
    std::vector<ValueIdentifierPair> m_valueIndentifierPairs;
    int nextColorStopIdentifier;
};

template<class TransferFunction>
void ColorStopArray::Transfer(TransferFunction &transfer) {
    TRANSFER(m_ColorStops);
    TRANSFER(nextColorStopIdentifier);
    TRANSFER(m_usedIndentifiers);
    TRANSFER(m_valueIndentifierPairs);
}

ColorStopArray Lerp(ColorStopArray &c0, ColorStopArray &c1, float t);

#endif //HUAHUOENGINEV2_COLORSTOP_H
