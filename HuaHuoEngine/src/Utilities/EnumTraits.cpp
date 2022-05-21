//
// Created by VincentZhang on 5/21/2022.
//

#include "EnumTraits.h"
#include "Word.h"

namespace EnumTraits
{
    const char* ReflectionInfo::GetNameForValue(int value) const
    {
        const int* ptr = std::find(values, values + count, value);
        if (ptr == values + count)
            return NULL;

        return names[std::distance(values, ptr)];
    }
}

typedef const char* const* TIterator;
template
TIterator FindStringInRange<TIterator, const char*>(TIterator begin, TIterator end, const char* str, bool ignoreCase);