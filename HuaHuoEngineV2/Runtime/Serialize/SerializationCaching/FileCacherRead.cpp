//
// Created by VincentZhang on 6/4/2022.
//

#include "FileCacherRead.h"
#include "Utilities/File.h"

FileCacherRead::FileCacherRead(MemLabelId memLabel, const std::string& pathName, size_t cacheSize, bool prefetchNextBlock)
        : m_MemLabel(memLabel)
        , m_CacheSize(cacheSize)
//        , m_Path(memLabel)
//        , m_PrefetchNextBlock(prefetchNextBlock)
{
//#if !SUPPORT_THREADS
//    Assert(prefetchNextBlock == false);
//#endif
    m_Path = PathToAbsolutePath(pathName);

    // Get File size
    m_FileSize = ::GetFileLength(m_Path);

//    m_DirectReadCommands.SetMemoryLabel(memLabel);
//    for (int i = 0; i < kCacheCount; i++)
//    {
//#if SUPPORT_THREADS
//        m_ReadCommandInProgress[i] = false;
//#endif
//        m_ReadCommands[i].SetMemoryLabel(memLabel);
//        AllocateBlock(m_ActiveBlocks[i]);
//        m_ActiveBlocks[i].block = -1;
//        Assert(m_ActiveBlocks[i].locked == 0);
//    }

#if DEBUG_LINEAR_FILE_ACCESS
    m_LastFileAccessPosition = 0;
#endif
}

FileCacherRead::~FileCacherRead(){

}

/////@TODO: Make a Async read that simply guarantees that data is loaded after transfer has completed.

void FileCacherRead::DirectRead(void* data, size_t position, size_t size)
{
//    // load the data from disk
//    // only if the physical file contains any data for this block
//    FatalErrorIf(m_FileSize - position < size);
//
//    Assert(m_DirectReadCommands.status == AsyncReadCommand::kCompleted || m_DirectReadCommands.status == AsyncReadCommand::kNotUsed);
//    m_DirectReadCommands.fileName = m_Path;
//    m_DirectReadCommands.buffer = (UInt8*)data;
//    m_DirectReadCommands.size = size;
//    m_DirectReadCommands.offset = (UInt64)position;
//
//    m_DirectReadCommands.SetAssetContext(AssetSubsystem::Other);
//    // TODO: Fill in the asset context information for more useful ASRM metrics results.
//
//    SyncReadRequest(&m_DirectReadCommands);
//
//    DebugLinearFileAccess(position, size);
}

////////@TODO: Write a test for reading an invalid file where it runs out of bounds... or make the file operation fail...

void FileCacherRead::LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
{
//    // Ensure that there is no recursive locking of cache blocks.
//    for (int i = 0; i < kCacheCount; i++)
//    {
//        Assert(m_ActiveBlocks[i].locked == 0);
//    }
//
//    // Request this block
//    int indexToLoad = RequestBlock(block);
//    SyncReadCommandBlock(indexToLoad);
//
//    // Return the active block
//    m_ActiveBlocks[indexToLoad].locked = 1;
//    *startPos = m_ActiveBlocks[indexToLoad].data;
//    *endPos = m_ActiveBlocks[indexToLoad].data + m_ReadCommands[indexToLoad].size;
//
//    if (m_PrefetchNextBlock)
//    {
//        // Use the other block to prefetch the next block
//        int prefetchIndex = indexToLoad == 0 ? 1 : 0;
//        int prefetchBlock = block + 1;
//        if (m_ReadCommands[prefetchIndex].status != AsyncReadCommand::kInProgress && m_ActiveBlocks[prefetchIndex].block != prefetchBlock)
//            Request(prefetchBlock, prefetchIndex, m_ActiveBlocks[prefetchIndex], false);
//    }
}


void FileCacherRead::UnlockCacheBlock(size_t block)
{
//    for (int i = 0; i < kCacheCount; i++)
//    {
//        if ((size_t)m_ActiveBlocks[i].block == block && m_ActiveBlocks[i].locked == 1)
//        {
//            m_ActiveBlocks[i].locked--;
//            return;
//        }
//    }
}

std::string FileCacherRead::GetPathName() const
{
    return m_Path;
}