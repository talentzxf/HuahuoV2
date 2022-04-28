//
// Created by VincentZhang on 4/28/2022.
//

#ifndef PERSISTENTMANAGER_GAMEOBJECT_H
#define PERSISTENTMANAGER_GAMEOBJECT_H
#include "TypeSystem/Object.h"
#include "Memory/AllocatorLabels.h"
#include "ImmediatePtr.h"
#include "Serialize/SerializeUtility.h"
#include "Components/BaseComponent.h"

class GameObject : public Object{
    REGISTER_CLASS(GameObject);
    DECLARE_OBJECT_SERIALIZE();
public:
    struct ComponentPair
    {
        ComponentPair() {}

        static ComponentPair FromComponent(BaseComponent* component);
        static ComponentPair FromTypeIndexAndComponent(RuntimeTypeIndex typeIndex, BaseComponent* component);

        DECLARE_SERIALIZE(ComponentPair);

        inline RuntimeTypeIndex const GetTypeIndex() const { return typeIndex; }
        inline ImmediatePtr<BaseComponent> const& GetComponentPtr() const { return component; }

        void SetComponentPtr(BaseComponent* const ptr);

    private:
        RuntimeTypeIndex typeIndex;
        ImmediatePtr<BaseComponent> component;
    };

    typedef std::vector<ComponentPair>    Container;
    GameObject(MemLabelId label, ObjectCreationMode mode);

    // Adds a new Component to the GameObject.
    // Using the PersistentObject interface so that Components,
    // which are not loaded at the moment can be added.
    // Use GameObjectUtility instead, you must invoke specific callbacks etc.
    void AddComponentInternal(BaseComponent* component, bool awake = true /*, AwakeFromLoadQueue* queue = nullptr*/);

    // ------------------------------------------------------------------------

    template<class T> T& GetComponent() const;
    template<class T> T* QueryComponent() const;

    BaseComponent* QueryComponentByType(const HuaHuo::Type* type) const;

    virtual char const* GetName() const override { return m_Name.c_str(); }
    virtual void SetName(char const* name) override;

    virtual void SetHideFlags(HideFlags flags) override;

    /// An GameObject can either be active or inactive (Template GameObjects are always inactive)
    /// If an GameObject is active/inactive all its components have the same state as well.
    /// (Components that are not added to a gameobject are always inactive)
    /// Querying and messaging still works for inactive GameObjects and Components
    void Activate();
    void ActivateInternal() {m_IsActive = true; }

    bool IsActive() const;
    bool IsSelfActive() const { return m_IsActive; }
    void SetSelfActive(bool state);
private:
    void FinalizeAddComponentInternal(BaseComponent* component, bool awake/*AwakeFromLoadQueue* queue = nullptr*/);
    Container   m_Component;
    std::string  m_Name;

    bool            m_IsActive;
    mutable SInt8   m_IsActiveCached;
};

template<class T> inline
T& GameObject::GetComponent() const
{
    T* component = QueryComponent<T>();
    DebugAssertMsg(component != NULL, "GetComponent returned NULL. You cannot use GetComponent unless the component is guaranteed to be part of the GO");
    return *component;
}

template<class T> inline
T* GameObject::QueryComponent() const
{
    return static_cast<T*>(QueryComponentByType(TypeOf<T>()));
}

inline GameObject::ComponentPair GameObject::ComponentPair::FromComponent(BaseComponent* component)
{
    ComponentPair ret;
    ret.typeIndex = component->GetType()->GetRuntimeTypeIndex();
    ret.component = component;
    return ret;
}

#endif //PERSISTENTMANAGER_GAMEOBJECT_H
