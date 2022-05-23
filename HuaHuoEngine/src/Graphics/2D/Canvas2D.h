//
// Created by VincentZhang on 5/23/2022.
//

#ifndef HUAHUOENGINE_CANVAS2D_H
#define HUAHUOENGINE_CANVAS2D_H
#include "Graphics/Renderer.h"
#include "Objects2D/Line2D.h"

class Canvas2DRenderer : public Renderer {
    REGISTER_CLASS(Canvas2DRenderer);
    DECLARE_OBJECT_SERIALIZE();
public:
    Canvas2DRenderer(MemLabelId label, ObjectCreationMode mode);

    void AddLine(Line2D* newLine) {
        object2dArray.push_back(newLine);
    }

private:
    std::vector<Base2DObject*> object2dArray;
};


#endif //HUAHUOENGINE_CANVAS2D_H
