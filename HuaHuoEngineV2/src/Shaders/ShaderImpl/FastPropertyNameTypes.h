#pragma once

enum ShaderPropertyNameFlags
{
    kShaderPropInvalidIndex = ~0,
    kShaderPropBuiltinVectorMask = (int)(1 << 30),
    kShaderPropBuiltinMatrixMask = (int)(2 << 30),
    kShaderPropBuiltinTexEnvMask = (int)(3 << 30),
    kShaderPropBuiltinMask = (int)(3 << 30),
    kShaderPropBuiltinIndexMask = ~kShaderPropBuiltinMask,
};
