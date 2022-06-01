#pragma once

// stl vector doesnt deallocate memory on itself.
// push_back and resizing increases memory by a factor of 2, when growing
// pop_back, resizing and even clear do not deallocate any memory
// This of course wastes a lot of memory, fortunately there is the swap trick to
// bring back memory usage to normality
//#include <algorithm>

template<class T>
inline void trim_vector(T& v)
{
    if (v.capacity() != v.size())
    {
        T temp = v;
        temp.swap(v);
    }
}

// The following functions do the same as their vector member function
// equivalents, only they allocate exactly the requested amount of memory.

template<class T>
inline void resize_trimmed(T& v, unsigned int sz)
{
    // the vector is growing
    if (sz > v.size())
    {
        if (sz != v.capacity())
        {
            T temp(v.get_allocator());
            temp.reserve(sz);
            temp.assign(v.begin(), v.end());
            temp.resize(sz);
            temp.swap(v);
        }
        else
            v.resize(sz);
    }
    // the vector is shrinking
    else if (sz < v.size())
    {
        T temp(v.begin(), v.begin() + sz, v.get_allocator());
        temp.swap(v);
    }
}

template<class T>
inline void reserve_trimmed(T& v, unsigned int sz)
{
    if (sz != v.capacity())
    {
        T temp;
        temp.reserve(sz);
        temp.assign(v.begin(), v.end());
        temp.swap(v);
    }
}

template<class T>
inline void clear_trimmed(T& v)
{
    T temp;
    temp.swap(v);
}

template<class T>
inline void push_back_trimmed(T& vec, const typename T::value_type& value)
{
    if (vec.size() + 1 != vec.capacity())
    {
        T temp;
        temp.reserve(vec.size() + 1);
        temp.assign(vec.begin(), vec.end());
        temp.push_back(value);
        temp.swap(vec);
    }
    else
        vec.push_back(value);
}

template<class T>
inline void erase_trimmed(T& vec, typename T::iterator i)
{
    Assert(i != vec.end());

    T temp;
    temp.reserve(vec.size() - 1);
    temp.assign(vec.begin(), i);
    temp.insert(temp.end(), ++i, vec.end());
    vec.swap(temp);
}

// Helper function for getting the first array element or NULL if empty
// std::vector does not allow address of vec[0] on empty array
// dynamic_array does not guarantee that data() returns NULL when empty

template<typename T>
const typename T::value_type* array_data_or_null_if_empty(const T& vec)
{
    return vec.empty() ? NULL : &vec[0];
}

template<typename T>
typename T::value_type* array_data_or_null_if_empty(T& vec)
{
    return vec.empty() ? NULL : &vec[0];
}
