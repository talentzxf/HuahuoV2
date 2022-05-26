//
// Created by VincentZhang on 5/23/2022.
//

#include "Line2D.h"


void Line2D::MainThreadCleanup(){
    LinePoint* newPoint = HUAHUO_NEW(LinePoint, kMemRenderer);

    for(PPtr<LinePoint> point : points){
        LinePoint* pointPtr = point;
        HUAHUO_DELETE(pointPtr, kMemRenderer);
    }

    points.clear();
}