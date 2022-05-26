//
// Created by VincentZhang on 5/23/2022.
//

#ifndef HUAHUOENGINE_LINE2D_H
#define HUAHUOENGINE_LINE2D_H
#include "Base2DObject.h"
#include "Math/Vector3f.h"
#include "BaseClasses/PPtr.h"

struct LinePoint{
    Vector3f position;
    bool isShadow;
};

class Line2D : public Base2DObject{
    REGISTER_CLASS(Line2D);
    DECLARE_OBJECT_SERIALIZE();
public:
    Line2D(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {

    }

    virtual void MainThreadCleanup() override;

    void AddPoint(Vector3f* point, bool isShadow){
        LinePoint* newPoint = HUAHUO_NEW(LinePoint, kMemRenderer);
        points.push_back(newPoint);
    }

private:
    std::vector<PPtr<LinePoint>> points;
};


#endif //HUAHUOENGINE_LINE2D_H
