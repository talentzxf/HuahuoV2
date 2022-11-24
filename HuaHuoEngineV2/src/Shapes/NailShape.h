//
// Created by VincentZhang on 2022-11-24.
//

#ifndef HUAHUOENGINEV2_NAILSHAPE_H
#define HUAHUOENGINEV2_NAILSHAPE_H
#include "BaseShape.h"
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
        return boundShapes.contains(shape);
    }

private:
    std::map<PPtr<BaseShape>, Vector3f> shapeLocalPointMap;
    std::set<PPtr<BaseShape>> boundShapes;

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

    NailShape* CreateNail(){
        NailShape* newNail = Object::Produce<NailShape>();
        GetPersistentManagerPtr()->MakeObjectPersistent(newNail->GetInstanceID(), StoreFilePath);
        return newNail;
    }

    void AddNailShapeMapping(BaseShape* shape, NailShape* nailShape){
        shapeNailMap[shape].insert(nailShape);
    }

    void RemoveNail(NailShape* nailShape){
        nails.erase(nailShape);
        for(auto itr : shapeNailMap){
            itr.second.erase(nailShape);
        }
    }

private:
    typedef std::set<PPtr<NailShape>> NailShapeSet;

    std::set<PPtr<NailShape>> nails;
    std::map<PPtr<BaseShape>, NailShapeSet> shapeNailMap; // The map from baseShape->Array of nails.
};


#endif //HUAHUOENGINEV2_NAILSHAPE_H
