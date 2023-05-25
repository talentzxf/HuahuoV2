//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"

#include <cmath>
#include <cstdio>
#include <cAssert>
#include "Export/Scripting/GameObjectExport.h"
#include "Editor/SceneInspector.h"
#include "Misc/GameObjectUtility.h"
#include "Editor/SceneView.h"
#include "ObjectStore.h"
#include "Shapes/LineShape.h"
#include "Utilities/File.h"
#include "Utilities/PathNameUtility.h"
#include "Utilities/MemoryFileSystem.h"
#include "Shapes/CircleShape.h"
#include "KeyFrames/FrameState.h"
#include "KeyFrames/ShapeTransformComponent.h"
#include "KeyFrames/ShapeFollowCurveFrameState.h"
#include "Shapes/RectangleShape.h"
#include "CloneObject.h"
#include "KeyFrames/CustomFrameState.h"
#include "KeyFrames/CustomComponent.h"
#include "Shapes/ImageShape.h"

#include "openssl/md5.h"

extern float eps;
#define Assert assert

void testTransform() {
    GameObject *go = MonoCreateGameObject("Go1");
    GameObject *go2 = MonoCreateGameObject("Go2");

    Transform *transform1 = go->QueryComponent<Transform>();
    Transform *transform2 = go2->QueryComponent<Transform>();

    transform1->SetParent(transform2);
    printf("Child cound:%d\n", transform2->GetChildrenCount());
    Assert(&transform2->GetChild(0) == transform1);
}

class RootStoreAddedEventHandler: public ScriptEventHandler {
    void handleEvent(ScriptEventHandlerArgs* args){
        ObjectStoreAddedEvent* storeAddedEvent = (ObjectStoreAddedEvent*)args;
        Assert( storeAddedEvent->GetStore() != NULL);
    }
};

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
    ObjectStoreManager *objectStoreManager = GetDefaultObjectStoreManager();
    objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    Layer *currentLayer = objectStoreManager->GetCurrentStore()->GetCurrentLayer();
