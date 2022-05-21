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