#pragma once

// Each new renderer should add a new enum here.
enum RendererType
{
    kRendererUnknown,

    kRendererMesh,
    kRendererSkinnedMesh,
    kRendererSprite,
    kRendererTilemap,

    kRendererTrail,
    kRendererLine,
    kRendererParticleSystem,
    kRendererBillboard,

    kRendererSpriteMask,
    kRendererSpriteShape,
    kRendererVFX,

    // Only add IntermediateRenderer types below
    kRendererIntermediate,
    kRendererBatchRendererGroup,
    kRendererSpriteGroup,
    kRendererQuadTreeNodeBatched,

    kRendererIntermediateInstancedBatched,

    kRendererTypeCount,

    kRenderer2D,

    //@TODO: Why are intermediate renderers handled special???
    //  kRendererTypeMaxCount = 32
    kRendererTypeBitSize = 6
};

extern const char* gRendererTypeNames[]; // Defined in Renderer.cpp
