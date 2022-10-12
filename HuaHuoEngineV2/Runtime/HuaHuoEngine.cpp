//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"
#include "Serialize/PathNamePersistentManager.h"
#include "BaseClasses/MessageHandler.h"
#include "Layer.h"

#ifdef HUAHUO_EDITOR

#include "BaseClasses/ManagerContextLoading.h"
#include "BaseClasses/ManagerContext.h"
#include "File/AsyncReadManager.h"

#endif

void HuaHuoEngine::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();

    MessageHandler::Get().Initialize(TypeOf<Object>());
    TypeManager::Get().CallInitializeTypes();

    TypeManager::Get().CallPostInitializeTypes();
    MessageHandler::Get().ResolveCallbacks();

    ManagerContextInitializeClasses();
#ifdef HUAHUO_EDITOR
    CreateMissingGlobalGameManagers();
#endif

    InitPathNamePersistentManager();
    InitializeAsyncReadManager();

}

HuaHuoEngine *HuaHuoEngine::gInstance = new HuaHuoEngine();

HuaHuoEngine::HuaHuoEngine() {
}

// TODO: Make this as lambda? But Cpp's lambda is really very mysterious.
class LayerSetterPreprocessor: public PreProcessor{
private:
    BaseShape* targetShape;
public:
    LayerSetterPreprocessor(BaseShape* sourceShape):
        targetShape(sourceShape)
    {

    }

    bool PreProcessBeforeAwake(Object *clonedObj) override {
        if(!clonedObj->GetType()->IsDerivedFrom<BaseShape>())
            return false;
        BaseShape* clonedShape = (BaseShape*)clonedObj;
        targetShape->GetLayer()->AddShapeInternal(clonedShape);
        return true;
    }
};

BaseShape *HuaHuoEngine::DuplicateShape(BaseShape *object) {
    if (object == NULL || object->GetType() == NULL)
        return NULL;

    if (!object->GetType()->IsDerivedFrom<BaseShape>())
        return NULL;

    // Layer setter, the cloned object should be in the same layer of the target object.

    LayerSetterPreprocessor layerSetterPreprocessor(object);
    BaseShape *clonedObject = (BaseShape *) CloneObject(*object, &layerSetterPreprocessor);

    clonedObject->SetIndex(-1); // Unset the index, as the index need to be updated by JS side.

    return clonedObject;
}

void HuaHuoEngine::DestroyShape(BaseShape *shape){
    // Remove the object from it's belonging layer

    shape->GetLayer()->RemoveShape(shape);

    DestroySingleObject(shape);
}