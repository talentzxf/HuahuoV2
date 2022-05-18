//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_RENDERLOOPENUMS_H
#define HUAHUOENGINE_RENDERLOOPENUMS_H

#pragma once

enum RenderingPath
{
    kRenderPathLegacyVertexLoop = 0, // "vertex lit"; used to be separate rendering loop but now just uses forward and disables some features (shadows etc.)
    kRenderPathForward,
    kRenderPathPrePass,
    kRenderPathDeferred,
    kRenderPathCount
};

enum OcclusionQueryType
{
    kOcclusionQueryTypeMostAccurate = 0,
    kOcclusionQueryTypeFastest,
    kOcclusionQueryTypeCount
};

// Match UnityEngine.Rendering.RenderQueue on C# side (GraphicsEnum.cs)
enum
{
    kBackgroundRenderQueue  = 1000,
    kGeometryRenderQueue    = 2000,
    kAlphaTestRenderQueue   = 2450, // we want it to be in the end of geometry queue
    kTransparentRenderQueue = 3000,
    kOverlayRenderQueue     = 4000,

    kQueueIndexMin = 0,
    kQueueIndexMax = 5000,

    kGeometryQueueIndexMin = kGeometryRenderQueue - 500,
    kGeometryQueueIndexMax = kGeometryRenderQueue + 500,
};

#endif //HUAHUOENGINE_RENDERLOOPENUMS_H
