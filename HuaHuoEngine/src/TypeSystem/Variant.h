#ifndef HUAHUOENGINE_VARIANT_H
#define HUAHUOENGINE_VARIANT_H

#include <cstdio>
namespace HuaHuo { class Type; }

template<typename T> const HuaHuo::Type* TypeOf();

class VariantRef
{
public:
    VariantRef() : m_Type(NULL), m_Data(NULL)
    {
    }

    VariantRef(const HuaHuo::Type* type, void* data) : m_Type(type), m_Data(data)
    {
        // Assert((m_Type != NULL) == (m_Data != NULL));
    }

    template<typename T>
    VariantRef(T& data) : m_Type(TypeOf<T>()), m_Data(&data)
    {
    }

    const HuaHuo::Type* GetType() const
    {
        return m_Type;
    }

    bool HasValue() const
    {
        // DebugAssert((m_Type != NULL) == (m_Data != NULL));
        return m_Type != NULL;
    }

    template<typename T>
    T& Get() const
    {
//        Assert(m_Type == TypeOf<T>());
//        Assert(m_Data != NULL);
        return *static_cast<T*>(m_Data);
    }

    bool operator==(const VariantRef& other) const
    {
        return m_Data == other.m_Data;
    }

    bool operator!=(const VariantRef& other) const
    {
        return !(*this == other);
    }

private:
    template<typename T>
    VariantRef(const T& data)
    {
        // const T& is disabled. Use ConstVariantRef for that case
    }

    const HuaHuo::Type* m_Type;
    void* m_Data;
};


class ConstVariantRef
{
public:
    ConstVariantRef() : m_Type(NULL), m_Data(NULL)
    {
    }

    ConstVariantRef(const HuaHuo::Type* type, const void* data) : m_Type(type), m_Data(data)
    {
//        Assert((m_Type != NULL) == (m_Data != NULL));
    }

    template<typename T>
    ConstVariantRef(const T& data) : m_Type(TypeOf<T>()), m_Data(&data)
    {
    }

    const HuaHuo::Type* GetType() const
    {
        return m_Type;
    }

    bool HasValue() const
    {
        // DebugAssert((m_Type != NULL) == (m_Data != NULL));
        return m_Type != NULL;
    }

    template<typename T>
    const T& Get() const
    {
        // Assert(m_Type == TypeOf<T>());
        // Assert(m_Data != NULL);
        return *static_cast<const T*>(m_Data);
    }

    bool operator==(const ConstVariantRef& other) const
    {
        return m_Data == other.m_Data;
    }

    bool operator!=(const ConstVariantRef& other) const
    {
        return !(*this == other);
    }

private:
    const HuaHuo::Type* m_Type;
    const void* m_Data;
};

#endif