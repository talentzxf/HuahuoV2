#include "RenderTextureMap.h"
#include "GfxDevice/GfxDevice.h"
#include "Memory/MemoryMacros.h"
//#include "Runtime/Threads/ThreadChecks.h"
//#include "Runtime/Threads/ReadWriteSpinLock.h"

RenderTextureMap::Map* RenderTextureMap::s_Map = NULL;

#define ENABLE_RENDER_TEXTURE_MAP_LOCK GFX_SUPPORTS_JOBIFIED_RENDERING
#if ENABLE_RENDER_TEXTURE_MAP_LOCK
static ReadWriteSpinLock s_RenderTextureMapLock;
#endif

void RenderTextureMap::Initialize()
{
    AssertMsg(s_Map == NULL, "Internal: RenderTextureMap::Initialize() should only be called once during initialization.");
    s_Map = HUAHUO_NEW_AS_ROOT(Map, kMemRenderer, "Rendering", "RenderTextureMap") ();
}

void RenderTextureMap::Cleanup()
{
    AssertMsg(s_Map->empty(), "Internal: Possible leak. All render texture surfaces should be removed from IdMap when destructed using RenderTextureMap::Remove(RenderSurfaceBase* rs).");
    HUAHUO_DELETE(s_Map, kMemRenderer);
}

void RenderTextureMap::Update(KeyType rs, RenderTexture* rt)
{
    if (!rs)
        return;

//#if ENABLE_RENDER_TEXTURE_MAP_LOCK
//    ReadWriteSpinLock::AutoWriteLock autoLock(s_RenderTextureMapLock);
//#else
//    DEBUG_ASSERT_RUNNING_ON_MAIN_THREAD;
//#endif

    Map::iterator it = s_Map->find(rs);
    if (it != s_Map->end())
        it->second = rt;
    else
        s_Map->insert(std::pair<KeyType, RenderTexture*>(rs, rt));
}

void RenderTextureMap::Remove(KeyType rs)
{
//#if ENABLE_RENDER_TEXTURE_MAP_LOCK
//    ReadWriteSpinLock::AutoWriteLock autoLock(s_RenderTextureMapLock);
//#else
//    DEBUG_ASSERT_RUNNING_ON_MAIN_THREAD;
//#endif

    s_Map->erase(rs);
}

RenderTexture* RenderTextureMap::Query(KeyType rs)
{
//#if ENABLE_RENDER_TEXTURE_MAP_LOCK
//    ReadWriteSpinLock::AutoReadLock autoLock(s_RenderTextureMapLock);
//#else
//    DEBUG_ASSERT_RUNNING_ON_MAIN_THREAD;
//#endif

    Map::iterator it = s_Map->find(rs);
    RenderTexture* rt = (it == s_Map->end()) ? 0 : it->second;

    return rt;
}
