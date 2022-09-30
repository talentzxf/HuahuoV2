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
    ,mStoreId(-1)
    {

    }

    virtual const char* GetTypeName() override{
        return "ElementShape";
    }

    SInt32 GetElementStoreId(){
        return mStoreId;
    }

    void SetElementStoreId(SInt32 storeId){
        mStoreId = storeId;
    }

    virtual void Apply(int frameId) override;

private:
    SInt32 mStoreId;

};


#endif //HUAHUOENGINEV2_ELEMENTSHAPE_H
