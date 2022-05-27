#pragma once

#include "Math/Matrix4x4.h"
#include "BuiltinShaderParams.h"

struct TransformState
{
public:
    void Invalidate(BuiltinShaderParamValues& builtins);
    void UpdateWorldViewMatrix(const BuiltinShaderParamValues& builtins) const;
    void SetWorldMatrix(const Matrix4x4f& matrix);
    void SetViewMatrix(const Matrix4x4f& matrix, BuiltinShaderParamValues& builtins);
    void SetProjectionMatrix(const Matrix4x4f& matrix);
    void UpdateLateLatchWorldMatrixParams(int lateLatchFlags, SInt32 graphicsThreadLateLatchHierarchyIndex, int lateLatchIndex);

    Matrix4x4f worldMatrix;
    Matrix4x4f projectionMatrixOriginal; // Originally set from Unity code
    bool       lateLatchRecordingWorldMatrix;
    int        lateLatchFlags;
    SInt32     graphicsThreadLateLatchHierarchyIndex;
    int        lateLatchIndex;

    mutable Matrix4x4f worldViewMatrix; // Lazily updated in UpdateWorldViewMatrix()
    mutable bool worldViewMatrixDirty;
};

inline void TransformState::Invalidate(BuiltinShaderParamValues& builtins)
{
    worldViewMatrix.SetIdentity();
    worldMatrix.SetIdentity();
    builtins.GetWritableMatrixParam(kShaderMatView).SetIdentity();
    builtins.GetWritableMatrixParam(kShaderMatProj).SetIdentity();
    builtins.GetWritableMatrixParam(kShaderMatViewProj).SetIdentity();
    projectionMatrixOriginal.SetIdentity();
    worldViewMatrixDirty = false;
}

inline void TransformState::UpdateWorldViewMatrix(const BuiltinShaderParamValues& builtins) const
{
    if (worldViewMatrixDirty)
    {
        MultiplyMatrices4x4(&builtins.GetMatrixParam(kShaderMatView), &worldMatrix, &worldViewMatrix);
        worldViewMatrixDirty = false;
    }
}

inline void TransformState::SetWorldMatrix(const Matrix4x4f& matrix)
{
    worldViewMatrixDirty = true;
    worldMatrix = matrix;
    lateLatchRecordingWorldMatrix = false;
}

inline void TransformState::UpdateLateLatchWorldMatrixParams(int lateLatchFlagsParam, SInt32 graphicsThreadLateLatchHierarchyIndexParam, int lateLatchIndexParam)
{
    lateLatchRecordingWorldMatrix = true;
    lateLatchFlags = lateLatchFlagsParam;
    graphicsThreadLateLatchHierarchyIndex = graphicsThreadLateLatchHierarchyIndexParam;
    lateLatchIndex = lateLatchIndexParam;
}

inline void TransformState::SetViewMatrix(const Matrix4x4f& matrix, BuiltinShaderParamValues& builtins)
{
    worldViewMatrixDirty = true;
    Matrix4x4f& viewMat = builtins.GetWritableMatrixParam(kShaderMatView);
    Matrix4x4f& invViewMat = builtins.GetWritableMatrixParam(kShaderMatInvView);
    viewMat = matrix;
    InvertMatrix4x4_General3D(matrix.GetPtr(), invViewMat.GetPtr());
    worldMatrix.SetIdentity();
}

inline void TransformState::SetProjectionMatrix(const Matrix4x4f& matrix)
{
    projectionMatrixOriginal = matrix;
}
