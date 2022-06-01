#pragma once

#include <algorithm>
#include "BaseClasses/BaseTypes.h"
#include "Logging/LogAssert.h"

// #include "Runtime/Jobs/Jobs.h"
// #include "Runtime/Threads/AtomicOps.h"
// #include "Runtime/Utilities/Utility.h"
// #include "Runtime/Utilities/BitUtility.h"
// #include "Runtime/Profiler/Profiler.h"

/*
    Optimizations:
        - All the QSort variants in this header are implemented using Bentley-McIlroy 3-way partitioning
        (http://www.cs.princeton.edu/courses/archive/spr10/cos226/lectures/06-23Quicksort-2x2.pdf).
        - For sorting quantities smaller than kMaxMedian3Sort it will choose the pivot using a median-of-3
            instead of the default median-of-9.
        - The array is allowed to split a maximum of 1.5 log2(N) times. The remaining arrays are sorted using
            heap sort.
        - Arrays smaller than kMaxInsertionSort will be sorted using insertion sort.
        - Tail recursive optimization
*/

namespace qsort_internal
{
    #define QSORT_USE_MEDIAN_SWAP 1 // when 0 it will use a max of 3 compares + 3 swaps / median. when 1 it will use a max of 5 compares and no swaps.
    #define QSORT_USE_MEDIAN_OF_9 1 // in some tests ( 1M random integers ) having this set to 0 is faster

    enum
    {
        kLogMaxInsertionSort    = 5,

        kMaxInsertionSort       = (1 << kLogMaxInsertionSort),  // arrays smaller than this value will be sorted with insertion sort
        kMaxMedian3Sort         = 64,                           // partitions smaller than this will use median-of-3 rather than median-of-9
        kMaxQSortJobs           = 4,
    };

    // internal pivot & and median finding functions

    #if QSORT_USE_MEDIAN_SWAP

    template<class T, class TCompareLessThan>
    inline void MedianOfThreeSwap(T first, T mid, T last, TCompareLessThan compareLessThan)
    {
        if (compareLessThan(*mid, *first))
            std::swap(*mid, *first);
        if (compareLessThan(*last, *mid))
            std::swap(*last, *mid);
        if (compareLessThan(*mid, *first))
            std::swap(*mid, *first);
    }

    #else

    template<class T, class TDifference, class TCompareLessThan>
    inline TDifference MedianOfThreeIndex(T a, TDifference i, TDifference j, TDifference k, TCompareLessThan compareLessThan)
    {
        return  (compareLessThan(a[i], a[j]) ?
            (compareLessThan(a[j], a[k]) ? j : compareLessThan(a[i], a[k]) ? k : i) :
            (compareLessThan(a[k], a[j]) ? j : compareLessThan(a[k], a[i]) ? k : i));
    }

    #endif

    template<class T, class TDifference, class TCompareLessThan>
    inline void FindAndMovePivotToLastPosition(T first, T last, TDifference count, TCompareLessThan compareLessThan)
    {
    #if QSORT_USE_MEDIAN_SWAP
        T mid = first + (count >> 1);

        if (!QSORT_USE_MEDIAN_OF_9 || count <= kMaxMedian3Sort)
        {
            MedianOfThreeSwap(first, mid, last, compareLessThan);
        }
        else
        {
            // use median-of-9 as partitioning element (Tukey ninther) for large size count
            // (http://www.cs.princeton.edu/courses/archive/spr10/cos226/lectures/06-23Quicksort-2x2.pdf)

            TDifference step = count >> 3;
            TDifference step2 = step << 1;

            MedianOfThreeSwap(first, first + step, first + step2, compareLessThan);
            MedianOfThreeSwap(mid - step, mid, mid + step, compareLessThan);
            MedianOfThreeSwap(last - step2, last - step, last, compareLessThan);
            MedianOfThreeSwap(first + step, mid, last - step, compareLessThan);
        }

        std::swap(*mid, *last);
    #else
        TDifference hi = count - 1;
        TDifference median;
        if (!QSORT_USE_MEDIAN_OF_9 || count <= kMaxMedian3Sort)
        {
            // use median-of-3 as partitioning element for medium size count
            median = MedianOfThreeIndex(first, (TDifference)0, hi >> 1, hi, compareLessThan);
        }
        else
        {
            // use median-of-9 as partitioning element (Tukey ninther) for large size count
            // (http://www.cs.princeton.edu/courses/archive/spr10/cos226/lectures/06-23Quicksort-2x2.pdf)

            TDifference eps = count >> 3;
            TDifference lo = 0;
            TDifference mid = lo + (count >> 1);
            TDifference m1 = MedianOfThreeIndex(first, lo, lo + eps, lo + eps + eps, compareLessThan);
            TDifference m2 = MedianOfThreeIndex(first, mid - eps, mid, mid + eps, compareLessThan);
            TDifference m3 = MedianOfThreeIndex(first, hi - eps - eps, hi - eps, hi, compareLessThan);
            median  = MedianOfThreeIndex(first, m1, m2, m3, compareLessThan);
        }
        std::swap(first[median], *last);
    #endif
    }

