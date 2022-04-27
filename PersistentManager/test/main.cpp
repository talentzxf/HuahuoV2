//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "PersistentManagerConfig.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"

#include <cstdio>

int main() {
    PersistentManager::InitEngine();

//    PersistentManager *pManager = PersistentManager::getInstance();
//    pManager->getBuffer();
//
//    printf("Version: %d.%d\n", PM_VERSION_MAJOR, PM_VERSION_MINOR);
//
    Transform *transform = Transform::Produce();
    if (transform == NULL) {
        printf("ERROR\n");
    }
    printf("%s\n", transform->GetPPtrTypeString());

    printf("%d", TypeOf<Transform>()->IsDerivedFrom<BaseComponent>());

    transform->RebuildTransformHierarchy();
    Quaternionf quaternionf(1.0f, 2.0f, 3.0f, 4.0f);
    transform->SetLocalRotation(quaternionf);

    StreamedBinaryWrite writeStream;
    CachedWriter &writeCache = writeStream.Init(kSerializeForPrefabSystem); //, BuildTargetSelection::NoTarget());
    BlockMemoryCacheWriter bmcw;
    writeCache.InitWrite(bmcw);
    transform->Transfer(writeStream);
    writeCache.CompleteWriting();

    Transform *targetTransform = Transform::Produce();
    targetTransform->RebuildTransformHierarchy();
    MemoryCacherReadBlocks cacheReader(bmcw.GetCacheBlocks(), bmcw.GetFileLength(), bmcw.GetCacheSize());
    StreamedBinaryRead readStream;
    CachedReader& readCache = readStream.Init(kSerializeForPrefabSystem | kDontCreateMonoBehaviourScriptWrapper | kIsCloningObject);
    readCache.InitRead(cacheReader, 0, writeCache.GetPosition());
    targetTransform->Transfer(readStream);
    readCache.End();

    Quaternionf quaternionfTarget = targetTransform->GetLocalRotation();
    printf("%f,%f,%f,%f\n", quaternionfTarget.x, quaternionfTarget.y, quaternionfTarget.z, quaternionfTarget.w);

    return 0;
}
