//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"

#include <cstdio>
#include "Export/Scripting/GameObjectExport.h"
#include "Editor/SceneInspector.h"
#include "Misc/GameObjectUtility.h"
#include "Editor/SceneView.h"
#include "ObjectStore.h"
#include "Shapes/LineShape.h"
#include "Utilities/File.h"
#include "Utilities/PathNameUtility.h"
#include "Utilities/MemoryFileSystem.h"

void testTransform() {
    GameObject *go = MonoCreateGameObject("Go1");
    GameObject *go2 = MonoCreateGameObject("Go2");

    Transform *transform1 = go->QueryComponent<Transform>();
    Transform *transform2 = go2->QueryComponent<Transform>();

    transform1->SetParent(transform2);
    printf("Child cound:%d\n", transform2->GetChildrenCount());
    Assert(&transform2->GetChild(0) == transform1);
}

class TestScriptEventHandler : public ScriptEventHandler {
    void handleEvent(ScriptEventHandlerArgs *args) {
        printf("HelloHello");
    }
};

//void testScene() {
//    HuaHuoScene *pScene = GetSceneManager().CreateScene();
//    GetSceneManager().SetActiveScene(pScene);
//
//    std::string goName("Go!!!");
//    MonoCreateGameObject(goName.c_str());
//
//    GetScriptEventManager()->RegisterEventHandler("OnHierarchyChangedSetParent", new TestScriptEventHandler());
//
//    for (auto itr = pScene->RootBegin(); itr != pScene->RootEnd(); itr++) {
//        printf("name: %s", itr->GetData()->GetName());
//    }
//    Assert(pScene->RootBegin() != pScene->RootEnd());
//}

void testGameObject() {
    printf("Creating camera Gameobject\n");
    GameObject &cameraGO = CreateGameObject("SceneCamera", "Transform", "Camera");
    printf("Camera Gameobject created!\n");
}

void testShapeStore() {
    ObjectStoreManager* objectStoreManager = GetDefaultObjectStoreManager();
    objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    Layer* currentLayer = objectStoreManager->GetCurrentStore()->GetCurrentLayer();
    LineShape* lineShape = Object::Produce<LineShape>();
    lineShape->SetStartPoint(0, 1, 0);
    lineShape->SetEndPoint(1, 0, 0);
    currentLayer->AddShapeInternal(lineShape);

    LineShape* lineShape1 = Object::Produce<LineShape>();
    lineShape1->SetStartPoint(2,2,2);
    lineShape1->SetEndPoint(3,3,3);
    currentLayer->AddShapeInternal(lineShape1);

    GetPersistentManagerPtr()->WriteFile(StoreFilePath);

    size_t length = GetMemoryFileSystem()->GetFileLength(StoreFilePath);
    printf("File length:%d\n", length);

    const char* filename = "objectstore.data";
    FILE* fp = fopen(filename, "w+b");
    fwrite(GetMemoryFileSystem()->GetDataPtr(StoreFilePath),length,1, fp);
    fclose(fp);

//    std::string path = StoreFilePath;
//    GetPersistentManager().BeginFileWriting(path);
//
//    BlockMemoryCacheWriter memoryCacheWriter(kMemDefault);
//    StreamedBinaryWrite writeStream;
//    CachedWriter* writeCache = &writeStream.Init(kReadWriteFromSerializedFile);
//    writeCache->InitWrite(memoryCacheWriter);
//    GetDefaultObjectStoreManager()->VirtualRedirectTransfer(writeStream);
//    writeCache->CompleteWriting();
//
//    UInt32 length = memoryCacheWriter.GetFileLength() * sizeof(UInt8);
//    Assert(length != 0);
//    printf("Length is:%d\n", length);
//
//    GetPersistentManager().BeginFileReading(path);
//
//    MemoryCacherReadBlocks memoryCacheReader(memoryCacheWriter.GetCacheBlocks(), memoryCacheWriter.GetFileLength(), memoryCacheWriter.GetCacheSize());
//    StreamedBinaryRead readStream;
//    CachedReader* readCache = &readStream.Init(kReadWriteFromSerializedFile);
//    readCache->InitRead(memoryCacheReader, 0 , writeCache->GetPosition());
//    Object* clonedObj = Object::Produce(GetDefaultObjectStoreManager()->GetType());
//    clonedObj->VirtualRedirectTransfer(readStream);
//    ObjectStoreManager* storeManager = (ObjectStoreManager*)clonedObj;
//    Assert(storeManager->GetCurrentStore()->GetCurrentLayer() != NULL);
//
//    Layer* layer = storeManager->GetCurrentStore()->GetCurrentLayer();
//    BaseShape* firstShape = layer->GetShapes()[0];
//    Assert(firstShape != NULL);
//
//    std::vector<UInt8> memoryStream;
//    size_t size = GetPersistentManager().WriteStoreFileInMemory();
//    Assert(size != 0);
}

int main() {
    HuaHuoEngine::InitEngine();
//    testTransform();
//    testScene();
//    testGameObject();
    testShapeStore();
    return 0;
}
