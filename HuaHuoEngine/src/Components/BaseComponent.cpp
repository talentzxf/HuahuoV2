//
// Created by VincentZhang on 4/10/2022.
//

#include "BaseComponent.h"
#include "Logging/LogAssert.h"
#include "BaseClasses/GameObject.h"

IMPLEMENT_REGISTER_CLASS(BaseComponent, 2);

template<class TransferFunction>
void BaseComponent::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

//    if (SerializePrefabIgnoreProperties(transfer))
//        transfer.Transfer(m_GameObject, "m_GameObject", kHideInEditorMask | kStrongPPtrMask);
}

char const* BaseComponent::GetName() const
{
    if (m_GameObject)
        return m_GameObject->GetName();
    else
        return GetTypeName();
}

void BaseComponent::SetName(char const* name)
{
    if (m_GameObject)
        m_GameObject->SetName(name);
}

void BaseComponent::AwakeFromLoad(AwakeFromLoadMode awakeMode)
{
    Super::AwakeFromLoad(awakeMode);

    // Force load the game object. This is in order to prevent ImmediatePtrs not being dereferenced after loading.
    // Which can cause a crash in Resources.UnloadUnusedAssets()
    // Resources.Load used to store incorrect preload data which made this trigger.
    ::GameObject* dereferenceGameObject = m_GameObject;
    UNUSED(dereferenceGameObject);
}

void BaseComponent::SetGameObjectInternal(GameObject* go)
{
    m_GameObject = go;
#if UNITY_EDITOR
    SetIsPreviewSceneObject(go ? go->IsPreviewSceneObject() : false);
#endif
}

void BaseComponent::SetEnabled(bool enable)
{
    AssertFormatMsg(!HasEnabled(), "Component of class '%s' overrides HasEnabled but not SetEnabled().", GetTypeName());

    // WarningStringMsg("Component of class '%s' does not support SetEnabled calls. Do not call SetEnabled on this class.", GetTypeName());
}

InstanceID BaseComponent::GetGameObjectInstanceID() const
{
    return m_GameObject.GetInstanceID();
}

void BaseComponent::SendMessageAny(const MessageIdentifier& messageID, MessageData& messageData)
{
    GameObject* go = GetGameObjectPtr();
    if (go)
        go->SendMessageAny(messageID, messageData);
}

IMPLEMENT_OBJECT_SERIALIZE(BaseComponent);
INSTANTIATE_TEMPLATE_TRANSFER(BaseComponent);