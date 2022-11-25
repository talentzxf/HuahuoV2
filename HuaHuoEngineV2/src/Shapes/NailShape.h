//
// Created by VincentZhang on 2022-11-24.
//

#ifndef HUAHUOENGINEV2_NAILSHAPE_H
#define HUAHUOENGINEV2_NAILSHAPE_H
#include "BaseShape.h"
#include <algorithm>
#include <set>

class NailManager;

NailManager* GetNailManagerPtr();

class NailShape: public BaseShape{
    REGISTER_CLASS(NailShape);
    DECLARE_OBJECT_SERIALIZE()

public:
    NailShape(MemLabelId memLabelId, ObjectCreationMode mode)
    :Super(memLabelId, mode)
    {
    }

    virtual const char* GetTypeName() override{
        return "NailShape";
    }

    bool AddShape(BaseShape* targetShape, float localX, float localY, float localZ);

    bool ContainShape(BaseShape* shape){
        for(auto itr: boundShapes){
            if(itr->GetInstanceID() == shape->GetInstanceID())
                return true;
        }
        return false;
    }

    Vector3f* GetLocalPositionInShape(BaseShape* targetShape){
        if(!shapeLocalPointMap.contains(targetShape)){
            return NULL;
        }

        return &shapeLocalPointMap[targetShape];
    }

    int GetShapeCount(){
        return boundShapes.size();
    }

    BaseShape* GetShapeAtIndex(int index){
        return boundShapes[index];
    }

private:
    std::map<PPtr<BaseShape>, Vector3f> shapeLocalPointMap;
    std::vector<PPtr<BaseShape>> boundShapes;
};

class NailManager: public Object{
    REGISTER_CLASS(NailManager);
    DECLARE_OBJECT_SERIALIZE()
public:
    NailManager(MemLabelId memLabelId, ObjectCreationMode mode)
    :Super(memLabelId, mode)
    {

    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    /**
    * Return true, means passed. The nail can be created because there's no duplication
    * Return false, means failed. The nail can't be created because there's already a nail with these two shapes.
    * @param shape1
    * @param shape2
    */
    bool CheckDuplication(BaseShape* shape1, BaseShape* shape2){
        if(!this->shapeNailMap.contains(shape1) || !this->shapeNailMap.contains(shape2)){
            return true;
        }

        auto shape1NailSet = this->shapeNailMap[shape1];
        for(auto nail : shape1NailSet){
            if(nail->ContainShape(shape2))
                return false;
        }

        return true;
    }

    void AddNailShapeMapping(BaseShape* shape, NailShape* nailShape){
        if( std::find_if(nails.begin(), nails.end(), [nailShape](PPtr<NailShape> shape){
            return shape.GetInstanceID() == nailShape->GetInstanceID();
        }) == nails.end()){
            nails.push_back(nailShape);
        }

        shapeNailMap[shape].insert(nailShape);
    }

    void RemoveNail(NailShape* nailShape){
        std::remove_if(nails.begin(), nails.end(), [nailShape](PPtr<NailShape> shape){
            return nailShape->GetInstanceID() == shape.GetInstanceID();
        });

        for(auto itr : shapeNailMap){
            itr.second.erase(nailShape);
        }
    }

    static NailManager* GetNailManager();

    int GetNailCount(){
        return nails.size();
    }

    NailShape* GetNail(int index){
        if(index >= nails.size())
            return NULL;

        return nails[index];
    }

private:
    typedef std::set<PPtr<NailShape>> NailShapeSet;

    std::vector<PPtr<NailShape>> nails;
    std::map<PPtr<BaseShape>, NailShapeSet> shapeNailMap; // The map from baseShape->Array of nails.
};

#endif //HUAHUOENGINEV2_NAILSHAPE_H
