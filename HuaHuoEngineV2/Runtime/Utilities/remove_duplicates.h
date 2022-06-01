#pragma once

template<class ForwardIterator, class BinaryPredicate>
ForwardIterator remove_duplicates_using_copy_internal(ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
{
    ForwardIterator result = first;

    // if empty we need to early out, otherwise we get in an infinite loop
    if (first == last)
        return result;

    ForwardIterator previous = first;
    first++;
    result++;

    while (first != last)
    {
        // The element is unique, thus copy it.
        if (pred(*previous, *first))
        {
            *result = *first;
            ++result;
        }
        ++first;
        ++previous;
    }
    return result;
}

// Removes any duplicates from the array
template<class ForwardIterator, class BinaryPredicate>
ForwardIterator remove_duplicates(ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
{
    // if empty we need to early out, otherwise we get in an infinite loop
    if (first == last)
        return last;

    ForwardIterator previous = first;
    ForwardIterator i = first;
    i++;

    // Loop to find if there are any duplicates.
    // if there are any duplicates, switch to remove_duplicates_using_copy_internal,
    // which will copy only the unique elements.
    // If however there are no actual duplicates, then we never even need to copy any elements.
    while (i != last)
    {
        // *previous < *i. Thus have different values (it can't be larger since we assume sorted)
        if (pred(*previous, *i))
        {
            previous = i;
            i++;
        }
        else
        {
            return remove_duplicates_using_copy_internal(previous, last, pred);
        }
    }

    return last;
}

// Assert if the elements are sorted and have no duplicates
template<class ForwardIterator, class BinaryPredicate>
void assert_duplicates_and_sorted(ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
{
    #if DEBUGMODE
    if (first == last)
        return;

    // Check that all elements are in increasing order. Thus no duplicates, and are sorted
    ForwardIterator previous = first;
    ForwardIterator i = first;
    i++;
    for (; i != last; ++i)
    {
        Assert(pred(*previous, *i));
        previous = i;
    }
    #endif
}

template<class T, class BinaryPredicate>
void sort_clear_duplicates(T& container, BinaryPredicate pred)
{
    if (container.empty())
        return;

    std::sort(container.begin(), container.end());

    typename T::iterator newEnd = remove_duplicates(container.begin(), container.end(), pred);
    // If we removed any elements, resize to new size
    if (newEnd != container.end())
        container.erase(newEnd, container.end());

    assert_duplicates_and_sorted(container.begin(), container.end(), pred);
}

template<class T>
void sort_clear_duplicates(T& container)
{
    sort_clear_duplicates(container, std::less<typename T::value_type>());
}
