#pragma once

#include "Utilities/HashFunctions.h"
#include <map>

struct RenderSurfaceBase;
class RenderTexture;

class RenderTextureMap
{
public:
    typedef const RenderSurfaceBase* KeyType;

    static void Initialize();
    static void Cleanup();

    static void Update(KeyType rs, RenderTexture* rt);
    static void Remove(KeyType rs);

    static RenderTexture* Query(KeyType rs);

private:
    typedef std::map<KeyType, RenderTexture*, PointerHashFunction<KeyType> > Map;

    static Map* s_Map;
};
