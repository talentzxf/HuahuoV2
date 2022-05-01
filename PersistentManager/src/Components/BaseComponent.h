//
// Created by VincentZhang on 4/10/2022.
//

#ifndef PERSISTENTMANAGER_BASECOMPONENT_H
#define PERSISTENTMANAGER_BASECOMPONENT_H


#include "TypeSystem/Object.h"
#include "BaseClasses/ImmediatePtr.h"
#include "BaseClasses/GameObjectDefines.h"

class GameObject;
class BaseComponent: public Object{
    REGISTER_CLASS(BaseComponent);
    DECLARE_OBJECT_SERIALIZE();
private:
    friend class GameObject;
private:
    ImmediatePtr<GameObject>    m_GameObject;
public:
    BaseComponent(ObjectCreationMode mode)
        :Super(mode)
    {
        m_GameObject = NULL;
    }

    // Returns a reference to the GameObject holding this component
    GameObject& GetGameObject()                    { return *m_GameObject; }
    const GameObject& GetGameObject() const        { return *m_GameObject; }
    GameObject* GetGameObjectPtr()     { return m_GameObject; }
    GameObject* GetGameObjectPtr() const   { return m_GameObject; }

//    template<class T> T& GetComponent() const { return m_GameObject->GetComponent<T>(); }
//    template<class T> T* QueryComponent() const { return m_GameObject->QueryComponent<T>(); }

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
};


#endif //PERSISTENTMANAGER_BASECOMPONENT_H
