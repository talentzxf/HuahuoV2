//
// Created by VincentZhang on 2022-11-08.
//

#include "CustomComponent.h"
#include "Serialize/PersistentManager.h"
#include "Shapes/BaseShape.h"

extern std::string StoreFilePath;

IMPLEMENT_REGISTER_CLASS(CustomComponent, 10018);

IMPLEMENT_OBJECT_SERIALIZE(CustomComponent);

INSTANTIATE_TEMPLATE_TRANSFER(CustomComponent);

template<class TransferFunction>
void CustomComponent::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_FrameStates);
    TRANSFER(m_SubComponents);
}

int CustomComponent::RegisterShapeValue(const char *fieldName) {
    int fieldIdx = this->RegisterField(fieldName, SHAPE);
    CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
    pComponent->GetDefaultValueData()->dataType = SHAPE;
    pComponent->GetDefaultValueData()->shapeValue = NULL;
    return fieldIdx;
}

CustomComponent *CustomComponent::CreateComponent(const char *componentTypeName) {
    const HuaHuo::Type *shapeType = HuaHuo::Type::FindTypeByName(componentTypeName);
    if (shapeType == NULL || !shapeType->IsDerivedFrom<CustomComponent>()) {
        return NULL;
    }

    CustomComponent *component = (CustomComponent *) Object::Produce(shapeType);
    GetPersistentManagerPtr()->MakeObjectPersistent(component->GetInstanceID(), StoreFilePath);
    return component;
}

bool CustomComponent::Apply(int frameId) {
    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->Apply(frameId);
    }

    return true;
}

void CustomComponent::SetBaseShape(BaseShape *pBaseShape) {
    AbstractFrameState::SetBaseShape(pBaseShape);

    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->SetBaseShape(pBaseShape);
    }
}

int CustomComponent::GetKeyFrameCount() {
    std::set<int> keyframeIdSet = GetKeyFrameIds();

    int totalKeyFrames = keyframeIdSet.size();
    keyFrameIdCache.clear();
    for (int keyframeId: keyframeIdSet) {
        keyFrameIdCache.push_back(keyframeId);
    }

    return totalKeyFrames;
}

int CustomComponent::GetKeyFrameAtIndex(int idx) {
    if (idx >= keyFrameIdCache.size())
        return -1;
    return keyFrameIdCache[idx];
}

vector<KeyFrameIdentifier> CustomComponent::GetKeyFrameIdentifiers() {
    std::vector<KeyFrameIdentifier> returnKeyFrameIdentifiers;

    for (auto frameState: m_FrameStates) {
        auto keyframeIdentifiers = frameState.GetComponentPtr()->GetKeyFrameIdentifiers();
        returnKeyFrameIdentifiers.insert(returnKeyFrameIdentifiers.end(), keyframeIdentifiers.begin(),
                                         keyframeIdentifiers.end());
    }

    return returnKeyFrameIdentifiers;
}

void CustomComponent::DeleteKeyFrame(int frameId, bool notifyFrontEnd) {
    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->DeleteKeyFrame(frameId, notifyFrontEnd);
    }
}

bool CustomComponent::ReverseKeyFrame(int startFrameId, int endFrameId, int currentFrameId) {
    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->ReverseKeyFrame(startFrameId, endFrameId, currentFrameId);
    }
    return true;
}

void CustomComponent::SaveAsKeyFrame() {
    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->SaveAsKeyFrame();
    }
}

void CustomComponent::MoveToStore(ObjectStore *pStore) {
    for (auto frameState: m_FrameStates) {
        frameState.GetComponentPtr()->MoveToStore(pStore);
    }
}


