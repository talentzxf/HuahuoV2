#pragma once
#include <Objbase.h>

template<typename T>
struct CoTaskMemHolder
{
    CoTaskMemHolder() :
        m_Ptr(nullptr)
    {
    }

    ~CoTaskMemHolder()
    {
        Free();
    }

    void Attach(T* ptr)
    {
        Free();
        m_Ptr = ptr;
    }

    void Free()
    {
        if (m_Ptr != nullptr)
        {
            CoTaskMemFree(m_Ptr);
            m_Ptr = nullptr;
        }
    }

    operator T*() const
    {
        return m_Ptr;
    }

    T** operator&()
    {
        Free();
        return &m_Ptr;
    }

private:
    T* m_Ptr;

    CoTaskMemHolder(const CoTaskMemHolder<T>&);
    CoTaskMemHolder<T>& operator=(const CoTaskMemHolder<T>&);
};
