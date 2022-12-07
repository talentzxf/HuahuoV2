//
// Created by VincentZhang on 6/1/2022.
//

#include "BaseShape.h"
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/SerializeUtility.h"
#include "Layer.h"
#include "ObjectStore.h"

IMPLEMENT_REGISTER_CLASS(BaseShape, 10002);

IMPLEMENT_OBJECT_SERIALIZE(BaseShape);

INSTANTIATE_TEMPLATE_TRANSFER(BaseShape);

template<class TransferFunction>
void BaseShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(mShapeName);
    TRANSFER(mBornFrameId);
    TRANSFER(mIndex);

    TransferFrameStates(transfer);
}

template<class TransferFunction>
void BaseShape::TransferFrameStates(TransferFunction &transfer) {
    // When cloning objects for prefabs and instantiate, we don't use serialization to duplicate the hierarchy,
    // we duplicate the hierarchy directly
    if (!SerializePrefabIgnoreProperties(transfer))
        return;

    if (transfer.IsWriting() && transfer.NeedsInstanceIDRemapping()) {
        Container filtered_framestates;
        for (Container::iterator i = mFrameStates.begin(); i != mFrameStates.end(); i++) {
            LocalSerializedObjectIdentifier localIdentifier;
            InstanceIDToLocalSerializedObjectIdentifier(i->GetComponentPtr()->GetInstanceID(), localIdentifier);
            if (localIdentifier.localIdentifierInFile != 0)
                filtered_framestates.push_back(*i);
        }
        transfer.Transfer(filtered_framestates, "mFrameStates",
                          kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);
        return;
    }
    transfer.Transfer(mFrameStates, "mFrameStates",
                      kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);
}

void BaseShape::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    if (this->mLayer !=
        NULL) { // When the shape is loaded, it's layer has not been loaded yet. It's possible as this might be a shape within another element.
        int frameId = this->GetLayer()->GetCurrentFrame();
        Apply(frameId);
    }

    ShapeLoadedEventArgs args(this);
    GetScriptEventManager()->TriggerEvent("OnShapeLoaded", &args);
}

BaseShape *BaseShape::CreateShape(const char *shapeName) {
    const HuaHuo::Type *shapeType = HuaHuo::Type::FindTypeByName(shapeName);
    if (shapeType == NULL || !shapeType->IsDerivedFrom<BaseShape>()) {
        return NULL;
    }

    BaseShape *baseShape = (BaseShape *) Object::Produce(shapeType);
    return baseShape;
}

// TODO: This logic is not good. Explicit is better than implicit!!!!
Layer *BaseShape::GetLayer(bool assignDefaultIfNotExist) {
    if (assignDefaultIfNotExist) {
        if (!this->mLayer) {
            this->mLayer = GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer();
        }
    }

    return mLayer;
}

int BaseShape::GetStoreId() {
    if (this->mLayer == NULL)
        return -1;
    return this->mLayer->GetObjectStore()->GetStoreId();
}

AbstractFrameState *BaseShape::AddFrameState(AbstractFrameState *frameState) {
    Assert(frameState != NULL);
    frameState->SetBaseShape(this);
    mFrameStates.push_back(FrameStatePair::FromState(frameState));

    return frameState;
}

AbstractFrameState *BaseShape::GetFrameState(const char *name) {
    for (auto frameState: mFrameStates) {
        if (strcmp(frameState.GetComponentPtr()->GetName(), name) == 0) {
            return frameState.GetComponentPtr();
        }
    }
    return NULL;
}

void BaseShape::SetLocalPivotPosition(float x, float y, float z) {

    Layer *shapeLayer = GetLayer();

    int currentFrameId = shapeLayer->GetCurrentFrame();
    ShapeTransformFrameState &frameState = GetFrameState<ShapeTransformFrameState>();
    TransformKeyFrame* pFrame = frameState.RecordLocalPivotPosition(currentFrameId, x, y, z);

    shapeLayer->AddKeyFrame(&pFrame->GetKeyFrame());
}

void BaseShape::SetBornFrameId(SInt32 bornFrameId) {
    mBornFrameId = bornFrameId;

    // TODO: How to handle this?? Do we need to handle this??
    // GetLayer()->AddKeyFrame(bornFrameId, this);
}

void BaseShape::SetGlobalPivotPosition(float x, float y, float z) {
    if (this->mRecordTransformationOfKeyFrame) {
        Layer *shapeLayer = GetLayer();
        int currentFrameId = shapeLayer->GetCurrentFrame();
        ShapeTransformFrameState &frameState = GetFrameState<ShapeTransformFrameState>();
        TransformKeyFrame* transformKeyFrame = frameState.RecordGlobalPivotPosition(currentFrameId, x, y, z);
        shapeLayer->AddKeyFrame(&transformKeyFrame->GetKeyFrame());
    } else { // Just update it temporarily
        GetFrameState<ShapeTransformFrameState>().UpdateTemporaryPosition(x, y, z);
    }
}

