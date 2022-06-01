//
// Created by VincentZhang on 4/10/2022.
//

#ifndef HUAHUOENGINE_BASECOMPONENT_H
#define HUAHUOENGINE_BASECOMPONENT_H


#include "TypeSystem/Object.h"
#include "BaseClasses/ImmediatePtr.h"
#include "BaseClasses/GameObjectDefines.h"
#include "BaseClasses/MessageIdentifier.h"
#include "BaseClasses/MessageData.h"

DECLARE_MESSAGE_IDENTIFIER(kLayerChanged);
DECLARE_MESSAGE_IDENTIFIER(kDidAddComponent);
DECLARE_MESSAGE_IDENTIFIER(kDidRemoveComponent);

class GameObject;
class BaseComponent: public Object{
    REGISTER_CLASS(BaseComponent);
    DECLARE_OBJECT_SERIALIZE();
private:
    friend class GameObject;
private:
    ImmediatePtr<GameObject>    m_GameObject;
public:
    BaseComponent(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {
        m_GameObject = NULL;
    }

    // Returns a reference to the GameObject holding this component
    GameObject& GetGameObject()                    { return *m_GameObject; }
    const GameObject& GetGameObject() const        { return *m_GameObject; }
    GameObject* GetGameObjectPtr()     { return m_GameObject; }
    GameObject* GetGameObjectPtr() const   { return m_GameObject; }

    template<class T> T& GetComponent() const;
    template<class T> T* QueryComponent() const;

    /// Is this component active?
    /// A component is always inactive if its not attached to a gameobject
    /// A component is always inactive if its gameobject is inactive
    /// If its a prefab, the gameobject and its components are always set to be inactive
    bool IsActive() const;

    virtual char const* GetName() const override;
    virtual void SetName(char const* name) override;

    // Indicates whether this component supports enabling. If false, GetEnabled always returns true and
    // SetEnabled is considered an error.
    virtual bool HasEnabled() const { return false; }
    virtual bool GetEnabled() const { return true; }
    virtual void SetEnabled(bool enable);

    InstanceID GetGameObjectInstanceID() const;

    /// SetGameObject is called whenever the GameObject of a component changes.
    void SetGameObjectInternal(GameObject* go);

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    /// Deactivate will be called just before the Component is going to be removed from a GameObject
    /// It can still communicate with other components at this point.
    /// Deactivate will only be called when the component is remove from the GameObject,
    /// not if the object is persistet to disk and removed from memory
    /// Deactivate will only be called if the GameObject the Component is being removed from is active
    /// YOU CAN NOT RELY ON IsActive returning false inside Deactivate
    virtual void Deactivate(DeactivateOperation /*operation*/) {}

    bool IsPrefabAsset() const { return false; }

    /// Send a message identified by messageName to every components of the gameobject
    /// that can handle it
    void SendMessageAny(const MessageIdentifier& messageID, MessageData& messageData);

    template<class T>
    void SendMessage(const MessageIdentifier& messageID, T messageData);
    void SendMessage(const MessageIdentifier& messageID);
};

template<class T> inline
void BaseComponent::SendMessage(const MessageIdentifier& messageID,
                                   T messageData)
{
    MessageData data(messageData);
    SendMessageAny(messageID, data);
}

inline void BaseComponent::SendMessage(const MessageIdentifier& messageID)
{
    MessageData data;
    SendMessageAny(messageID, data);
}

#endif //HUAHUOENGINE_BASECOMPONENT_H