    // partitioning functions
    template<class T, class TDifference, class TCompareLessThan>
    inline std::pair<T, T> Partition3Way(T start, T end, TDifference count, TCompareLessThan compareLessThan)
    {
        T last = end - 1;

        FindAndMovePivotToLastPosition(start, last, count - 1, compareLessThan);
        typedef typename std::iterator_traits<T>::value_type TValue;
        TValue& pivot = *last;

        // Bentley-McIlroy 3-way partitioning - handles equal values nicely
        // (https://www.cs.princeton.edu/~rs/talks/QuicksortIsOptimal.pdf)

        TDifference i = -1, j = count - 1, p = -1, q = count - 1;
        for (;;)
        {
            while (compareLessThan(start[++i], pivot))
                if (i == count - 1)
                    break;
            while (compareLessThan(pivot, start[--j]))
                if (j == 0)
                    break;
            if (i >= j)
                break;
            std::swap(start[i], start[j]);
        }
        std::swap(start[i], pivot); j = i - 1; i = i + 1;

        for (TDifference k = 0; k < p; k++, j--)
            std::swap(start[k], start[j]);
        for (TDifference k = count - 2; k > q; k--, i++)
            std::swap(start[i], start[k]);
        return std::make_pair(start + i, start + j);
    }

    template<class T, class TDifference, class TCompareLessThan, class TCompareEqualTo>
    inline std::pair<T, T> Partition3Way(T start, T end, TDifference count, TCompareLessThan compareLessThan, TCompareEqualTo compareEqualTo)
    {
        T last = end - 1;

        FindAndMovePivotToLastPosition(start, last, count - 1, compareLessThan);
        typedef typename std::iterator_traits<T>::value_type TValue;
        TValue& pivot = *last;

        // Bentley-McIlroy 3-way partitioning - handles equal values nicely
        // (https://www.cs.princeton.edu/~rs/talks/QuicksortIsOptimal.pdf)

        TDifference i = -1, j = count - 1, p = -1, q = count - 1;
        for (;;)
        {
            while (compareLessThan(start[++i], pivot))
                if (i == count - 1)
                    break;
            while (compareLessThan(pivot, start[--j]))
                if (j == 0)
                    break;
            if (i >= j)
                break;
            std::swap(start[i], start[j]);
            if (compareEqualTo(start[i], pivot))
            {
                std::swap(start[++p], start[i]);
            }
            if (compareEqualTo(pivot, start[j]))
            {
                std::swap(start[j], start[--q]);
            }
        }
        std::swap(start[i], pivot); j = i - 1; i = i + 1;

        for (TDifference k = 0; k < p; k++, j--)
            std::swap(start[k], start[j]);
        for (TDifference k = count - 2; k > q; k--, i++)
            std::swap(start[i], start[k]);
        return std::make_pair(start + i, start + j);
    }

    // sort internal functions
    template<class T, class TCompare>
    inline void InsertionSort(T start, T end, TCompare compareLessThan)
    {
        for (T i = start; i < end; i++)
            for (T j = i; j > start && compareLessThan(*j, *(j - 1)); j--)
                std::swap(*j, *(j - 1));
    }

    // Single threaded, run in place
    template<class T, class TDifference, class TCompareLessThan>
    inline void QSort(T start, T end, TDifference difference, TCompareLessThan compareLessThan)
    {
        TDifference count;
        while (((count = end - start) >= kMaxInsertionSort) && 0 < difference)
        {
            std::pair<T, T> pindex = Partition3Way(start, end, count, compareLessThan);
            difference /= 2, difference += difference / 2;  // allow 1.5 log2(N) divisions

            if (((pindex.second + 1) - start) < (end - pindex.first))
            {
                QSort(start, (pindex.second + 1), (pindex.second + 1) - start, compareLessThan);
                start = pindex.first;
            }
            else
            {
                // loop on first half
                QSort(pindex.first, end, end - pindex.first, compareLessThan);
                end = (pindex.second + 1);
            }
        }

        if (count >= kMaxInsertionSort)
        {
            std::make_heap(start, end, compareLessThan);
            std::sort_heap(start, end, compareLessThan);
        }
        else if (count > 1)
            InsertionSort(start, end, compareLessThan);
    }

    // Single threaded, run in place
    // Faster QSort version. It handles equal values better but requires an "equal_to" compare function.
    template<class T, class TDifference, class TCompareLessThan, class TCompareEqualTo>
    inline void QSortFast(T start, T end, TDifference difference, TCompareLessThan compareLessThan, TCompareEqualTo compareEqualTo)
    {
        TDifference count;
        while (((count = end - start) >= kMaxInsertionSort) && 0 < difference)
        {
            std::pair<T, T> pindex = Partition3Way(start, end, count, compareLessThan, compareEqualTo);
            difference /= 2, difference += difference / 2;  // allow 1.5 log2(N) divisions

            if (((pindex.second + 1) - start) < (end - pindex.first))
            {
                QSortFast(start, (pindex.second + 1), (pindex.second + 1) - start, compareLessThan, compareEqualTo);
                start = pindex.first;
            }
            else
            {
                // loop on first half
                QSortFast(pindex.first, end, end - pindex.first, compareLessThan, compareEqualTo);
                end = (pindex.second + 1);
            }
        }

        if (count >= kMaxInsertionSort)
        {
            std::make_heap(start, end, compareLessThan);
            std::sort_heap(start, end, compareLessThan);
        }
        else if (count > 1)
            InsertionSort(start, end, compareLessThan);
    }
}
