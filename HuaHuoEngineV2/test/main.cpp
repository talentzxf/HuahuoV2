//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"

#include <cstdio>
#include <cassert>
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

#define ASSERT assert

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
//    assert(*lineShape->GetPosition() == Vector3f(100.0, 100.0, 100.0));
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
    Vector3f *vector3F = circleShape->GetGlobalPivotPosition();

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

    ImageShape* imageShape = Object::Produce<ImageShape>();
    imageShape->SetFileName("TestTestTest");
    vector<UInt8> data = {30,31,32,33,34,35,36,37,38,39,40};
    imageShape->SetData("asdf", data.data(), data.size());

    GetPersistentManagerPtr()->WriteFile(StoreFilePath);

    size_t length = GetMemoryFileSystem()->GetFileLength(StoreFilePath);
    printf("File length:%d\n", length);

    std::string fName = StoreFilePath.substr(StoreFilePath.find_last_of("/\\") + 1);
    const char *filename = fName.c_str();
    FILE *fp = fopen(filename, "w+b");
    fwrite(GetMemoryFileSystem()->GetDataPtr(StoreFilePath), length, 1, fp);
    fclose(fp);

    // std::string filenamestr("C:\\Users\\vincentzhang\\Downloads\\huahuo_project (44)\\0Gp3iuAmyG1678541779");
    std::string filenamestr = std::string("mem://") + filename;
    GetPersistentManagerPtr()->LoadFileCompletely(filenamestr);

    vector<UInt8> imgData = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    GetDefaultResourceManager()->SetFileData("mellon", "test.jps", imgData.data(), imgData.size());

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
    Assert(timeLineCellManager->IsSpanHead(10) == true);
    Assert(timeLineCellManager->IsSpanHead(11) == false);
    Assert(timeLineCellManager->IsSpanHead(20) == false);
    timeLineCellManager->MergeCells(20, 50);
    Assert(timeLineCellManager->IsSpanHead(20) == true);
    Assert(timeLineCellManager->IsSpanHead(30) == true);
    Assert(timeLineCellManager->IsSpanHead(40) == true);
    Assert(timeLineCellManager->IsSpanHead(50) == true);
    Assert(timeLineCellManager->IsSpanHead(51) == false);
    timeLineCellManager->MergeCells(70, 60);
    Assert(timeLineCellManager->IsSpanHead(51) == false);
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
    assert(result == false);

    CustomDataKeyFrame k1;
    k1.SetFrameId(0);
    k1.data.vector3Value = Vector3f(0.0, 1.0, 0.0);
    transformKeyFrames.push_back(k1);

    result = FindKeyFramePair(0, transformKeyFrames, framePair);
    assert(result == true);

    transformKeyFrames.clear();
    k1.SetFrameId(1);
    k1.data.vector3Value = Vector3f(0.0, 1.0, 0.0);
    transformKeyFrames.push_back(k1);
    result = FindKeyFramePair(0, transformKeyFrames, framePair);
    assert(result == true);
    assert(framePair.second == NULL);
    assert(framePair.first->GetFrameId() == 1);

    CustomDataKeyFrame k2;
    k2.SetFrameId(5);
    k2.data.vector3Value = Vector3f(0.0, 5.0, 0.0);
    transformKeyFrames.push_back(k2);
    result = FindKeyFramePair(3, transformKeyFrames, framePair);
    assert(result == true);

    CustomDataKeyFrame k3;
    k3.SetFrameId(10);
    k3.data.vector3Value = Vector3f(0.0, 10.0, 0.0);
    transformKeyFrames.push_back(k3);
    result = FindKeyFramePair(6, transformKeyFrames, framePair);
    assert(result == true);
    assert(framePair.first->GetFrameId() == 5 && framePair.second->GetFrameId() == 10);

    result = FindKeyFramePair(11, transformKeyFrames, framePair);
    assert(result == true);
    assert(framePair.second == NULL);
    assert(framePair.first->GetFrameId() == 10);
}

