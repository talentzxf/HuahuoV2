//
// Created by VincentZhang on 5/21/2022.
//

#include "Material.h"

Material::Material(MemLabelId label, ObjectCreationMode mode)
        :   Super(label, mode)
#if ENABLE_VIRTUALTEXTURING
, m_BuildTextureStacks(label)
#endif
{
//    m_SharedMaterialData = NULL;
//    m_Shader = 0;
//    m_CustomRenderQueue = -1;
//    m_EnableInstancingVariants = false;
//    m_DoubleSidedGI = false;
//    m_LightmapFlags = kEmissiveIsBlack;
//    m_PerMaterialTextureDirty = true;
//    m_PerMaterialCBDirty = true;
//    m_CachedMainTexturePropertyNameId = kCachedPropertyNameIdUninitialized;
//    m_CachedMainColorPropertyNameId = kCachedPropertyNameIdUninitialized;

#if UNITY_EDITOR
    m_WasLoadedVer5OrEarlier = false;
#endif
}

void Material::Reset()
{
//    ResetWithShader(m_Shader);
}


//void Material::ResetWithShader(Shader* shader)
//{
//    Super::SmartReset();
//
//    if (!m_SharedMaterialData)
//        m_SharedMaterialData = UNITY_NEW(SharedMaterialData, kMemMaterial)(kMemMaterial);
//
//    ClearProperties();
//    m_SavedProperties = UnityPropertySheet();
//
//    m_Shader = GetWritableSharedMaterialData().shader = shader;
//    m_DoubleSidedGI = false;
//    m_EnableInstancingVariants = false;
//    if (!shader)
//    {
//        shader = Shader::GetDefault();
//        if (!shader)
//            return;
//    }
//    UpdateToNewShader(true);
//}


