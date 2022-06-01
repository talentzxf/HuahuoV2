#include "umbraHashGenerator.hpp"
#include "umbraHash.hpp"
#include <cstdio>

namespace Umbra
{

/*---------------------------------------------------------------*//*!
 * \brief
 *//*---------------------------------------------------------------*/

HashGenerator::HashGenerator (Allocator* a)
    : m_allocator(a), m_forward(NULL), m_bytes(0)
{
    m_values[0] = 0x9e3779b9;
    m_values[1] = 0x9e3779b9;
    m_values[2] = 0x9e3779b9;
}


/*---------------------------------------------------------------*//*!
 * \brief
 *//*---------------------------------------------------------------*/

String HashGenerator::getHashValue (void)
{
    if ((m_bytes % 12) != 0)
    {
        m_values[2] += m_bytes;
        shuffle(m_values[0], m_values[1], m_values[2]);
        m_bytes = 0;
    }

    char tmp[32];
    std::sprintf(tmp, "%08x%08x%08x", m_values[0], m_values[1], m_values[2]);
    return String(tmp, m_allocator);

}

/*---------------------------------------------------------------*//*!
 * \brief
 *//*---------------------------------------------------------------*/

Umbra::UINT32 HashGenerator::write (const void* ptr, Umbra::UINT32 numBytes)
{
    UMBRA_ASSERT(ptr);
    const UINT8* src = (const UINT8*)ptr;
    UINT32 len = numBytes;

    while (len--)
    {
        int pos = m_bytes++ % 12;
        int idx = pos >> 2;
        int shift = (pos & 0x3) << 3;
        m_values[idx] += (*src++ << shift);

        if (pos == 11)
            shuffle(m_values[0], m_values[1], m_values[2]);
    }

    if (m_forward)
        return m_forward->write(ptr, numBytes);

    return numBytes;
}

}