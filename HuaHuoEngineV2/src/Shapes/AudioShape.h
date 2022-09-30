//
// Created by VincentZhang on 6/29/2022.
//

#ifndef HUAHUOENGINEV2_AUDIOSHAPE_H
#define HUAHUOENGINEV2_AUDIOSHAPE_H


#include "AbstractMediaShape.h"

class AudioShape : public AbstractMediaShape{
    REGISTER_CLASS(AudioShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    AudioShape(MemLabelId label, ObjectCreationMode mode)
            :Super(label, mode)
    {
    }

    virtual const char* GetTypeName() override{
        return "AudioShape";
    }
};


#endif //HUAHUOENGINEV2_AUDIOSHAPE_H
