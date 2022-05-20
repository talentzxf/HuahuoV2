//
// Created by VincentZhang on 5/18/2022.
//

#include "RenderNodeQueue.h"

RenderNodeQueue::RenderNodeQueue(/*MemLabelRef memLabel*/)
        // : m_Nodes(memLabel), m_PageAllocator("RenderNodeQueue", memLabel),
         : m_ProjectorCount(0), m_RenderNodeCount(0)
{
    // memset(m_CleanupCallbacks, 0, sizeof(m_CleanupCallbacks));
}

RenderNodeQueue::~RenderNodeQueue()
{
//    Reset();
}
