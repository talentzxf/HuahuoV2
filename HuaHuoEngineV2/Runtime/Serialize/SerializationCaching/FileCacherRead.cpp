//
// Created by VincentZhang on 6/4/2022.
//

#include "FileCacherRead.h"
#include "Utilities/File.h"
#include "Memory/MemoryMacros.h"

FileCacherRead::FileCacherRead(MemLabelId memLabel, const std::string& pathName, size_t cacheSize, bool prefetchNextBlock)
        : m_MemLabel(memLabel)
        , m_CacheSize(cacheSize)
//        , m_Path(memLabel)
        , m_PrefetchNextBlock(prefetchNextBlock)
{
#if !SUPPORT_THREADS
    Assert(prefetchNextBlock == false);
#endif
    m_Path = PathToAbsolutePath(pathName);

    // Get File size
    m_FileSize = ::GetFileLength(m_Path);

    // m_DirectReadCommands.SetMemoryLabel(memLabel);
    for (int i = 0; i < kCacheCount; i++)
    {
#if SUPPORT_THREADS
        m_ReadCommandInProgress[i] = false;
#endif
        // m_ReadCommands[i].SetMemoryLabel(memLabel);
        AllocateBlock(m_ActiveBlocks[i]);
        m_ActiveBlocks[i].block = -1;
        Assert(m_ActiveBlocks[i].locked == 0);
    }

#if DEBUG_LINEAR_FILE_ACCESS
    m_LastFileAccessPosition = 0;
#endif
}

void FileCacherRead::AllocateBlock(CacheBlock& block)
{
    Assert(block.data == NULL);
    block.data = (UInt8*)HUAHUO_MALLOC(m_MemLabel, m_CacheSize);
}

void FileCacherRead::DeallocateBlock(CacheBlock& block)
{
    HUAHUO_FREE(m_MemLabel, block.data);
    block.data = NULL;
}

FileCacherRead::~FileCacherRead(){
    for (int i = 0; i < kCacheCount; i++)
    {
        Assert(m_ActiveBlocks[i].locked == 0);

        SyncReadCommandBlock(i);
        DeallocateBlock(m_ActiveBlocks[i]);
    }

    AsyncReadForceCloseFile(m_Path);
}

void FileCacherRead::DebugLinearFileAccess(size_t position, size_t size)
{
#if DEBUG_LINEAR_FILE_ACCESS
    // printf_console("ACCCESS: [%08x] %s %i bytes @ %i to %08x\n",this, __FUNCTION__, size, position, data);
    core::string_ref fileName = GetLastPathNameComponent(m_Path);
    if (position < m_LastFileAccessPosition && fileName != "mainData" && fileName != "unity default resources")
    {
        ErrorString(core::Format("File access: {0} is not linear  Reading: {1} Seek position: {2}", fileName, position, m_LastFileAccessPosition));
    }

    m_LastFileAccessPosition = position + size;
#endif
}

/////@TODO: Make a Async read that simply guarantees that data is loaded after transfer has completed.

void FileCacherRead::DirectRead(void* data, size_t position, size_t size)
{
    // load the data from disk
    // only if the physical file contains any data for this block
    // FatalErrorIf(m_FileSize - position < size);

    Assert(m_DirectReadCommands.status == AsyncReadCommand::kCompleted || m_DirectReadCommands.status == AsyncReadCommand::kNotUsed);
    m_DirectReadCommands.fileName = m_Path;
    m_DirectReadCommands.buffer = (UInt8*)data;
    m_DirectReadCommands.size = size;
    m_DirectReadCommands.offset = (UInt64)position;

    m_DirectReadCommands.SetAssetContext(AssetSubsystem::Other);
    // TODO: Fill in the asset context information for more useful ASRM metrics results.

    SyncReadRequest(&m_DirectReadCommands);

    DebugLinearFileAccess(position, size);
}

