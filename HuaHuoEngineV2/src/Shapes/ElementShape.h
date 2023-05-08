//
// Created by VincentZhang on 7/1/2022.
//

#ifndef HUAHUOENGINEV2_ELEMENTSHAPE_H
#define HUAHUOENGINEV2_ELEMENTSHAPE_H


#include "BaseShape.h"

class ElementShape : public BaseShape{
    REGISTER_CLASS(ElementShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    ElementShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    {

    }

    virtual const char* GetTypeName() override{
        return "ElementShape";
    }

    const char* GetElementStoreId(){
        return GUIDToString(mStoreId).c_str();
    }

    void SetElementStoreId(const char* storeId){
        mStoreId = StringToGUID(storeId);
    }

    virtual void Apply(int frameId) override;

private:
    HuaHuoGUID mStoreId;
};


#endif //HUAHUOENGINEV2_ELEMENTSHAPE_H
