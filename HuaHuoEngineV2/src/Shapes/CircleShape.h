//
// Created by VincentZhang on 6/12/2022.
//

#ifndef HUAHUOENGINEV2_CIRCLESHAPE_H
#define HUAHUOENGINEV2_CIRCLESHAPE_H
#include "BaseShape.h"
#include "Math/Vector3f.h"

class CircleShape : public BaseShape{
    REGISTER_CLASS(CircleShape);
    DECLARE_OBJECT_SERIALIZE()
public:
    CircleShape(MemLabelId label, ObjectCreationMode mode)
    :Super(label, mode)
    {

    }

    virtual char* GetName() override{
        return "CircleShape";
    }

    Vector3f* GetCenter(){
        return &m_Center;
    }

    float GetRadius(){
        return m_Radius;
    }

    void SetCenter(float x, float y, float z){
        m_Center.x = x;
        m_Center.y = y;
        m_Center.z = z;
    }

    void SetRadius(float radius){
        m_Radius = radius;
    }
private:
    Vector3f m_Center;
    float m_Radius;
};


#endif //HUAHUOENGINEV2_CIRCLESHAPE_H