bool FileCacherRead::Request(int block, int readCmdIndex, CacheBlock& cacheBlock, bool sync)
{
    size_t startBlockOffset = block * GetCacheSize();
    if (startBlockOffset >= m_FileSize)
        return false;

    // this block could be in the process of being prefetched. in that case we need to wait for it
    SyncReadCommandBlock(readCmdIndex);

    size_t size = std::min<size_t>(m_FileSize - startBlockOffset, GetCacheSize());
    AsyncReadCommand &readCommand = m_ReadCommands[readCmdIndex];
    Assert(readCommand.status != AsyncReadCommand::kInProgress);
    readCommand.fileName = m_Path;
    readCommand.buffer = cacheBlock.data;
    readCommand.size = size;
    readCommand.offset = (UInt64)(block * m_CacheSize);
    readCommand.priority = AsyncReadCommand::kPriorityHigh;
    readCommand.SetAssetContext(AssetSubsystem::Other);
    // TODO: Fill in the asset context information for more useful ASRM metrics results.
#if SUPPORT_THREADS
    readCommand.userData = &m_ReadCompleteSemaphore[readCmdIndex];
    Assert(!m_ReadCommandInProgress[readCmdIndex]);
    readCommand.userCallback = sync ? NULL : SignalCallback;
    m_ReadCommandInProgress[readCmdIndex] = !sync;
#endif
    cacheBlock.block = block;
    Assert(readCommand.offset < m_FileSize);

    if (sync)
    {
        SyncReadRequest(&readCommand);
        return readCommand.status == AsyncReadCommand::kCompleted;
    }

    AsyncReadRequest(&readCommand);
    return true;
}


int FileCacherRead::RequestBlock(int block)
{
    // Check if we already requested loading the block
    for (int i = 0; i < kCacheCount; i++)
    {
        if (m_ActiveBlocks[i].block == block)
            return i;
    }

    // Check if there is an available block that is not in progress
    int indexToLoad = -1;
    for (int i = 0; i < kCacheCount; i++)
    {
        if (m_ReadCommands[i].status != AsyncReadCommand::kInProgress)
            indexToLoad = i;
    }

    // If there is no block available, wait for a slot
    if (indexToLoad == -1)
        indexToLoad = 0;

    // wait for the slot to complete if necessary
    SyncReadCommandBlock(indexToLoad);

    bool res = Request(block, indexToLoad, m_ActiveBlocks[indexToLoad], true);
    Assert(res);

    return indexToLoad;
}

void FileCacherRead::SyncReadCommandBlock(int index)
{
    // __FAKEABLE_METHOD__(FileCacherRead, SyncReadCommandBlock, (index));
#if SUPPORT_THREADS
    if (m_ReadCommandInProgress[index] == true)
    {
        m_ReadCompleteSemaphore[index].WaitForSignal();
        m_ReadCommandInProgress[index] = false;
    }
#endif
}

////////@TODO: Write a test for reading an invalid file where it runs out of bounds... or make the file operation fail...

void FileCacherRead::LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
{
    // Ensure that there is no recursive locking of cache blocks.
    for (int i = 0; i < kCacheCount; i++)
    {
        Assert(m_ActiveBlocks[i].locked == 0);
    }

    // Request this block
    int indexToLoad = RequestBlock(block);
    SyncReadCommandBlock(indexToLoad);

    // Return the active block
    m_ActiveBlocks[indexToLoad].locked = 1;
    *startPos = m_ActiveBlocks[indexToLoad].data;
    *endPos = m_ActiveBlocks[indexToLoad].data + m_ReadCommands[indexToLoad].size;

    if (m_PrefetchNextBlock)
    {
        // Use the other block to prefetch the next block
        int prefetchIndex = indexToLoad == 0 ? 1 : 0;
        int prefetchBlock = block + 1;
        if (m_ReadCommands[prefetchIndex].status != AsyncReadCommand::kInProgress && m_ActiveBlocks[prefetchIndex].block != prefetchBlock)
            Request(prefetchBlock, prefetchIndex, m_ActiveBlocks[prefetchIndex], false);
    }
}


void FileCacherRead::UnlockCacheBlock(size_t block)
{
    for (int i = 0; i < kCacheCount; i++)
    {
        if ((size_t)m_ActiveBlocks[i].block == block && m_ActiveBlocks[i].locked == 1)
        {
            m_ActiveBlocks[i].locked--;
            return;
        }
    }
}

std::string FileCacherRead::GetPathName() const
{
    return m_Path;
}