void BaseShape::SetRotation(float rotation) {

    if (this->mRecordTransformationOfKeyFrame) {
        Layer *shapeLayer = GetLayer();

        int currentFrameId = shapeLayer->GetCurrentFrame();
        ShapeTransformFrameState &frameState = GetFrameState<ShapeTransformFrameState>();
        TransformKeyFrame* transformKeyFrame = frameState.RecordRotation(currentFrameId, rotation);

        shapeLayer->AddKeyFrame(&transformKeyFrame->GetKeyFrame());
    } else {
        GetFrameState<ShapeTransformFrameState>().UpdateTemporaryRotation(rotation);
    }
}

void BaseShape::SetScale(float xScale, float yScale, float zScale) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    ShapeTransformFrameState &frameState = GetFrameState<ShapeTransformFrameState>();
    TransformKeyFrame* transformKeyFrame = frameState.RecordScale(currentFrameId, xScale, yScale, zScale);
    shapeLayer->AddKeyFrame(&transformKeyFrame->GetKeyFrame());
}

Vector3f *BaseShape::GetScale() {
    return GetFrameState<ShapeTransformFrameState>().GetScale();
}

bool BaseShape::IsVisibleInFrame(SInt32 frameId) {
    if (this->mBornFrameId < 0)
        return false;

    if (frameId < this->mBornFrameId)
        return false;

    Layer *shapeLayer = GetLayer();
    unsigned int bornFrameSpanHead = shapeLayer->GetTimeLineCellManager()->GetSpanHead(mBornFrameId);
    unsigned int currentFrameSpanHead = shapeLayer->GetTimeLineCellManager()->GetSpanHead(frameId);

    return bornFrameSpanHead == currentFrameSpanHead;
}

bool BaseShape::IsVisible() {
    if (!mIsVisible)
        return false;

    return IsVisibleInFrame(GetLayer()->GetCurrentFrame());
}

void BaseShape::SetSegments(float segmentBuffer[], int size) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    ShapeSegmentFrameState &frameState = GetFrameState<ShapeSegmentFrameState>();
    SegmentKeyFrame* segmentKeyFrame = frameState.RecordSegments(currentFrameId, segmentBuffer, size);
    shapeLayer->AddKeyFrame(&segmentKeyFrame->GetKeyFrame());
}

void BaseShape::SetSegmentsAtFrame(float segmentBuffer[], int size, int keyFrameId) {
    GetFrameState<ShapeSegmentFrameState>().RecordSegments(keyFrameId, segmentBuffer, size);
}

void BaseShape::RemoveSegment(int index) {
    this->GetFrameState<ShapeSegmentFrameState>().RemoveSegment(index);

    int currentFrameId = this->GetLayer()->GetCurrentFrame();
    this->Apply(currentFrameId);
}


AbstractFrameState *BaseShape::ProduceFrameStateByType(const HuaHuo::Type *type) {
    AbstractFrameState *component = AbstractFrameState::Produce(type);

    if (component == NULL) {
        return NULL;
    }

    Assert(component->Is<AbstractFrameState>());

    component->Reset();

    printf("Creating component at path:%s\n", StoreFilePath.c_str());
    GetPersistentManagerPtr()->MakeObjectPersistent(component->GetInstanceID(), StoreFilePath);

    return component;
}


AbstractFrameState *BaseShape::AddFrameStateByName(const char *frameStateName) {
    const HuaHuo::Type *componentType = HuaHuo::Type::FindTypeByName(frameStateName);
    if (componentType != NULL && componentType->IsDerivedFrom<AbstractFrameState>()) {
        AbstractFrameState *newFrameState = ProduceFrameStateByType(componentType);
        return AddFrameState(newFrameState);
    }

    return NULL;
}

AbstractFrameState *BaseShape::GetFrameStateByTypeName(const char *frameStateName) {
    const HuaHuo::Type *componentType = HuaHuo::Type::FindTypeByName(frameStateName);
    return QueryFrameStateByType(componentType);
}

template<class TransferFunction>
void FrameStatePair::Transfer(TransferFunction &transfer) {
    transfer.Transfer(component, "component");
    if (transfer.IsReadingPPtr()) {
        typeIndex = component ? component->GetType()->GetRuntimeTypeIndex() : 0;
    }
}

AbstractFrameState *BaseShape::QueryFrameStateByType(const HuaHuo::Type *type) const {
    // Find a component with the requested ID
    Container::const_iterator i;
    Container::const_iterator end = mFrameStates.end();
    for (i = mFrameStates.begin(); i != end; ++i) {
        if (type->IsBaseOf(i->GetTypeIndex()))
            return i->GetComponentPtr();
    }

    return NULL;
}

void FrameStatePair::SetComponentPtr(AbstractFrameState *const ptr) {
    if (ptr != NULL) {
        component = ptr;
        typeIndex = ptr->GetType()->GetRuntimeTypeIndex();
        return;
    }

    component = NULL;
    typeIndex = RTTI::DefaultTypeIndex;
}

void BaseShape::AddAnimationOffset(int offset) {
    // Find a component with the requested ID
    Container::const_iterator i;
    Container::const_iterator end = mFrameStates.end();
    for (i = mFrameStates.begin(); i != end; ++i) {
        i->GetComponentPtr()->AddAnimationOffset(offset);
    }
}