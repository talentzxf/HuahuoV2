//
// Created by VincentZhang on 6/27/2022.
//

#ifndef HUAHUOENGINEV2_IMAGESHAPE_H
#define HUAHUOENGINEV2_IMAGESHAPE_H


#include "BaseShape.h"
#include "AbstractMediaShape.h"

class ImageShape : public AbstractMediaShape{
    REGISTER_CLASS(ImageShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    ImageShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    ,mIsAnimation(false)
    {
    }

    virtual const char* GetTypeName() override{
        return "ImageShape";
    }

    void SetIsAnimation(bool isAnimation){
        this->mIsAnimation = isAnimation;
    }

    bool GetIsAnimation(){
        return this->mIsAnimation;
    }
private:
    bool mIsAnimation;
};


#endif //HUAHUOENGINEV2_IMAGESHAPE_H
