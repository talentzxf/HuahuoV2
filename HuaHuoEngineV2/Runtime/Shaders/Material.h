//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_MATERIAL_H
#define HUAHUOENGINE_MATERIAL_H
#include "Modules/ExportModules.h"
#include "TypeSystem/ObjectDefines.h"
#include "BaseClasses/NamedObject.h"

class EXPORT_COREMODULE Material: public NamedObject{
    REGISTER_CLASS(Material);
    DECLARE_OBJECT_SERIALIZE();
public:
    Material(MemLabelId label, ObjectCreationMode mode);
    // ~Material (); declared-by-macro

    virtual void Reset() override;

//    /// Set the shader and calls Reset
//    void ResetWithShader(Shader* shader);
};


#endif //HUAHUOENGINE_MATERIAL_H
