//
// Created by VincentZhang on 5/16/2022.
//

#include "GraphicsSettings.h"
#include "RenderManager.h"
#include "BaseClasses/ManagerContext.h"

GraphicsSettings::GraphicsSettings(/*MemLabelId label,*/ ObjectCreationMode mode)
        :   Super(/*label,*/ mode)
//#if UNITY_EDITOR
//        ,   m_NeedToInitializeDefaultShaderReferences(false)
//    ,   m_NeedToInitializeDefaultuGUIShaders(false)
//#endif
//        ,   m_VideoShadersIncludeMode(kVideoShadersAlwaysInclude)
//        ,   m_LightsUseLinearIntensity(false)
//        ,   m_LightsUseColorTemperature(false)
//        ,   m_DefaultRenderingLayerMask(1)
//        ,   m_RealtimeDirectRectangularAreaLights(false)
//        ,   m_LogWhenShaderIsCompiled(false)
//        ,   m_DisableBuiltinCustomRenderTextureUpdate(false)
{
#if UNITY_EDITOR
    m_EditorOnly.Reset();
    // We want to fill in default shader references, but at construction
    // time resource files aren't loaded yet. So set up a flag
    // that we'll want to fill them in later. Unless we already have
    // serialized data for graphics settings in the project;
    // then we'll unset the flag in transfer function.
    m_NeedToInitializeDefaultShaderReferences = true;
#endif

//    ::memset(&m_TierSettings, 0x00, sizeof(m_TierSettings));
//    m_TransparencySortMode = Camera::kTransparencySortDefault;
//    m_TransparencySortAxis = Vector3f(0.0f, 0.0f, 1.0f);
}

void GraphicsSettings::InitializeClass()
{
    RenderManager::InitializeClass();
#if UNITY_EDITOR
    RegisterAllowNameConversion("GraphicsSettings", "m_BuildTargetShaderSettings", "m_TierSettings");
    RegisterAllowNameConversion("GraphicsSettings", "m_ShaderSettings_Tier1", "m_TierSettings_Tier1");
    RegisterAllowNameConversion("GraphicsSettings", "m_ShaderSettings_Tier2", "m_TierSettings_Tier2");
    RegisterAllowNameConversion("GraphicsSettings", "m_ShaderSettings_Tier3", "m_TierSettings_Tier3");

    // this is for EditorOnlyGraphicsSettings::TierSettings, it will use TierSettings name for serialization
    RegisterAllowNameConversion("TierSettings", "m_ShaderSettings", "m_Settings");
#endif
}

void GraphicsSettings::CleanupClass()
{
    RenderManager::CleanupClass();
}

template<class TransferFunction>
void GraphicsSettings::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    transfer.SetVersion(13);

