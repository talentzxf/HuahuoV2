//
// Created by VincentZhang on 2022-11-08.
//

#include "CustomComponent.h"

IMPLEMENT_REGISTER_CLASS(CustomComponent, 10018);

IMPLEMENT_OBJECT_SERIALIZE(CustomComponent);

INSTANTIATE_TEMPLATE_TRANSFER(CustomComponent);

template<class TransferFunction>
void CustomComponent::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_FrameStates);
}

CustomComponent *CustomComponent::CreateComponent() {
    CustomComponent *producedComponent = Object::Produce<CustomComponent>();
    GetPersistentManagerPtr()->MakeObjectPersistent(producedComponent->GetInstanceID(), StoreFilePath);
    return producedComponent;
}

bool CustomComponent::Apply(int frameId) {
    for(auto frameState : m_FrameStates){
        frameState.GetComponentPtr()->Apply(frameId);
    }

    return true;
}

void CustomComponent::SetBaseShape(BaseShape *pBaseShape) {
    AbstractFrameState::SetBaseShape(pBaseShape);

    for(auto frameState : m_FrameStates){
        frameState.GetComponentPtr()->SetBaseShape(pBaseShape);
    }
}

int CustomComponent::GetKeyFrameCount() {
    std::set<int> keyframeIdSet = GetKeyFrameIds();

    int totalKeyFrames = keyframeIdSet.size();
    keyFrameIdCache.clear();
    for(int keyframeId : keyframeIdSet){
        keyFrameIdCache.push_back(keyframeId);
    }

    return totalKeyFrames;
}

int CustomComponent::GetKeyFrameAtIndex(int idx) {
    if(idx >= keyFrameIdCache.size())
        return -1;
    return keyFrameIdCache[idx];
}

vector<KeyFrameIdentifier> CustomComponent::GetKeyFrameIdentifiers() {
    std::vector<KeyFrameIdentifier> returnKeyFrameIdentifiers;

    for(auto frameState : m_FrameStates){
        auto keyframeIdentifiers = frameState.GetComponentPtr()->GetKeyFrameIdentifiers();
        returnKeyFrameIdentifiers.insert(returnKeyFrameIdentifiers.end(), keyframeIdentifiers.begin(), keyframeIdentifiers.end());
    }

    return returnKeyFrameIdentifiers;
}

void CustomComponent::DeleteKeyFrame(int frameId) {
    for(auto frameState : m_FrameStates){
        frameState.GetComponentPtr()->DeleteKeyFrame(frameId);
    }
}

void CustomComponent::ReverseKeyFrame(int startFrameId, int endFrameId) {
    for(auto frameState : m_FrameStates){
        frameState.GetComponentPtr()->ReverseKeyFrame(startFrameId, endFrameId);
    }
}


