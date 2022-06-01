//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_SHADERPROPERTYSHEET_H
#define HUAHUOENGINE_SHADERPROPERTYSHEET_H


#include "Memory/AllocatorLabels.h"
#include "Core/SharedObject.h"
#include "Math/Matrix4x4.h"
#include "Shaders/ShaderImpl/FastPropertyName.h"

class ShaderPropertySheet : public ThreadSharedObject<ShaderPropertySheet>{
public:
    enum SetFlags
    {
        kDefault = 0,
        kForceSRGB = 1 << 0,
        kNewProperty = 1 << 1
    };
    typedef ShaderLab::FastPropertyName PropertyName;
public:
    ShaderPropertySheet(MemLabelRef memoryLabel)
            : ThreadSharedObject<ShaderPropertySheet>(memoryLabel)
//            , m_Names(memoryLabel)
//            , m_Descs(memoryLabel)
//            , m_ValueBuffer(memoryLabel)
//            , m_Hash(0)
//            , m_LayoutHash(0)
//            , m_AllowAddProperty(true)
    {
//        memset(m_TypeStartIndex, 0, sizeof(m_TypeStartIndex));
    }

    void SetMatrix(PropertyName name, const Matrix4x4f& val, SetFlags flags = kDefault);
};


#endif //HUAHUOENGINE_SHADERPROPERTYSHEET_H
