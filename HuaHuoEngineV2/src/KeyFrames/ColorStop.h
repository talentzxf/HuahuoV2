//
// Created by VincentZhang on 2022-11-15.
//

#ifndef HUAHUOENGINEV2_COLORSTOP_H
#define HUAHUOENGINEV2_COLORSTOP_H

#include "Math/Color.h"
#include <map>
#include <set>

class ColorStopEntry {
public:
    ColorStopEntry() : index(-1), value(-1.0f) {

    }

    ColorStopEntry(int index, float value, float r, float g, float b, float a) {
        this->index = index;
        this->value = value;
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    int GetIndex() {
        return index;
    }

    void SetIndex(int index) {
        this->index = index;
    }

    float GetValue() const;

    void SetValue(float value);

    const ColorRGBAf *GetColor() const;

    void SetColor(ColorRGBAf *color);

    DECLARE_SERIALIZE(ColorStopEntry);
private:
    int index;
    float value;
    ColorRGBAf color;
};

template<class TransferFunction>
void ColorStopEntry::Transfer(TransferFunction &transfer) {
    TRANSFER(index);
    TRANSFER(value);
    TRANSFER(color);
}

class ColorStopArray {
public:
    ColorStopArray() :
        nextColorStopId(0)
    {

    }

    const int GetColorStopCount() {
        return m_ColorStops.size();
    }

    ColorStopEntry *GetColorStop(int idx) { // This index might be different from what's recorded in the colorentry.
        if (idx >= m_ColorStops.size()) {
            return NULL;
        }

        int realIndex = *std::next(m_usedIndexes.begin(), idx);
        return &m_ColorStops[realIndex];
    }

    void AddEntry(ColorStopEntry& colorStopEntry) {
        colorStopEntry.SetIndex(nextColorStopId++);
        m_ColorStops.insert( std::pair<int, ColorStopEntry>(colorStopEntry.GetIndex(),  colorStopEntry));
        m_usedIndexes.insert(colorStopEntry.GetIndex());

        m_valueIndexPairs.push_back(ValueIndexPair(colorStopEntry.GetValue(), colorStopEntry.GetIndex()));

        SortValueIndexPair();
    }

    void DeleteEntry(int idx) {
        m_ColorStops.erase(idx);
        m_usedIndexes.erase(idx);

        std::erase_if(m_valueIndexPairs, [idx](ValueIndexPair pair){
            if(pair.second == idx)
                return true;
            return false;
        });
    }

    void UpdateAtIndex(int idx, float value, float r, float g, float b, float a) {
        m_ColorStops[idx].SetValue(value);
        ColorRGBAf rgbAf(r, g, b, a);
        m_ColorStops[idx].SetColor(&rgbAf);

        SortValueIndexPair();
    }

    void Lerp(ColorStopArray &c0, ColorStopArray &c1, float t);

    ColorRGBAf LerpColor(float value);

    DECLARE_SERIALIZE(ColorStopArray);
private:
        void SortValueIndexPair(){
            std::sort(m_valueIndexPairs.begin(), m_valueIndexPairs.end(), [](ValueIndexPair x1, ValueIndexPair x2){
                return x1.first > x2.first;
            });
        }
private:
    typedef std::pair<float, int> ValueIndexPair;

    std::map<int, ColorStopEntry> m_ColorStops;
    std::set<int> m_usedIndexes;

    // Always keep this from min->max based on value;
    std::vector<ValueIndexPair> m_valueIndexPairs;
    int nextColorStopId;
};

template<class TransferFunction>
void ColorStopArray::Transfer(TransferFunction &transfer) {
    TRANSFER(m_ColorStops);
    TRANSFER(nextColorStopId);
    TRANSFER(m_usedIndexes);
    TRANSFER(m_valueIndexPairs);
}

ColorStopArray Lerp(ColorStopArray &c0, ColorStopArray &c1, float t);

#endif //HUAHUOENGINEV2_COLORSTOP_H