void testRecordKeyFrames() {
    std::vector<CustomDataKeyFrame> transformKeyFrames;
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(0, transformKeyFrames, NULL);
    pKeyFrame->data.vector3Value = Vector3f(1.0, 0.0, 0.0f);
    assert(transformKeyFrames.size() == 1);
    assert(transformKeyFrames[0].data.vector3Value == Vector3f(1.0, 0.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(5, transformKeyFrames, NULL);
    pKeyFrame->data.vector3Value = Vector3f(5.0, 1.0, 0.0f);
    assert(transformKeyFrames.size() == 2);
    assert(transformKeyFrames[1].data.vector3Value == Vector3f(5.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(3, transformKeyFrames, NULL);
    pKeyFrame->data.vector3Value = Vector3f(3.0, 1.0, 0.0f);
    assert(transformKeyFrames.size() == 3);
    assert(transformKeyFrames[1].data.vector3Value == Vector3f(3.0, 1.0, 0.0));
    assert(transformKeyFrames[2].data.vector3Value == Vector3f(5.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(10, transformKeyFrames, NULL);
    pKeyFrame->data.vector3Value = Vector3f(10.0, 1.0, 0.0f);
    assert(transformKeyFrames.size() == 4);
    assert(transformKeyFrames[3].data.vector3Value == Vector3f(10.0, 1.0, 0.0));

    pKeyFrame = InsertOrUpdateKeyFrame(5, transformKeyFrames, NULL);
    pKeyFrame->data.vector3Value = Vector3f(5.0, 2.0, 0.0f);
    assert(transformKeyFrames.size() == 4);
    assert(transformKeyFrames[2].GetFrameId() == 5);
    assert(transformKeyFrames[2].data.vector3Value == Vector3f(5.0, 2.0, 0.0));
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
    int defaultStoreId = GetDefaultObjectStoreManager()->GetCurrentStore()->GetStoreId();
    ObjectStore *pNewStore = GetDefaultObjectStoreManager()->CreateStore();

    int newStore = pNewStore->GetStoreId();

    bool result = GetDefaultObjectStoreManager()->SetDefaultStoreByIndex(newStore);
    assert(result);
}

void testCloneObject() {
    ObjectStoreManager *objectStoreManager = GetDefaultObjectStoreManager();
    objectStoreManager->GetCurrentStore()->CreateLayer("TestTest");
    Layer *currentLayer = objectStoreManager->GetCurrentStore()->GetCurrentLayer();

    RectangleShape *rectangleShape = (RectangleShape *) BaseShape::CreateShape("RectangleShape");
    rectangleShape->SetStartPoint(2, 2, 2);
    rectangleShape->SetEndPoint(3, 3, 3);

    rectangleShape->SetGlobalPivotPosition(10, 10, 10);

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
    std::string imgName("TestImageTestImage");
    imageShape->SetFileName(imgName);
    vector<UInt8> imgData = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    imageShape->SetData("asdf", imgData.data(), imgData.size());

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
    printf("GetValue:%f\n", customComponent->GetFloatValue("growth"));

    circleShape->AddFrameState(customComponent);
    GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer()->AddShapeInternal(circleShape);

    customComponent->AddColorStop("gradientColor", 0.0, 1.0, 0.0, 0.0, 1.0);
    customComponent->AddColorStop("gradientColor", 1.0, 0.0, 0.0, 1.0, 1.0);
    int identifier = customComponent->AddColorStop("gradientColor", 0.75, 0.3, 0.4, 0.5, 1.0);
    customComponent->DeleteColorStop("gradientColor", identifier);
    customComponent->GetColorStopArray("gradientColor");

    customComponent->DeleteColorStop("gradientColor", 1);
    customComponent->GetColorStopArray("gradientColor");
    customComponent->AddColorStop("gradientColor", 1.0, 0.0, 0.0, 1.0, 1.0);

    customComponent->GetColorStopArray("gradientColor");

    customComponent->UpdateColorStop("gradientColor", 0, 0.1, 0.0, 1.0, 1.0, 1.0);

    customComponent->SetFloatValue("growth", 1.0f);
    customComponent->SetColorValue("strokeColor", 1.0, 1.0, 0.0, 1.0);

    // customComponent->SetBinaryResourceName("particleShape", "HelloHello1");

    ColorRGBAf *color = customComponent->GetColorValue("strokeColor");

    customComponent->Apply(0);
    circleShape->GetMinFrameId();

    Layer *shapeLayer = circleShape->GetLayer();
    shapeLayer->SetCurrentFrame(10);

    customComponent->AddColorStop("gradientColor", 0.5);

    customComponent->UpdateColorStop("gradientColor", 0, 1.0, 1.0, 0.0, 0.0, 1.0);

    ColorStopArray *pResult = customComponent->GetColorStopArray("gradientColor");

    customComponent->SetColorValue("strokeColor", 0.0, 1.0, 0.0, 1.0);
    float growthValue = customComponent->GetFloatValue("growth");

    customComponent->SetBinaryResourceName("particleShape", "HelloHello2");
    BinaryResource* binaryResource = customComponent->GetBinaryResource("particleShape");
    Assert(binaryResource->GetResourceName() == "HelloHello2");

    GetPersistentManager().WriteFile(StoreFilePath);

    customComponent->SetFloatValue("growth", 0.5f);
    customComponent->RegisterShapeArrayValue("targetShapeArray");
    customComponent->CreateShapeArrayValue("targetShapeArray");

    printf("GetValue:%f\n", customComponent->GetFloatValue("growth"));

    color = customComponent->GetColorValue("strokeColor");
    printf("GetColor:%f,%f,%f,%f\n", color->r, color->g, color->b, color->a);


    CircleShape *clonedCircleShape = (CircleShape *) CloneObject(*circleShape);

    CustomComponent *clonedGrowthComponent = (CustomComponent *) clonedCircleShape->GetFrameState("CurveGrowth");
    clonedGrowthComponent->SetFloatValue("growth", 100.0f);

    Assert(customComponent->GetFloatValue("growth") != clonedGrowthComponent->GetFloatValue("growth"));

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
    const char* string = customComponent->GetStringValue("eventGraphJson");
    Assert(string != NULL);

    KeyFrameCurve* pCurve = customComponent->GetFloatKeyFrameCurve("growth");
    Assert(pCurve != NULL && pCurve->GetTotalPoints() != 0);
}

void testKeyFrameCurve(){
    KeyFrameCurve keyFrameCurve;
    keyFrameCurve.AddValue(1.0, 1);
    keyFrameCurve.AddValue(2.0, 10);
    keyFrameCurve.AddValue(3.0, 5);
    keyFrameCurve.AddValue(4.0, 10);
    keyFrameCurve.AddValue(120.0, 120.0);

    Assert(keyFrameCurve.GetKeyFrameCurvePoint(0)->GetFrameId() == 1);
    Assert(keyFrameCurve.GetKeyFrameCurvePoint(1)->GetFrameId() == 5);
    Assert(keyFrameCurve.GetKeyFrameCurvePoint(2)->GetFrameId() == 10);

    const char* fieldName = "thickness";

    Layer* layer = GetDefaultObjectStoreManager()->GetCurrentStore()->CreateLayer("Hahahaha");
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

    KeyFrameCurve* keyFrameCurve1 = customComponent->GetFloatKeyFrameCurve(fieldName);
    Assert(keyFrameCurve1->GetTotalPoints() == 4);

    layer->SetCurrentFrame(400);
    keyFrameCurve1->SetValueByIndex(3, 400, 400);

    bool isKeyFrame = layer->IsKeyFrame(300);
    Assert(isKeyFrame == false);
    isKeyFrame = layer->IsKeyFrame(400);
    Assert(isKeyFrame == true);
    customComponent->Apply(400);
    float value = customComponent->GetFloatValue(fieldName);
    Assert(value == 400);
}

int main() {
    HuaHuoEngine::InitEngine();

    testReadFromFile();
    // testTransform();
    // testScene();
    // testGameObject();
    testTimeManager();
    testShapeStore();
    testKeyFrames();
    testRecordKeyFrames();
    testSegmentKeyFrames();
    testCloneObject();
    testDelete();

    testKeyFrameCurve();

    testMultipleStores();
    return 0;
}