template<class TransferFunc>
void Material::Transfer(TransferFunc& transfer)
{
//    // We need to make sure that all of the properties are in the materials before they are written to disk (case 655473):
//    // In the editor we don't call BuildProperties at AwakeFromLoad due to performance / memory consumption (general preference of delay loading)
//    // Since BuildProperties adds properties to UnityPropertySheet, the result will be different depending on if BuildProperties has been called or not.
//    // Hence we always make sure it has been called at least once.
//    if (transfer.IsWritingGameReleaseData())
//        AddDefaultShaderPropertiesToSavedProperties();
//
//    transfer.SetVersion(6);
//    Super::Transfer(transfer);
//    TRANSFER(m_Shader);
//
//    // Shaderkeywords were introduced in 4.x as vector<string>.
//    // Now they are stored as a single string to reduce allocation overhead
//    // If we're using the YAML transfer backend and try to load an array as a string, we'll try to get the length of the string from an invalid union element (See yaml_node_s)
//    // So lets be sure to use the correct type, other case will be handled in DeprecatedTransfer at the bottom of this function
//    if (!transfer.IsVersionSmallerOrEqual(3))
//    {
//        TRANSFER(m_ShaderKeywords);
//    }
//
//    TRANSFER(m_LightmapFlags);
//
//    TRANSFER(m_EnableInstancingVariants);
//    TRANSFER(m_DoubleSidedGI);
//    transfer.Align();
//
//    TRANSFER(m_CustomRenderQueue);
//
//    // Since we don't know what ID:s our tags & pass names have, we need to store this as strings in the serialization.
//    // We make sure the string map is empty, translate the int map over to strings, transfer the string map and then translate back.
//    typedef UNITY_MAP (kMemTempAlloc, core::string, core::string) StringTagMap;
//    typedef UNITY_VECTOR (kMemTempAlloc, core::string) StringVector;
//    StringTagMap stringTagMap;
//    StringVector stringVector;
//    if (transfer.IsWriting() && m_SharedMaterialData)
//    {
//        const SharedMaterialData::TagMap& tagMap = m_SharedMaterialData->tagMap;
//        for (SharedMaterialData::TagMap::const_iterator iter = tagMap.begin(); iter != tagMap.end(); ++iter)
//            stringTagMap[shadertag::GetShaderTagName(iter->first)] = shadertag::GetShaderTagName(iter->second);
//        for (size_t i = 0, n = m_SharedMaterialData->disabledShaderPasses.size(); i != n; ++i)
//            stringVector.push_back(shadertag::GetShaderTagName(m_SharedMaterialData->disabledShaderPasses[i]));
//    }
//    transfer.Transfer(stringTagMap, "stringTagMap");
//    transfer.Transfer(stringVector, "disabledShaderPasses");
//    if (transfer.IsReading())
//    {
//        if (!m_SharedMaterialData)
//            m_SharedMaterialData = UNITY_NEW(SharedMaterialData, kMemMaterial)(kMemMaterial);
//        SharedMaterialData& sharedData = GetWritableSharedMaterialData();
//        sharedData.tagMap.clear();
//        for (StringTagMap::const_iterator iter = stringTagMap.begin(); iter != stringTagMap.end(); ++iter)
//            sharedData.tagMap[shadertag::GetShaderTagID(iter->first)] = shadertag::GetShaderTagID(iter->second);
//        sharedData.disabledShaderPasses.clear_dealloc();
//        for (size_t i = 0, n = stringVector.size(); i != n; ++i)
//            sharedData.disabledShaderPasses.push_back(shadertag::GetShaderTagID(stringVector[i]));
//    }
//
//    // Cull unused properties when making build!
//#if UNITY_EDITOR
//    if ((transfer.GetFlags() & kBuildPlayerOnlySerializeBuildProperties))
//    {
//        Shader *shader = m_Shader;
//        if (shader)
//        {
//            UnityPropertySheet tempProps = m_SavedProperties;
//            tempProps.CullUnusedProperties(shader->GetParsedForm());
//#if ENABLE_VIRTUALTEXTURING
//            const bool transferForDependencies = transfer.GetBuildingTarget().platform == kBuildNoTargetPlatform;
//            if (transferForDependencies || IsVirtualTexturingEnabled(transfer.GetBuildingTarget()))
//            {
//                // We want VT-only textures from a stacked texture stripped from the build, but we don't want to strip non VT-only textures.
//                // These should be included in the build, but they should be stripped from the material. So don't strip them when generating
//                // the dependency graph during player build so they remain in the dependency list, we do when serializing to disk.
//                bool forceStripAll = (transfer.GetFlags() & kSerializeGameRelease) && (transfer.GetFlags() & kReadWriteFromSerializedFile);
//                shader->StripTextureStackTextures(tempProps, forceStripAll);
//            }
//#endif
//
//            transfer.Transfer(tempProps, "m_SavedProperties");
//        }
//        else
//        {
//            UnityPropertySheet tempProps;
//            transfer.Transfer(tempProps, "m_SavedProperties");
//        }
//    }
//    else
//#endif
//    {
//        TRANSFER(m_SavedProperties);
//    }
//
//#if ENABLE_VIRTUALTEXTURING && UNITY_EDITOR
//    // Make sure we get the fresh BakedStacks from the VT manager
//    if (IsVirtualTexturingEnabled(transfer.GetBuildingTarget())
//        && transfer.IsWriting() && transfer.IsSerializingForGameRelease()
//        && (GetIVirtualTexturingManager()->IsPlayerBuilding() || GetIVirtualTexturingManager()->IsAssetBundleBuilding()))
//    {
//        Shader* shader = (Shader *)Object::IDToPointerThreadSafe(m_Shader.GetInstanceID());
//        if (shader)
//        {
//            m_BuildTextureStacks.resize_initialized(shader->GetNumTextureStacks());
//            PPtr<Texture> textures[ShaderTextureStack::MaxLayers];
//            for (int i = 0; i < shader->GetNumTextureStacks(); i++)
//            {
//                auto stack = shader->GetTextureStack(i);
//                stack.ExtractTextures(m_SavedProperties, textures);
//                m_BuildTextureStacks[i] = GetIVirtualTexturingManager()->IncludeTextureStackForPlayerBuild(textures, stack.GetNumLayers(), this);
//            }
//        }
//    }
//#endif
//    TRANSFER(m_BuildTextureStacks);
//
//    DeprecatedTransfer(transfer);
}


IMPLEMENT_REGISTER_CLASS(Material, 21);
IMPLEMENT_OBJECT_SERIALIZE(Material);
INSTANTIATE_TEMPLATE_TRANSFER(Material);