//    LineShape* lineShape = Object::Produce<LineShape>();
//    lineShape->SetStartPoint(0, 1, 0);
//    lineShape->SetEndPoint(1, 0, 0);
//    currentLayer->AddShapeInternal(lineShape);
//    lineShape->SetPosition(100.0, 100.0, 100.0);
//
//    int segmentSize = 4;
//    float buffer[4*6];
//    for(int i = 0; i < segmentSize; i++){
//        buffer[i*6] = 1.0f;
//        buffer[i*6 + 1 ] = 2.0f;
//        buffer[i*6 + 2 ] = 2.0f;
//        buffer[i*6 + 3 ] = 2.0f;
//        buffer[i*6 + 4 ] = 2.0f;
//        buffer[i*6 + 5 ] = 2.0f;
//    }
//    lineShape->SetSegments(buffer, segmentSize);
//
//    Assert(*lineShape->GetPosition() == Vector3f(100.0, 100.0, 100.0));
//
//    lineShape->GetLayer()->SetCurrentFrame(10);
//    lineShape->SetPosition(200.0, 200.0, 200.0);
//    lineShape->GetLayer()->SetCurrentFrame(5);
//    lineShape->SetScale(0.5,0.5,0.5);
//
//
//    CircleShape* circleShape = Object::Produce<CircleShape>();
//    circleShape->SetCenter(0,0,0);
//    circleShape->SetRadius(10.0);
//    currentLayer->AddShapeInternal(circleShape);
//
//    LineShape* lineShape1 = Object::Produce<LineShape>();
//    lineShape1->SetStartPoint(2,2,2);
//    lineShape1->SetEndPoint(3,3,3);
//    currentLayer->AddShapeInternal(lineShape1);
//
//    for(int i = 0; i < 100; i++){
//        CircleShape* shape = Object::Produce<CircleShape>();
//        shape->SetColor(1.0, 0.0, 1.0, 1.0);
//        currentLayer->AddShapeInternal(shape);
//    }

    RectangleShape *rectangleShape = (RectangleShape *) BaseShape::CreateShape("RectangleShape");
    rectangleShape->SetStartPoint(2, 2, 2);
    rectangleShape->SetEndPoint(3, 3, 3);

    HuaHuoEngine::GetInstance()->DuplicateShape(rectangleShape);

    currentLayer->AddShapeInternal(rectangleShape);

    RectangleShape *clonedRectangleShape = (RectangleShape *) CloneObject(*rectangleShape);

    CircleShape *circleShape = (CircleShape *) BaseShape::CreateShape("CircleShape");
    circleShape->SetCenter(1.0, 1.0, 1.0);
    circleShape->SetRadius(1.0);

    circleShape->SetGlobalPivotPosition(100, 100, 100);
    Vector3f *vector3f = circleShape->GetGlobalPivotPosition();

    int currentFrame = circleShape->GetLayer()->GetCurrentFrame();
    circleShape->GetLayer()->SetCurrentFrame(currentFrame);
    vector3f = circleShape->GetGlobalPivotPosition();

    circleShape->RefreshKeyFrameCache();
    int keyFrameCount = circleShape->GetKeyFrameCount();
    int firstKeyFrame = circleShape->GetKeyFrameAtIdx(0);

    ShapeFollowCurveFrameState *curveFrameState = (ShapeFollowCurveFrameState *) circleShape->GetFrameStateByTypeName(
            "ShapeFollowCurveFrameState");
    curveFrameState->RecordTargetShape(10, rectangleShape);
    curveFrameState->RecordLengthRatio(10, 1.0f);

    curveFrameState->RecordTargetShape(20, clonedRectangleShape);
    curveFrameState->RecordLengthRatio(20, 0.0f);

    currentLayer->SetCurrentFrame(0);

    currentLayer->AddShapeInternal(circleShape);

    circleShape->AddAnimationOffset(100);

    printf("maxFrameId: %d\n", circleShape->GetMaxFrameId());

    ImageShape *imageShape = Object::Produce<ImageShape>();
    vector<UInt8> data = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    if (GetDefaultResourceManager()->LoadBinaryResource("TestTest", "binary", data.data(), data.size())) {
        MD5_CTX md5Ctx;
        MD5_Init(&md5Ctx);
        MD5_Update(&md5Ctx, data.data(), data.size());
        Hash128 resultHash;
        MD5_Final(resultHash.hashData.bytes, &md5Ctx);

        std::string resultHashString = Hash128ToString(resultHash);
        imageShape->SetResourceByMD5(resultHashString.c_str());
    }

    GetPersistentManagerPtr()->WriteFile(StoreFilePath);

    size_t length = GetMemoryFileSystem()->GetFileLength(StoreFilePath);
    printf("File length:%d\n", length);

    std::string fName = StoreFilePath.substr(StoreFilePath.find_last_of("/\\") + 1);
    const char *filename = fName.c_str();
    FILE *fp = fopen(filename, "w+b");
    fwrite(GetMemoryFileSystem()->GetDataPtr(StoreFilePath), length, 1, fp);
    fclose(fp);

    GetScriptEventManager()->RegisterEventHandler("OnRootStoreAdded", new RootStoreAddedEventHandler());

    // std::string filenamestr("C:\\Users\\vincentzhang\\MyProjects\\HuahuoV2\\HuahuoBackend\\projectfiles\\vincentzhang\\ELEMENT\\NewElement_laski\\NewElement_laski.ele");
    std::string filenamestr = std::string("mem://") + filename;
    GetPersistentManagerPtr()->LoadFileCompletely(filenamestr);

    vector<UInt8> imgData = {31, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    GetDefaultResourceManager()->LoadBinaryResource("mellon", "test.jps", imgData.data(), imgData.size());

    GetPersistentManagerPtr()->WriteFile(StoreFilePath);

    GetScriptEventManager()->IsEventRegistered("Hello");

    GetDefaultObjectStoreManager()->GetCurrentStore();
//
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

void testTimeManager() {
    ObjectStore *objectStore = GetDefaultObjectStoreManager()->GetCurrentStore();
    objectStore->CreateLayer("TestTest");
    Layer *layer = objectStore->GetCurrentLayer();
    TimeLineCellManager *timeLineCellManager = layer->GetTimeLineCellManager();
    timeLineCellManager->MergeCells(0, 10);
    Assert(timeLineCellManager->IsSpanHead(0) == true);
    Assert(timeLineCellManager->IsSpanHead(10) == false);
    Assert(timeLineCellManager->IsSpanHead(11) == true);
    Assert(timeLineCellManager->IsSpanHead(20) == true);
    timeLineCellManager->MergeCells(20, 50);
    Assert(timeLineCellManager->IsSpanHead(20) == true);
    Assert(timeLineCellManager->IsSpanHead(30) == false);
    Assert(timeLineCellManager->IsSpanHead(40) == false);
    Assert(timeLineCellManager->IsSpanHead(50) == false);
    Assert(timeLineCellManager->IsSpanHead(51) == true);
    timeLineCellManager->MergeCells(70, 60);
    Assert(timeLineCellManager->IsSpanHead(61) == false);
    timeLineCellManager->MergeCells(0, 100);
    Assert(timeLineCellManager->GetCellSpan(0) == 101);
    Assert(timeLineCellManager->GetSpanHead(1) == 0);

    timeLineCellManager->MergeCells(0, 1000);
    timeLineCellManager->MergeCells(0, 20);

    GetPersistentManagerPtr()->WriteFile(StoreFilePath);
    int length = GetFileLength(StoreFilePath);
    const char *filename = "objectstore_persistent2.data";
    FILE *fp = fopen(filename, "w+b");
    fwrite(GetMemoryFileSystem()->GetDataPtr(StoreFilePath), length, 1, fp);
    fclose(fp);
    std::string filenamestr(filename);
    GetPersistentManagerPtr()->LoadFileCompletely(filenamestr);
    TimeLineCellManager *pTimelineCellManager = GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer()->GetTimeLineCellManager();
    Assert(pTimelineCellManager->GetSpanHead(100) == 0);
}

void testKeyFrames() {
    std::vector<CustomDataKeyFrame> transformKeyFrames;
    std::pair<CustomDataKeyFrame *, CustomDataKeyFrame *> framePair;
    bool result = FindKeyFramePair(0, transformKeyFrames, framePair);
    Assert(result == false);

    CustomDataKeyFrame k1;
    k1.SetFrameId(0);
    k1.data.vector3Value = Vector3f(0.0, 1.0, 0.0);
    transformKeyFrames.push_back(k1);

    result = FindKeyFramePair(0, transformKeyFrames, framePair);
    Assert(result == true);

    transformKeyFrames.clear();
    k1.SetFrameId(1);
    k1.data.vector3Value = Vector3f(0.0, 1.0, 0.0);
    transformKeyFrames.push_back(k1);
    result = FindKeyFramePair(0, transformKeyFrames, framePair);
    Assert(result == true);
    Assert(framePair.second == NULL);
    Assert(framePair.first->GetFrameId() == 1);

    CustomDataKeyFrame k2;
    k2.SetFrameId(5);
    k2.data.vector3Value = Vector3f(0.0, 5.0, 0.0);
    transformKeyFrames.push_back(k2);
    result = FindKeyFramePair(3, transformKeyFrames, framePair);
    Assert(result == true);

    CustomDataKeyFrame k3;
    k3.SetFrameId(10);
    k3.data.vector3Value = Vector3f(0.0, 10.0, 0.0);
    transformKeyFrames.push_back(k3);
    result = FindKeyFramePair(6, transformKeyFrames, framePair);
    Assert(result == true);
    Assert(framePair.first->GetFrameId() == 5 && framePair.second->GetFrameId() == 10);

    result = FindKeyFramePair(11, transformKeyFrames, framePair);
    Assert(result == true);
    Assert(framePair.second == NULL);
    Assert(framePair.first->GetFrameId() == 10);
}

void testRecordKeyFrames() {
    RectangleShape* pRectangle = (RectangleShape*) BaseShape::CreateShape("RectangleShape", true);
    ShapeTransformComponent* pTransformComponent = pRectangle->GetTransform();
    AbstractFrameState* pPosition = pTransformComponent->GetFrameStates()[0].GetComponentPtr();

    std::vector<CustomDataKeyFrame> transformKeyFrames;
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(0, transformKeyFrames, pPosition);
    pKeyFrame->data.vector3Value = Vector3f(1.0, 0.0, 0.0f);
    Assert(transformKeyFrames.size() == 1);
    Assert(transformKeyFrames[0].data.vector3Value == Vector3f(1.0, 0.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(5, transformKeyFrames, pPosition);
    pKeyFrame->data.vector3Value = Vector3f(5.0, 1.0, 0.0f);
    Assert(transformKeyFrames.size() == 2);
    Assert(transformKeyFrames[1].data.vector3Value == Vector3f(5.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(3, transformKeyFrames, pPosition);
    pKeyFrame->data.vector3Value = Vector3f(3.0, 1.0, 0.0f);
    Assert(transformKeyFrames.size() == 3);
    Assert(transformKeyFrames[1].data.vector3Value == Vector3f(3.0, 1.0, 0.0));
    Assert(transformKeyFrames[2].data.vector3Value == Vector3f(5.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(10, transformKeyFrames, pPosition);
    pKeyFrame->data.vector3Value = Vector3f(10.0, 1.0, 0.0f);
    Assert(transformKeyFrames.size() == 4);
    Assert(transformKeyFrames[3].data.vector3Value == Vector3f(10.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(5, transformKeyFrames, pPosition);
    pKeyFrame->data.vector3Value = Vector3f(5.0, 2.0, 0.0f);
    Assert(transformKeyFrames.size() == 4);
    Assert(transformKeyFrames[2].GetFrameId() == 5);
    Assert(transformKeyFrames[2].data.vector3Value == Vector3f(5.0, 2.0, 0.0));
}

void testSegmentKeyFrames() {
    ShapeSegmentFrameState *segmentFrameState = Object::Produce<ShapeSegmentFrameState>();

    int segmentSize = 4;
    std::vector<float> segmentBuffer;
    for (int i = 0; i < segmentSize; i++) {
        segmentBuffer.push_back(i * 6);
        segmentBuffer.push_back(i * 6 + 1);
        segmentBuffer.push_back(i * 6 + 2);
        segmentBuffer.push_back(i * 6 + 3);
        segmentBuffer.push_back(i * 6 + 4);
        segmentBuffer.push_back(i * 6 + 5);
    }

    segmentFrameState->RecordSegments(0, segmentBuffer.data(), segmentSize);

    segmentBuffer.clear();
    for (int i = 0; i < segmentSize; i++) {
        segmentBuffer.push_back(i * 6 + 10);
        segmentBuffer.push_back(i * 6 + 1 + 10);
        segmentBuffer.push_back(i * 6 + 2 + 10);
        segmentBuffer.push_back(i * 6 + 3 + 10);
        segmentBuffer.push_back(i * 6 + 4 + 10);
        segmentBuffer.push_back(i * 6 + 5 + 10);
    }

    segmentFrameState->RecordSegments(9, segmentBuffer.data(), segmentSize);

    segmentFrameState->Apply(5);
}

void testMultipleStores() {
    const char *defaultStoreId = GetDefaultObjectStoreManager()->GetCurrentStore()->GetStoreId();
    ObjectStore *pNewStore = GetDefaultObjectStoreManager()->CreateStore();

    const char *newStore = pNewStore->GetStoreId();

    bool result = GetDefaultObjectStoreManager()->SetDefaultStoreByIndex(newStore);
    Assert(result);

    ObjectStoreManager *objectStoreManager = GetDefaultObjectStoreManager();
    Layer *currentLayer = objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    RectangleShape *rectangleShape = (RectangleShape *) BaseShape::CreateShape("RectangleShape");
    ShapeTransformComponent *pTransformComponent = (ShapeTransformComponent *) rectangleShape->GetFrameStateByTypeName(
            "ShapeTransformComponent");

    pTransformComponent->SetVector3Value("globalPivotPosition", 100.0, 100.0, 0.0);

    currentLayer->AddShapeInternal(rectangleShape);

    // Test save a complicated structure into the store.
    ObjectStore* pNewStore1 = GetDefaultObjectStoreManager()->CreateStore();
    ElementShape* elementShape1 = (ElementShape*) BaseShape::CreateShape("ElementShape");
    elementShape1->SetElementStoreId(pNewStore1->GetStoreId());
    Layer* layer1 = pNewStore1->CreateLayer("Layer1");

    ObjectStore* pNewStore2_1 = GetDefaultObjectStoreManager()->CreateStore();
    ElementShape* elementShape2_1 = (ElementShape*) BaseShape::CreateShape("ElementShape");
    elementShape2_1->SetElementStoreId(pNewStore2_1->GetStoreId());
    layer1->AddShapeInternal(elementShape2_1);
    Layer* layer2_1 = pNewStore2_1->CreateLayer("Layer2_1");

    RectangleShape *rectangleShape2_1_1 = (RectangleShape *) BaseShape::CreateShape("RectangleShape");
    layer2_1->AddShapeInternal(rectangleShape2_1_1);

    ObjectStore* pNewStore2_2 = GetDefaultObjectStoreManager()->CreateStore();
    ElementShape* elementShape2_2 = (ElementShape*) BaseShape::CreateShape("ElementShape");
    elementShape2_2->SetElementStoreId(pNewStore2_2->GetStoreId());
    layer1->AddShapeInternal(elementShape2_2);
    Layer* layer2_2 = pNewStore2_2->CreateLayer("Layer2-2");
    RectangleShape *rectangleShape2_2_1 = (RectangleShape *) BaseShape::CreateShape("RectangleShape");
    layer2_2->AddShapeInternal(rectangleShape2_2_1);

    std::string storeFilePath = StoreFilePath;
    GetPersistentManager().WriteObject(storeFilePath, pNewStore1);
}

void testCloneObject() {
    ObjectStoreManager *objectStoreManager = GetDefaultObjectStoreManager();
    objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    Layer *currentLayer = objectStoreManager->GetCurrentStore()->GetCurrentLayer();

    RectangleShape *rectangleShape = (RectangleShape *) BaseShape::CreateShape("RectangleShape");

    ShapeTransformComponent *pTransformComponent = (ShapeTransformComponent *) rectangleShape->GetFrameStateByTypeName(
            "ShapeTransformComponent");
    KeyFrameCurve *xCurve = pTransformComponent->GetVectorKeyFrameCurve("globalPivotPosition", 0);
    KeyFrameCurve *yCurve = pTransformComponent->GetVectorKeyFrameCurve("globalPivotPosition", 1);

    currentLayer->SetCurrentFrame(10);
    xCurve->SetValue(10, 100.0f);

    currentLayer->SetCurrentFrame(20);
    xCurve->SetValueByIndex(0, 20, 1000.0f);

    rectangleShape->SetStartPoint(2, 2, 2);
    rectangleShape->SetEndPoint(3, 3, 3);

    rectangleShape->SetGlobalPivotPosition(10, 10, 10);
    Vector3f *position = rectangleShape->GetGlobalPivotPosition();
    Assert(*position == Vector3f(10, 10, 10));

    Vector3f *scale = rectangleShape->GetScale();

    currentLayer->SetCurrentFrame(10);
    rectangleShape->SetGlobalPivotPosition(100, 100, 100);
    currentLayer->AddShapeInternal(rectangleShape);

    RectangleShape *clonedRectangle = (RectangleShape *) CloneObject(*rectangleShape);
    currentLayer->AddShapeInternal(clonedRectangle);

    currentLayer->SetCurrentFrame(0);
    Vector3f *curPosition = clonedRectangle->GetGlobalPivotPosition();
    Assert(curPosition != NULL);

    ShapeTransformComponent *shapeTransform = (ShapeTransformComponent *) clonedRectangle->GetFrameStateByTypeName(
            "ShapeTransformComponent");

    int keyFrameCount = shapeTransform->GetKeyFrameCount();

    for (int i = 0; i < keyFrameCount; i++) {
        int frameId = shapeTransform->GetKeyFrameAtIndex(i);
        Assert(frameId >= 0);
    }

    ImageShape *imageShape = (ImageShape *) BaseShape::CreateShape("ImageShape");
    vector<UInt8> imgData = {32, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    std::string imgName = "asdf";
    GetDefaultResourceManager()->LoadBinaryResource(imgName.c_str(), "jpeg", imgData.data(), imgData.size());
    MD5_CTX md5Ctx;
    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, imgData.data(), imgData.size());
    Hash128 resultHash;
    MD5_Final(resultHash.hashData.bytes, &md5Ctx);

    std::string md5Str = Hash128ToString(resultHash);
    imageShape->SetResourceByMD5(md5Str.c_str());

    ImageShape *imageShapeDup = (ImageShape *) CloneObject(*imageShape);
    Assert(imageShapeDup->GetFileName() == imgName);
    Assert(imageShapeDup->GetDataSize() == imgData.size());
}

void testDelete() {
    ObjectStoreManager *objectStoreManager = GetDefaultObjectStoreManager();
    objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    Layer *currentLayer = objectStoreManager->GetCurrentStore()->GetCurrentLayer();
    currentLayer->SetCurrentFrame(5);

    RectangleShape *rectangleShape = (RectangleShape *) BaseShape::CreateShape("RectangleShape");

    float segments[] = {
            0.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            2.0f, 2.0f,
            2.0f, 2.0f,
            2.0f, 2.0f,
            3.0f, 3.0f,
            3.0f, 3.0f,
            3.0f, 3.0f
    };

    rectangleShape->SetSegments(segments, 4);
    rectangleShape->SetGlobalPivotPosition(100.0f, 200.0f, 300.0f);
    rectangleShape->RemoveSegment(2);

    currentLayer->AddShapeInternal(rectangleShape);

    currentLayer->RemoveShape(rectangleShape);

    for (int i = 0; i < 100; i++) {
        RectangleShape *rectangleShape2 = (RectangleShape *) BaseShape::CreateShape("RectangleShape");

        float segments2[] = {
                0.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 0.0f,
                1.0f, 1.0f,
                1.0f, 1.0f,
                1.0f, 1.0f,
                2.0f, 2.0f,
                2.0f, 2.0f,
                2.0f, 2.0f,
                3.0f, 3.0f,
                3.0f, 3.0f,
                3.0f, 3.0f
        };

        rectangleShape2->SetSegments(segments, 4);
        rectangleShape2->SetGlobalPivotPosition(300.0f, 400.0f, 500.0f);
        rectangleShape2->RemoveSegment(2);

        currentLayer->AddShapeInternal(rectangleShape2);

        rectangleShape2->SetGlobalPivotPosition(500.0f, 600.0f, 700.0f);

        currentLayer->RemoveShape(rectangleShape2);
    }
}

// TODO: What a huge mess!
void testReadFromFile() {
//    std::string fileName("0Gp3iuAmyG1663567838");
//    std::string filePath = "C:\\Users\\vincentzhang\\Downloads\\" + fileName + ".hua";
//
//    //1. Copy the file into memory
//    std::string memFileName = "mem://" + fileName;
//    size_t fileLength = GetFileLength(filePath);
//
//    unsigned char * pBuffer = new unsigned char[fileLength];
//    FILE* fp = fopen(filePath.c_str(), "rb");
//    fread(pBuffer, 1, fileLength, fp);
//    fclose(fp);
//
//    MemoryFileAccessor* pAccessor = GetMemoryFileSystem()->OpenFile(memFileName, kWritePermission);
//    pAccessor->Write(pBuffer, fileLength);
//    GetMemoryFileSystem()->CloseFile(pAccessor);
//
//    // Set the memory file as the default file
//    StoreFilePath = memFileName;
    // GetPersistentManager().LoadFileCompletely(memFileName);

    GetDefaultObjectStoreManager()->GetCurrentStore()->CreateLayer("asdfasdfasdfasdf");
    CircleShape *circleShape = (CircleShape *) BaseShape::CreateShape("CircleShape");

    circleShape->SaveAsKeyFrame();
    ShapeTransformComponent *transformComponent = (ShapeTransformComponent *) circleShape->GetFrameStateByTypeName(
            "ShapeTransformComponent");
    transformComponent->GetFieldCount();

    circleShape->SetRadius(10.0f);
    circleShape->SetCenter(0.0, 1.0, 2.0);
    circleShape->SetBornFrameId(10);

    Layer *pLayer = circleShape->GetLayer();
    pLayer->SetCurrentFrame(0);
    bool shapeIsVisible = circleShape->IsVisible();

    CustomComponent *customComponent = CustomComponent::CreateComponent("CustomComponent");
    customComponent->SetName("CurveGrowth");

    customComponent->RegisterFloatValue("growth", 1.0f);
    customComponent->RegisterColorValue("strokeColor", 1.0, 0.0, 0.0, 1.0);
    customComponent->RegisterColorStopArrayValue("gradientColor");
    customComponent->RegisterBinaryResource("particleShape");
    Assert(customComponent->GetFloatValue("growth") == 1.0f);

    customComponent->RegisterBooleanValue("active", true);
    customComponent->SetBooleanValue("active", true);

    circleShape->AddFrameState(customComponent);
    GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer()->AddShapeInternal(circleShape);

    customComponent->AddColorStop("gradientColor", 0.0, 1.0, 0.0, 0.0, 1.0);
    customComponent->AddColorStop("gradientColor", 1.0, 0.0, 0.0, 1.0, 1.0);
    int identifier = customComponent->AddColorStop("gradientColor", 0.75, 0.3, 0.4, 0.5, 1.0);

    Assert(identifier == 2);
    customComponent->DeleteColorStop("gradientColor", identifier);
    ColorStopArray* colorStopArray = customComponent->GetColorStopArray("gradientColor");

    Assert(colorStopArray->GetColorStopCount() == 2);

    customComponent->DeleteColorStop("gradientColor", 1);

    Assert(colorStopArray->GetColorStopCount() == 1);
    customComponent->AddColorStop("gradientColor", 1.0, 0.0, 0.0, 1.0, 1.0);

    Assert(colorStopArray->GetColorStopCount() == 2);
    customComponent->UpdateColorStop("gradientColor", 0, 0.1, 0.0, 1.0, 1.0, 1.0);

    Assert(colorStopArray->GetColorStop(0)->GetColor()->r == 0.0 );
    Assert(colorStopArray->GetColorStop(0)->GetColor()->g == 1.0 );
    Assert(colorStopArray->GetColorStop(0)->GetColor()->b == 1.0 );
    Assert(colorStopArray->GetColorStop(0)->GetColor()->a == 1.0 );
    Assert( abs(colorStopArray->GetColorStop(0)->GetValue() - 0.1) <= eps );

    customComponent->SetFloatValue("growth", 1.0f);
    customComponent->SetColorValue("strokeColor", 1.0, 1.0, 0.0, 1.0);

    Assert(customComponent->GetFloatValue("growth") == 1.0f);
    ColorRGBAf *color = customComponent->GetColorValue("strokeColor");

    Assert(color->r == 1.0f);
    Assert(color->g == 1.0f);
    Assert(color->b == 0.0f);
    Assert(color->a == 1.0f);

    customComponent->Apply(0);
    int frameId = circleShape->GetMinFrameId();

    Assert(frameId == 0);

    Layer *shapeLayer = circleShape->GetLayer();
    shapeLayer->SetCurrentFrame(10);

    customComponent->AddColorStop("gradientColor", 0.5);
    customComponent->UpdateColorStop("gradientColor", 0, 1.0, 1.0, 0.0, 0.0, 1.0);

    ColorStopArray *pResult = customComponent->GetColorStopArray("gradientColor");

    Assert(pResult->GetColorStopCount() == 3);

    customComponent->SetColorValue("strokeColor", 0.0, 1.0, 0.0, 1.0);
    color = customComponent->GetColorValue("strokeColor");
    Assert(color->r == 0.0f);
    Assert(color->g == 1.0f);
    Assert(color->b == 0.0f);
    Assert(color->a == 1.0f);


    std::vector<UInt8> test = {12, 32, 123, 123, 1, 23, 1, 23, 1, 22, 3, 1};
    GetDefaultResourceManager()->LoadBinaryResource("HelloHello2", "asfdasdf", test.data(), test.size());
    MD5_CTX md5Ctx;
    MD5_Init(&md5Ctx);
    MD5_Update(&md5Ctx, test.data(), test.size());
    Hash128 md5Result;
    MD5_Final(md5Result.hashData.bytes, &md5Ctx);
    std::string md5ResultString = Hash128ToString(md5Result);

    customComponent->SetBinaryResourceByMD5("particleShape", md5ResultString.c_str());
    BinaryResourceWrapper *binaryResource = customComponent->GetBinaryResource("particleShape");
    Assert(strcmp(binaryResource->GetResourceName(), "HelloHello2") == 0);

    // GetPersistentManager().WriteFile(StoreFilePath);

    customComponent->SetFloatValue("growth", 0.5f);
    Assert(customComponent->GetFloatValue("growth") == 0.5f);

    customComponent->RegisterShapeArrayValue("targetShapeArray");
    customComponent->CreateShapeArrayValue("targetShapeArray");


    CircleShape *clonedCircleShape = (CircleShape *) CloneObject(*circleShape);
    clonedCircleShape->Apply(shapeLayer->GetCurrentFrame());

    CustomComponent *clonedGrowthComponent = (CustomComponent *) clonedCircleShape->GetFrameState("CurveGrowth");
    Assert(clonedGrowthComponent->GetFloatValue("growth") == 0.5f);
    clonedGrowthComponent->SetFloatValue("growth", 100.0f);

    Assert(customComponent->GetFloatValue("growth") != clonedGrowthComponent->GetFloatValue("growth"));
    Assert(customComponent->GetFloatValue("growth") == 0.5f);
    Assert(clonedGrowthComponent->GetFloatValue("growth") == 100.0f);

    customComponent->ReverseKeyFrame(0, 10, 5);

    int keyFrameCount = customComponent->GetKeyFrameCount();
    for (int idx = 0; idx < keyFrameCount; idx++) {
        int keyframeId = customComponent->GetKeyFrameAtIndex(idx);

        Assert(keyframeId >= 0);
    }
    customComponent->DeleteKeyFrame(0);

    customComponent->GetBaseShape()->GetLayer()->SetCurrentFrame(20);
    customComponent->SetFloatValue("growth", 0.5);

    customComponent->DeleteKeyFrame(20);

    customComponent->GetBaseShape()->GetLayer()->IsKeyFrame(20);

    customComponent->RegisterSubcomponentArray("particleSystems");

    CustomComponent *particleSystem = customComponent->GetSubComponentArrayByName("particleSystems");

    particleSystem->RegisterVector3Value("maxInitVelocity", 100.0, 100.0, 100.0);
    Vector3f *velocity = particleSystem->GetVector3Value("maxInitVelocity");

    customComponent->Apply(1);
    velocity = particleSystem->GetVector3Value("maxInitVelocity");

    Assert(particleSystem != NULL);
    Assert(velocity != NULL);

    customComponent->RegisterStringValue("eventGraphJson", "");
    customComponent->SetStringValue("eventGraphJson", "Test");
    const char *string = customComponent->GetStringValue("eventGraphJson");
    Assert(string != NULL);

    KeyFrameCurve *pCurve = customComponent->GetFloatKeyFrameCurve("growth");
    Assert(pCurve != NULL && pCurve->GetTotalPoints() != 0);
}

void testKeyFrameCurve() {
    KeyFrameCurve keyFrameCurve;
    keyFrameCurve.SetCurveValue(1.0, 1);
    keyFrameCurve.SetCurveValue(2.0, 10);
    keyFrameCurve.SetCurveValue(3.0, 5);
    keyFrameCurve.SetCurveValue(4.0, 10);
    keyFrameCurve.SetCurveValue(120.0, 120.0);

    Assert(keyFrameCurve.GetKeyFrameCurvePoint(0)->GetFrameId() == 1);
    Assert(keyFrameCurve.GetKeyFrameCurvePoint(1)->GetFrameId() == 2);
    Assert(keyFrameCurve.GetKeyFrameCurvePoint(2)->GetFrameId() == 3);

    const char *fieldName = "thickness";

    Layer *layer = GetDefaultObjectStoreManager()->GetCurrentStore()->CreateLayer("Hahahaha");
    CircleShape *circleShape = (CircleShape *) BaseShape::CreateShape("CircleShape");

    CustomComponent *customComponent = CustomComponent::CreateComponent("CustomComponent");
    circleShape->AddFrameState(customComponent);
    customComponent->RegisterFloatValue(fieldName, 1.0f);
    layer->SetCurrentFrame(0);
    customComponent->SetFloatValue(fieldName, 0.0f);
    layer->SetCurrentFrame(100);
    customComponent->SetFloatValue(fieldName, 100.0f);
    layer->SetCurrentFrame(200);
    customComponent->SetFloatValue(fieldName, 200.0f);
    layer->SetCurrentFrame(300);
    customComponent->SetFloatValue(fieldName, 300.0f);

    KeyFrameCurve *keyFrameCurve1 = customComponent->GetFloatKeyFrameCurve(fieldName);
    Assert(keyFrameCurve1->GetTotalPoints() == 4);

    layer->SetCurrentFrame(400);
    keyFrameCurve1->SetValueByIndex(3, 400, 400);

    bool isKeyFrame = layer->IsKeyFrame(300);
    Assert(isKeyFrame == true);
    isKeyFrame = layer->IsKeyFrame(400);
    Assert(isKeyFrame == false);
    customComponent->Apply(400);
    float value = customComponent->GetFloatValue(fieldName);
    Assert(value == 400);
}

void testMoveShapeFromStoreToStore(){
    auto store1 = GetDefaultObjectStoreManager()->CreateStore();
    auto layer1_1 = store1->CreateLayer("First Layer");
    RectangleShape* pRectangleShape = (RectangleShape*)BaseShape::CreateShape("RectangleShape");
    layer1_1->AddShapeInternal(pRectangleShape);
    pRectangleShape->SetGlobalPivotPosition(100, 100, 100);

    layer1_1->SetCurrentFrame(10);
    pRectangleShape->SetGlobalPivotPosition(200, 200, 200);

    layer1_1->SetCurrentFrame(5);
    Vector3f* currentPosition = pRectangleShape->GetGlobalPivotPosition();
    Assert(currentPosition->x == 150);
    Assert(currentPosition->y == 150);
    Assert(currentPosition->z == 150);

    auto store2 = GetDefaultObjectStoreManager()->CreateStore();
    auto layer2_1 = store2->CreateLayer("Second Layer");
    layer2_1->SetCurrentFrame(0);
    layer2_1->AddShapeInternal(pRectangleShape);

    layer2_1->SetCurrentFrame(1);
    currentPosition = pRectangleShape->GetGlobalPivotPosition();
    Assert(currentPosition->x == 110);
    Assert(currentPosition->y == 110);
    Assert(currentPosition->z == 110);
}

int main() {
    HuaHuoEngine::InitEngine();

    testReadFromFile();
//     testTransform();
//     testScene();
//     testGameObject();
    testTimeManager();
    testShapeStore();
    testKeyFrames();
    testRecordKeyFrames();
    testSegmentKeyFrames();
    testCloneObject();
    testDelete();

    testKeyFrameCurve();

    testMultipleStores();

    testMoveShapeFromStoreToStore();
    return 0;
}
