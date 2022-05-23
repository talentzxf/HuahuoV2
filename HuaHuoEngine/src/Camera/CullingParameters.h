//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_CULLINGPARAMETERS_H
#define HUAHUOENGINE_CULLINGPARAMETERS_H

#include "SceneNode.h"
#include "Logging/LogAssert.h"
#include "BaseClasses/TagTypes.h"
#include "Geometry/Plane.h"
#include "Math/Matrix4x4.h"
// #include "BatchRendererGroup.h"

class AABB;
namespace Umbra { class DebugRenderer; class Visibility; class Tome; class QueryExt; }

enum CullFiltering { kFilterModeOff = 0,       kFilterModeShowFiltered = 1 };

enum CameraType
{
    kCameraTypeGame         = 1 << 0,
    kCameraTypeSceneView    = 1 << 1,
    kCameraTypePreview      = 1 << 2,
    kCameraTypeVR           = 1 << 3,
    kCameraTypeReflection   = 1 << 4
};

struct IndexList
{
    int* indices;
    int  size;
    int  reservedSize;

    int& operator[](int index)
    {
        DebugAssert(index < reservedSize);
        return indices[index];
    }

    int operator[](int index) const
    {
        DebugAssert(index < reservedSize);
        return indices[index];
    }

    IndexList()
    {
        indices = NULL;
        reservedSize = size = 0;
    }

    IndexList(int* i, int sz, int rs)
    {
        indices = i;
        size = sz;
        reservedSize = rs;
    }

    void operator=(const IndexList& in)
    {
        indices = in.indices;
        size = in.size;
        reservedSize = in.reservedSize;
    }
};

struct LODParameters
{
    int         isOrthographic;
    Vector3f    cameraPosition;
    float       fieldOfView;            // Vertical field of view (in degrees)
    float       orthoSize;
    int         cameraPixelHeight;
};

//NOTE: this must be kept in sync with
//      Managed CullingParameters / ScriptableCullingParameters
struct CullingParameters
{
    CullingParameters() = default;
    enum { kMaxPlanes = 10 };
    enum { kOptimizedCullingPlaneCount = 12 };

    enum LayerCull
    {
        kLayerCullNone,
        kLayerCullPlanar,
        kLayerCullSpherical
    };

    int             isOrthographic; // bool for compatibility with script bindings
    LODParameters   lodParams;
    Plane           cullingPlanes[kMaxPlanes]; // Note: do not write to cullingPlanes directly. Use SetCullingPlanes
    int             cullingPlaneCount;
    UInt32          cullingMask;
    UInt64          sceneMask;

    // Note: not supported in scriptable renderloops
    float           layerFarCullDistances[kNumLayers];
    LayerCull       layerCull;

    // Used for Umbra
    Matrix4x4f cullingMatrix;
    Vector3f  position;
};

// Output for all culling operations.
// simple index list indexing into the different places where renderers can be found.
struct CullingOutput
{
    IndexList*          visible;
    int                 totalVisibleListsCount;  // this includes all the standard renderers & the new custom ones added after kStandardVisibleListCount

    bool                useUmbraOcclusionCulling;
    Umbra::Visibility*  umbraVisibility;

    // BatchRendererCullingOutputs* batchRendererCullingOutputs;

    CullingOutput() : useUmbraOcclusionCulling(false), umbraVisibility(NULL)/*, batchRendererCullingOutputs(NULL)*/, visible(NULL), totalVisibleListsCount(0)  {}
};

enum
{
    kStaticRenderers = 0,
    kDynamicRenderer,
    kSceneIntermediate,
    kCameraIntermediate,
    kCustomCullRenderers,
    kSpriteGroup,
    kStandardVisibleListCount,

    // Trees & BatchRenderGroups are dynamic ones and placed after.
};

struct RendererCullData
{
    const AABB*      bounds;
    const SceneNode* nodes;
    size_t           rendererCount;

    RendererCullData() { bounds = NULL; nodes = NULL; rendererCount = 0; }
};

struct LODDataArray
{
    UInt8* masks;
    float* fades;
    size_t count;
};

typedef void PostProcessCullResults (const SceneNode* nodes, const AABB* bounds, IndexList& list, void* userData);


struct SceneCullingParameters : CullingParameters
{
    SceneCullingParameters()
            : lodDataArrays(NULL)
            , excludeLightmappedShadowCasters(false)
            , cullLights(false)
            , cullReflectionProbes(false)
            , stereo(false)
            , computeShadowCasterBounds(false)
            , enablePerObjectCulling(true)
            , includeShadowCasters(true)
            , renderPath(0)
            , filterMode(kFilterModeOff)
            , stereoSeparation(0.0f)
            , sceneVisbilityForShadowCulling(NULL)
            , umbraTome(NULL)
            , umbraGateState(NULL)
            , umbraDebugRenderer(NULL)
            , umbraDebugFlags(0)
            , umbraMaximumPortalJobCount(6)
            , postProcessCullResults(NULL)
            , postProcessCullResultsUserData(NULL)
            , accurateOcclusionThreshold(-1.f)
            , renderers(NULL)
            , totalRendererListsCount(0)
            , maximumVisibleLights(-1)
    {}

    RendererCullData *renderers;
    int              totalRendererListsCount;

    LODDataArray*   lodDataArrays;

    bool            excludeLightmappedShadowCasters;
    bool            cullLights;
    bool            cullReflectionProbes;
    bool            stereo;
    bool            computeShadowCasterBounds;
    bool            enablePerObjectCulling;
    bool            includeShadowCasters;

    int             renderPath;
    CullFiltering   filterMode;

    // used for stereo
    float           stereoSeparation;
    Matrix4x4f      stereoCombinedProj;
    Matrix4x4f      stereoCombinedView;

    /// This stores the visibility of previous culling operations.
    /// For example, shadow caster culling uses the visibility of the visible objects from the camera.
    const CullingOutput*    sceneVisbilityForShadowCulling;

    const Umbra::Tome*      umbraTome;
    const void*             umbraGateState;
    Umbra::DebugRenderer*   umbraDebugRenderer;
    UInt32                  umbraDebugFlags;
    UInt32                  umbraMaximumPortalJobCount;

    PostProcessCullResults* postProcessCullResults;
    void*                   postProcessCullResultsUserData;

    float accurateOcclusionThreshold;
    int maximumVisibleLights;
};
#endif //HUAHUOENGINE_CULLINGPARAMETERS_H
