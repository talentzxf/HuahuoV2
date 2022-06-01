#pragma once

#include "qsort_internal.h"

/*
    Timings comparison with std::sort:
        - std::sort - RandomInts: 791ms, SortedInts: 189ms, RandomFloats: 878ms, SortedFloats: 190ms
        - QSort         - RandomInts: 795ms, SortedInts: 200ms, RandomFloats: 843ms, SortedFloats: 228ms
        - QSortFast - RandomInts: 723ms, SortedInts: 186ms, RandomFloats: 844ms, SortedFloats: 206ms
*/

// Single threaded, run in place
template<class T, class TCompareLessThan>
inline void QSort(T start, T end, TCompareLessThan compareLessThan)
{
    qsort_internal::QSort(start, end, end - start, compareLessThan);
}

// Single threaded, run in place
// Faster QSort version. It handles equal values better but requires an "equal_to" compare function.
template<class T, class TCompareLessThan, class TCompareEqualTo>
inline void QSortFast(T start, T end, TCompareLessThan compareLessThan, TCompareEqualTo compareEqualTo)
{
    qsort_internal::QSortFast(start, end, end - start, compareLessThan, compareEqualTo);
}