//    transfer.Transfer(m_Deferred, "m_Deferred");
//    transfer.Transfer(m_DeferredReflections, "m_DeferredReflections");
//    transfer.Transfer(m_ScreenSpaceShadows, "m_ScreenSpaceShadows");
//    transfer.Transfer(m_LegacyDeferred, "m_LegacyDeferred");
//    transfer.Transfer(m_DepthNormals, "m_DepthNormals");
//    transfer.Transfer(m_MotionVectors, "m_MotionVectors");
//    transfer.Transfer(m_LightHalo, "m_LightHalo");
//    transfer.Transfer(m_LensFlare, "m_LensFlare");
//    TRANSFER_ENUM(m_VideoShadersIncludeMode);
//#if UNITY_EDITOR
//    if (transfer.IsSerializingForGameRelease())
//    {
//        // Internal shaders are added to user-specified always included shaders when doing a build,
//        // but not when loading/saving GraphicsSettings in the editor.
//        ShaderArray alwaysIncludedAndInternalShaders(m_AlwaysIncludedShaders);
//        AddHiddenInternalShaders(alwaysIncludedAndInternalShaders);
//        transfer.Transfer(alwaysIncludedAndInternalShaders, "m_AlwaysIncludedShaders");
//    }
//    else
//#endif
//    {
//        transfer.Transfer(m_AlwaysIncludedShaders, "m_AlwaysIncludedShaders");
//    }
//
//    transfer.Transfer(m_PreloadedShaders, "m_PreloadedShaders");
//
//    transfer.Transfer(m_SpritesDefaultMaterial, "m_SpritesDefaultMaterial");
//    TRANSFER(m_CustomRenderPipeline);
//    TRANSFER(m_TransparencySortMode);
//    TRANSFER(m_TransparencySortAxis);
//
//    // in editor we should serialize m_TierSettings only when building game
//    // otherwise, as we change it every time we switch platform, VCS will be not happy
//    if (transfer.IsSerializingForGameRelease())
//    {
//        TRANSFER_WITH_NAME(m_TierSettings[0], "m_TierSettings_Tier1");
//        TRANSFER_WITH_NAME(m_TierSettings[1], "m_TierSettings_Tier2");
//        TRANSFER_WITH_NAME(m_TierSettings[2], "m_TierSettings_Tier3");
//        CompileTimeAssert(kGraphicsTierCount == 3, "Update GraphicsSettings::Transfer if you change tier count");
//    }
//
//    if (transfer.IsSerializingForGameRelease())
//    {
//        // if we build final game we re-init shader defines with target platform
//#if UNITY_EDITOR
//        CollectShaderDefinesForCurrentBuildTarget(&m_ShaderDefinesPerShaderCompiler, false);
//#endif
//        TRANSFER(m_ShaderDefinesPerShaderCompiler);
//    }
//
//
//#if UNITY_EDITOR
//    if (!transfer.IsSerializingForGameRelease())
//    {
//        m_EditorOnly.Transfer(transfer);
//        TransferDeprecated(transfer);
//    }
//#endif
//
//
//#if UNITY_EDITOR
//    // We are reading data from existing shader file; no need
//    // to go later and overwrite it with default set of shaders.
//    if (transfer.IsReading())
//        m_NeedToInitializeDefaultShaderReferences = false;
//    if (transfer.IsVersionSmallerOrEqual(1))
//        m_NeedToInitializeDefaultuGUIShaders = true;
//
//    // if Editor is not yet fully inited (build settings are not loaded) we defer TierSettings/Default-Shaders initialization to Application::InitializeProject
//    //   this happens when we load old project with newer editor
//    // if Editor is already inited and build settings are available we init TierSettings/Default-Shaders right away to avoid broken behavior until project is reloaded
//    //   this happens when we import old unity package with newer editor during editor lifetime
//    bool areBuildSettingReady = (GetEditorUserBuildSettingsPtr() != NULL);
//    if (transfer.IsVersionSmallerOrEqual(2))
//    {
//        if (areBuildSettingReady)
//            SetDefaultShaderReferences();
//        else
//            m_NeedToInitializeDefaultShaderReferences = true;
//    }
//    if (transfer.IsReading() && areBuildSettingReady)
//        UpdateTierSettingsForCurrentBuildTarget();
//
//#endif // #if UNITY_EDITOR
//
//    TRANSFER_PROPERTY(bool, m_LightsUseLinearIntensity, GetLightsUseLinearIntensity, SetLightsUseLinearIntensity);
//    TRANSFER_PROPERTY(bool, m_LightsUseColorTemperature, GetLightsUseColorTemperature, SetLightsUseColorTemperature);
//    transfer.Align();
//    TRANSFER_PROPERTY(UInt32, m_DefaultRenderingLayerMask, GetDefaultRenderingLayerMask, SetDefaultRenderingLayerMask);
//
//    // Unity 5.6 added ColorTemperature and linear intensity. Don't use on old projects as relighting would be required.
//    if (transfer.IsVersionSmallerOrEqual(10))
//    {
//        m_LightsUseLinearIntensity = false;
//        m_LightsUseColorTemperature = false;
//    }
//
//    TRANSFER(m_LogWhenShaderIsCompiled);
}

IMPLEMENT_REGISTER_CLASS(GraphicsSettings, 10);
IMPLEMENT_OBJECT_SERIALIZE(GraphicsSettings);
GET_MANAGER(GraphicsSettings)
GET_MANAGER_PTR(GraphicsSettings)
