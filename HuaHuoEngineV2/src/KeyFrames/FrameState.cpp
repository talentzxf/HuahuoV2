//
// Created by VincentZhang on 2022-06-16.
//

#include "FrameState.h"
#include "Shapes/BaseShape.h"

IMPLEMENT_REGISTER_CLASS(AbstractFrameState, 10007);

void AbstractFrameState::SetBaseShape(BaseShape *pBaseShape) {
    baseShape = pBaseShape;
}
