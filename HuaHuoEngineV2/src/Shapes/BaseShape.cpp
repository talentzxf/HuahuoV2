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
void BaseShape::TransferFrameStates(TransferFunction& transfer){
    // When cloning objects for prefabs and instantiate, we don't use serialization to duplicate the hierarchy,
    // we duplicate the hierarchy directly
    if (!SerializePrefabIgnoreProperties(transfer))
        return;

    if (transfer.IsWriting() && transfer.NeedsInstanceIDRemapping())
    {
        Container filtered_framestates;
        for (Container::iterator i = mFrameStates.begin(); i != mFrameStates.end(); i++)
        {
            LocalSerializedObjectIdentifier localIdentifier;
            InstanceIDToLocalSerializedObjectIdentifier(i->GetComponentPtr()->GetInstanceID(), localIdentifier);
            if (localIdentifier.localIdentifierInFile != 0)
                filtered_framestates.push_back(*i);
        }
        transfer.Transfer(filtered_framestates, "mFrameStates", kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);
        return;
    }



    transfer.Transfer(mFrameStates, "mFrameStates", kHideInEditorMask | kStrongPPtrMask | kDisallowSerializedPropertyModification);
}

void BaseShape::AwakeFromLoad(AwakeFromLoadMode awakeMode) {

    int frameId = this->GetLayer()->GetCurrentFrame();
    Apply(frameId);

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

Layer *BaseShape::GetLayer() {
    if (!this->mLayer) {
        this->mLayer = GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer();
    }

    return mLayer;
}

AbstractFrameState *BaseShape::AddFrameStateInternal(AbstractFrameState *frameState) {
    Assert(frameState != NULL);
    mFrameStates.push_back(FrameStatePair::FromState(frameState));

    return frameState;
}

void BaseShape::SetPosition(float x, float y, float z) {

    Layer *shapeLayer = GetLayer();

    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeTransformFrameState>().RecordPosition(currentFrameId, x, y, z);

    shapeLayer->AddKeyFrame(currentFrameId, this);
}

void BaseShape::SetLocalCenterPosition(float x, float y, float z) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeTransformFrameState>().RecordCenterOffset(currentFrameId, x, y, z);
    shapeLayer->AddKeyFrame(currentFrameId, this);
}

void BaseShape::SetRotation(float rotation) {

    Layer *shapeLayer = GetLayer();

    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeTransformFrameState>().RecordRotation(currentFrameId, rotation);

    shapeLayer->AddKeyFrame(currentFrameId, this);
}

void BaseShape::SetScale(float xScale, float yScale, float zScale) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeTransformFrameState>().RecordScale(currentFrameId, xScale, yScale, zScale);
    shapeLayer->AddKeyFrame(currentFrameId, this);
}

Vector3f *BaseShape::GetScale() {
    return GetFrameState<ShapeTransformFrameState>().GetScale();
}

void BaseShape::SetColor(float r, float g, float b, float a) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeColorFrameState>().RecordColor(currentFrameId, r, g, b, a);

    shapeLayer->AddKeyFrame(currentFrameId, this);
}

void BaseShape::SetStrokeColor(float r, float g, float b, float a) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeStrokeColorFrameState>().RecordColor(currentFrameId, r, g, b, a);

    shapeLayer->AddKeyFrame(currentFrameId, this);
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
    GetFrameState<ShapeSegmentFrameState>().RecordSegments(currentFrameId, segmentBuffer, size);
    shapeLayer->AddKeyFrame(currentFrameId, this);
}

void BaseShape::SetStrokeWidth(float strokeWidth) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    GetFrameState<ShapeStrokeWidthFrameState>().RecordStrokeWidth(currentFrameId, strokeWidth);
    shapeLayer->AddKeyFrame(currentFrameId, this);
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

    GetPersistentManagerPtr()->MakeObjectPersistent(component->GetInstanceID(), StoreFilePath);

    return component;
}


AbstractFrameState *BaseShape::AddFrameStateByName(const char *frameStateName) {
    const HuaHuo::Type *componentType = HuaHuo::Type::FindTypeByName(frameStateName);
    if (componentType != NULL && componentType->IsDerivedFrom<AbstractFrameState>()) {
        AbstractFrameState *newFrameState = ProduceFrameStateByType(componentType);
        return AddFrameStateInternal(newFrameState);
    }

    return NULL;
}

template<class TransferFunction>
void BaseShape::FrameStatePair::Transfer(TransferFunction &transfer) {
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

void BaseShape::FrameStatePair::SetComponentPtr(AbstractFrameState *const ptr) {
    if (ptr != NULL) {
        component = ptr;
        typeIndex = ptr->GetType()->GetRuntimeTypeIndex();
        return;
    }

    component = NULL;
    typeIndex = RTTI::DefaultTypeIndex;
}