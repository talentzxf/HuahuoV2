//
// Created by VincentZhang on 5/27/2022.
//

#ifndef HUAHUOENGINE_BUILTINSHADERPARAMS_H
#define HUAHUOENGINE_BUILTINSHADERPARAMS_H

#include "Math/Vector4f.h"
#include "Math/Matrix4x4.h"
#include "Shaders/ShaderImpl/ShaderTextureProperty.h"
#include "BuiltinShaderParamsNames.h"

class BuiltinShaderParamValues {
public:
    BuiltinShaderParamValues();

    inline Matrix4x4f&        GetWritableMatrixParam(BuiltinShaderMatrixParam param)           { DebugAssert(param >= 0 && param < kShaderMatCount); isDirty = true; return matrixParamValues[param]; }
    inline const Matrix4x4f&          GetMatrixParam(BuiltinShaderMatrixParam param) const     { DebugAssert(param >= 0 && param < kShaderMatCount); return matrixParamValues[param]; }
    inline Vector4f&          GetWritableVectorParam(BuiltinShaderVectorParam param)          { DebugAssert(param >= 0 && param < kShaderVecCount); isDirty = true; return vectorParamValues[param]; }
    inline void   SetVectorParam(BuiltinShaderVectorParam param, const Vector4f& val)          { GetWritableVectorParam(param) = val; }
    inline void   SetMatrixParam(BuiltinShaderMatrixParam param, const Matrix4x4f& val)        { GetWritableMatrixParam(param) = val; }
    bool                isDirty;
private:
    Vector4f            vectorParamValues[kShaderVecCount];
    Matrix4x4f          matrixParamValues[kShaderMatCount];
    ShaderLab::TexEnv   texEnvParamValues[kShaderTexEnvCount];
};

#endif //HUAHUOENGINE_BUILTINSHADERPARAMS_H
