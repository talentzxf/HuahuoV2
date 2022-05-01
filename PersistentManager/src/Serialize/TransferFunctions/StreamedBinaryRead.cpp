//
// Created by VincentZhang on 4/21/2022.
//

#include "StreamedBinaryRead.h"

void StreamedBinaryRead::Align()
{
    m_Cache.Align4Read();
}

void StreamedBinaryRead::ReadDirect(void* data, int byteSize)
{
    m_Cache.Read(data, byteSize);
}