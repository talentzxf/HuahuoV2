#include "GrowableBuffer.h"
#include "Memory/MemoryMacros.h"

GrowableBuffer::GrowableBuffer(MemLabelRef label, size_t initialSize, size_t growIncrement, GrowMode growMode)
    : m_Label(label)
    , m_Buffer(NULL)
    , m_Capacity(initialSize)
    , m_GrowStepSize(growIncrement)
    , m_GrowMode(growMode)
{
    Assert(initialSize > 0);
    m_Buffer = (char*)HUAHUO_MALLOC_ALIGNED(m_Label, initialSize, 64);
    m_Size = 0;
}

GrowableBuffer::GrowableBuffer(const GrowableBuffer& other)
    : m_Label(other.m_Label)
    , m_Capacity(other.m_Capacity)
    , m_Size(other.m_Size)
    , m_GrowStepSize(other.m_GrowStepSize)
{
    m_Buffer = (char*)HUAHUO_MALLOC_ALIGNED(m_Label, m_Capacity, 64);
    memcpy(m_Buffer, other.m_Buffer, m_Size);
}

GrowableBuffer::~GrowableBuffer()
{
    if (m_Buffer != NULL){
        HUAHUO_FREE(m_Label, m_Buffer);
    }
}

void GrowableBuffer::EnlargeBuffer(size_t dataPos, size_t dataEnd)
{
    size_t dataSize = dataEnd - dataPos;
    size_t growSize = std::max(dataSize, m_GrowStepSize);
    m_Capacity += growSize;

    if (m_GrowMode == GrowMode::DoubleOnGrow)
        m_Capacity *= 2;

    m_Buffer = (char*)HUAHUO_REALLOC_ALIGNED(m_Label, m_Buffer, m_Capacity, 64);
}
