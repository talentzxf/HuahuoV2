#pragma once
#include <wtypes.h>
#include <OleAuto.h>

struct SafeArrayHolder
{
    SafeArrayHolder() :
        m_SafeArray(NULL)
    {
    }

    ~SafeArrayHolder()
    {
        if (m_SafeArray != NULL)
            SafeArrayDestroy(m_SafeArray);
    }

    operator SAFEARRAY*() const
    {
        return m_SafeArray;
    }

    SAFEARRAY** operator&()
    {
        if (m_SafeArray != NULL)
        {
            SafeArrayDestroy(m_SafeArray);
            m_SafeArray = NULL;
        }

        return &m_SafeArray;
    }

private:
    SAFEARRAY* m_SafeArray;

    SafeArrayHolder(const SafeArrayHolder&);
    SafeArrayHolder& operator=(const SafeArrayHolder&);
};

template<typename T>
struct SafeArrayAccessHolder
{
    SafeArrayAccessHolder(SAFEARRAY* safeArray) :
        m_SafeArray(safeArray),
        m_Data(NULL)
    {
        m_Hr = SafeArrayAccessData(m_SafeArray, reinterpret_cast<void**>(&m_Data));
    }

    ~SafeArrayAccessHolder()
    {
        if (SUCCEEDED(m_Hr))
            SafeArrayUnaccessData(m_SafeArray);
    }

    operator bool() const
    {
        return SUCCEEDED(m_Hr);
    }

    T operator[](size_t index) const
    {
        return m_Data[index];
    }

private:
    SAFEARRAY* m_SafeArray;
    T* m_Data;
    HRESULT m_Hr;

    SafeArrayAccessHolder(const SafeArrayAccessHolder&);
    SafeArrayAccessHolder& operator=(const SafeArrayAccessHolder&);
};
