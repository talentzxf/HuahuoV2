//
// Created by VincentZhang on 5/27/2022.
//

#ifndef HUAHUOENGINE_SHADERKEYWORDS_H
#define HUAHUOENGINE_SHADERKEYWORDS_H


namespace keywords
{
    // Builtin keyword enums
    enum
    {
        // Light keywords
        kLightSpot = 0,
        kLightDirectional,
        kLightDirectionalCookie,
        kLightPoint,
        kLightPointCookie,
        kLightKeywordCount,

        // Shadow keywords
        kShadowsDepth = 5,
        kShadowsScreen,
        kShadowsCube,
        kShadowsSoft,
        kShadowsSplitSpheres,
        kShadowsSingleCascade,

        // lightmapping keywords
        kLightmapOn,
        kLightmapDirectionalCombined,
        kLightmapDynamicOn,
        kLightmapShadowMixing,
        kShadowShadowMask,

        kLightProbeSH,

        // fog keywords
        kFogLinear,
        kFogExp,
        kFogExp2,

        // other builtin keywords
        kEmissionMap,
        kVertexLightOn,
        kSoftParticlesOn,
        kHDROn,
        kLODFadeCrossfade,

        kInstancingOn,
        kProceduralInstancingOn,
        kDOTSInstancingOn,
        kSinglePassStereo,

        kETC1ExternalAlpha,

        kStereoInstancingOn,
        kStereoMultiviewOn,
        kStereoCubemapRenderOn,

        kEditorVisualization,

        kBuiltinKeywordCount,

        kAllLightKeywordsMask = 0x1F
    };

}


#endif //HUAHUOENGINE_SHADERKEYWORDS_H
