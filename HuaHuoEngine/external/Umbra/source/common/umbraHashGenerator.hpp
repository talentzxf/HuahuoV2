// Copyright (c) 2015 Umbra Software Ltd.
// All rights reserved. www.umbrasoftware.com

#ifndef UMBRAHASHGENERATOR_HPP
#define UMBRAHASHGENERATOR_HPP

#include "umbraChecksum.hpp"
#include "umbraString.hpp"

namespace Umbra
{

/*!
 * \brief   Helper for generating hash value
 */
class HashGenerator: public OutputStream
{
public:
    HashGenerator (Allocator* a);

    UINT32  write (const void* ptr, UINT32 numBytes);
    String  getHashValue (void);
    void    setForward (OutputStream* forward) { m_forward = forward; }

private:
    Allocator*      m_allocator;
    OutputStream*   m_forward;
    int             m_bytes;
    UINT32          m_values[3];
};

}

#endif