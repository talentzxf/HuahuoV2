//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_SHAREDMATERIALDATA_H
#define HUAHUOENGINE_SHAREDMATERIALDATA_H

#include "ShaderPropertySheet.h"
#include "ShaderKeywordSet.h"
#include "ShaderTags.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include <vector>
#include "Utilities/vector_map.h"

struct SharedMaterialData // : public ThreadSharedObject<SharedMaterialData>
{
    explicit SharedMaterialData(/*/MemLabelId label*/)
//            : ThreadSharedObject<SharedMaterialData>(label)
//            , shader(NULL)
//            , properties(label)
//            , shadowCasterHash(0)
//            , keywordSetHash(0)
//            , stateKeyHash(0)
//            , propertiesValid(false)
//            , enableInstancing(false)
//            , hashesDirty(false)
//            , smallMaterialIndex(0)
//            , disabledShaderPasses(label)
//            , perMaterialCB(NULL)
//            , userBufferOverride(false)
//            , perMaterialTextureDirty(false)
//            , perMaterialCBDirty(false)
//#if ENABLE_VIRTUALTEXTURING
//    , stacksDirty(false)
//        , textureStackHandles(label)
//#endif
    {
//        tagMap.clear();
    }

    SharedMaterialData(SharedMaterialData&& other) = delete;

    SharedMaterialData& operator=(const SharedMaterialData& other) = delete;
    SharedMaterialData& operator=(SharedMaterialData&& other) = delete;


    void    InvalidatePerMaterialCBMainThread();
//    void    UpdateTextureIDList(const Shader* shader);
//    void    UpdatePerMaterialCB(const Shader* shader);

//    const Shader*                   shader;

    ShaderPropertySheet             properties;
    ShaderKeywordSet                shaderKeywordSet;

    // Hash of material property values that affect shadow caster pass (0 if no shadow caster pass).
    // This is used in shadow caster batching, so that casters with different materials but same
    // shaders (and nothing different that affects shadow caster pass) can be batched together.
    UInt32                          shadowCasterHash;

    // Hash of material property values that affect fixed function state blocks (e.g. blending state
    // in the shader). Different hashes might cause different underlying GfxDevice state objects
    // to be created.
    UInt32                          stateKeyHash;

    // Hash of the material keyword set ( in SRPBatcher context, only this hash is important to sort objects )
    UInt32                          keywordSetHash;

    UInt32                          propertiesValid : 1;
    UInt32                          enableInstancing : 1;

    // True when m_ShadowCasterHash or m_StateKeyHash needs to be recomputed; make sure
    // UpdateHashesIfNeeded is called before using them. Hashes are invalidated if a new shader variant
    // is compiled/loaded (might result in new properties affecting hashes); or a material property
    // changes that affects them.
    UInt32                          hashesDirty     : 1;

    //@TODO: Later make this value actually a smallMaterialIndex instead of
    int                             smallMaterialIndex;

    // Shader pass types that are disabled in the material (e.g. even if shader supports MotionVectors pass,
    // we can make it pretend to not be supported per-material).
    std::vector<ShaderTagID>      disabledShaderPasses;

    // per-material tag overrides (can override what the shader tags have specified)
    typedef vector_map<ShaderTagID, ShaderTagID> TagMap;
    TagMap tagMap;

    TextureID               texturesIdUnionCache[kMaxSupportedTextureUnits];
    // GfxBuffer*              perMaterialCB;
    bool                    perMaterialTextureDirty;
    bool                    perMaterialCBDirty;
    bool                    userBufferOverride;

#if GFX_SUPPORTS_DISPLAY_LISTS
    // MaterialDisplayListCache maintains this data and is responsible for thread safety.
    mutable MaterialDisplayListCache::MaterialDisplayLists          cachedMaterialDisplayLists;
#endif

#if ENABLE_VIRTUALTEXTURING
    dynamic_array<VTTextureStackID> textureStackHandles;
    bool stacksDirty;

    void SetTextureStackProperties(int index, const VirtualTexturingShaderParameters &params);
    void ClearTextureStackHandles();
    void DirtyTextureStacks() { stacksDirty = true; }
    bool AreTextureStacksDirty() const;
    VTTextureStackID FindTextureStack(ShaderLab::FastPropertyName stackName) const;
    void UpdateTextureStackProperties();

#if UNITY_EDITOR
    void SyncTextureStackHandlesAndProperties(Material* owner);
    void SyncTextureStack(int index, Material* owner);
    void ValidateTextureStacks(dynamic_array<VirtualTexturingStackValidationResult>& errorMessages, Material* owner) const;
#else
    void InitializeFromBuildStacks(const dynamic_array<BuildTextureStackReference> &serializedStacks, char const* errorLoggableMaterialName);
    void DuplicateTextureStackHandles(const SharedMaterialData &other);
#endif
#endif

};
#endif //HUAHUOENGINE_SHAREDMATERIALDATA_H
