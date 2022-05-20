//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_CULLINGPARAMETERS_H
#define HUAHUOENGINE_CULLINGPARAMETERS_H

#include "SceneNode.h"
#include "Logging/LogAssert.h"
#include "BaseClasses/TagTypes.h"
#include "Geometry/Plane.h"

class AABB;

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

typedef void PostProcessCullResults (const SceneNode* nodes, const AABB* bounds, IndexList& list, void* userData);

#endif //HUAHUOENGINE_CULLINGPARAMETERS_H
