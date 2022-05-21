//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_SUBMESH_H
#define HUAHUOENGINE_SUBMESH_H
#pragma once

#include "Geometry/AABB.h"
#include "GfxDevice/GfxDeviceTypes.h"

struct SubMesh
{
    UInt32 trianglesFirstIndexByte;
    UInt32 trianglesIndexCount;
    AABB   localAABB;

    UInt32 firstIndexByte;
    UInt32 indexCount;
    GfxPrimitiveType topology;
    UInt32 baseVertex;
    UInt32 firstVertex;
    UInt32 vertexCount;

    SubMesh()
        : trianglesFirstIndexByte(0)
        , trianglesIndexCount(0)
        , localAABB(AABB::zero)
        , firstIndexByte(0)
        , indexCount(0)
        , topology(kPrimitiveTriangles)
        , baseVertex(0)
        , firstVertex(0)
        , vertexCount(0)
    {}

//    DrawBuffersRange ToDrawBuffersRange(bool tessellation) const
//    {
//        DrawBuffersRange drawRange;
//        drawRange.firstIndexByte = firstIndexByte;
//        drawRange.baseVertex = baseVertex;
//        drawRange.firstVertex = firstVertex;
//        drawRange.indexCount = indexCount;
//        drawRange.topology = topology;
//        drawRange.vertexCount = vertexCount;
//
//        // Fallback to use triangles if tessellation mode doesn't work with original primitives
//        if ((tessellation && topology == kPrimitiveTriangleStrip) ||
//            (!tessellation && topology == kPrimitiveQuads))
//        {
//            drawRange.topology = kPrimitiveTriangles;
//            drawRange.firstIndexByte = trianglesFirstIndexByte;
//            drawRange.indexCount = trianglesIndexCount;
//        }
//
//        return drawRange;
//    }

    DECLARE_SERIALIZE_NO_PPTR(SubMesh)
};

template<class TransferFunc>
void SubMesh::Transfer(TransferFunc& transfer)
{
    transfer.SetVersion(2);
    transfer.Transfer(firstIndexByte, "firstByte");
    TRANSFER(indexCount);
    TRANSFER_ENUM(topology);
    TRANSFER(baseVertex);
    TRANSFER(firstVertex);
    TRANSFER(vertexCount);
    TRANSFER(localAABB);
    if (transfer.IsOldVersion(1))
    {
        UInt32 triStrip;
        transfer.Transfer(triStrip, "isTriStrip");
        topology = triStrip ? kPrimitiveTriangleStrip : kPrimitiveTriangles;
    }
}

#endif //HUAHUOENGINE_SUBMESH_H
