//
// Created by VincentZhang on 2022-08-12.
//

#ifndef HUAHUOENGINEV2_TEXTSHAPE_H
#define HUAHUOENGINEV2_TEXTSHAPE_H


#include "TypeSystem/ObjectDefines.h"
#include "BaseShape.h"

class TextShape: public BaseShape{
    REGISTER_CLASS(TextShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    TextShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    {

    }

    virtual const char* GetTypeName() override{
        return "TextShape";
    }

    void SetText(const char* inText){
        this->text = inText;
    }

    char* GetText(){
        return const_cast<char *>(text.c_str());
    }

private:
    std::string text;
};


#endif //HUAHUOENGINEV2_TEXTSHAPE_H
