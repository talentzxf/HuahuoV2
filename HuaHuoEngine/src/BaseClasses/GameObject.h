//
// Created by VincentZhang on 4/28/2022.
//

#ifndef HUAHUOENGINE_GAMEOBJECT_H
#define HUAHUOENGINE_GAMEOBJECT_H
#include "TypeSystem/Object.h"
#include "Memory/AllocatorLabels.h"
#include "ImmediatePtr.h"
#include "Serialize/SerializeUtility.h"
#include "Components/BaseComponent.h"
#include "Utilities/LinkedList.h"
#include "TagTypes.h"
#include "PPtr.h"
#include "GameObjectDefines.h"

class Transform;
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
    GameObject(/*MemLabelId label,*/ ObjectCreationMode mode);

    static void InitializeClass();
    static void CleanupClass();

    static bool ShouldClearActiveCached(AwakeFromLoadMode mode) { return (mode == kPersistentManagerAwakeFromLoadMode); }
    void ClearActiveCachedInternal() { m_IsActiveCached = -1; }

    void UpdateActiveGONode();

    // Adds a new Component to the GameObject.
    // Using the PersistentObject interface so that Components,
    // which are not loaded at the moment can be added.
    // Use GameObjectUtility instead, you must invoke specific callbacks etc.
    void AddComponentInternal(BaseComponent* component, bool awake = true /*, AwakeFromLoadQueue* queue = nullptr*/);

    // ------------------------------------------------------------------------

    template<class T> T& GetComponent() const;
    template<class T> T* QueryComponent() const;

    BaseComponent* QueryComponentByType(const HuaHuo::Type* type) const;
    BaseComponent* QueryComponentByExactType(const HuaHuo::Type* type) const;

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
//    void SetSelfActive(bool state);

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    virtual void Reset() override;

    // Internally used during object destruction to prevent double deletion etc.
    bool IsDestroying() const { return (m_ActivationState & kDestroying) != 0; }
    bool IsActivating() const { return (m_ActivationState & kActivatingOrDeactivating) != 0; }
    bool IsActivatingChildren() const { return (m_ActivationState & (kActivatingChildren | kDeactivatingChildren)) != 0; }
    bool IsDeactivatingComponents() const { return (m_ActivationState & kDeactivatingComponents) != 0; }

    void ReplaceTransformComponentInternal(Transform* newTransform/*, AwakeFromLoadQueue* queue*/);
    void AddFirstTransformComponentInternal(::Transform* newTransform/*, AwakeFromLoadQueue* queue = nullptr*/);

//    const HuaHuo::Type* GetComponentTypeAtIndex(int index) const;
//    template<class T> T& GetComponentAtIndex(int index) const;
    template<class T> T* QueryComponentAtIndex(int index) const;

    void ActivateAwakeRecursively(DeactivateOperation deactivateOperation = kNormalDeactivate);
    void ActivateAwakeRecursivelyInternal(DeactivateOperation deactivateOperation = kNormalDeactivate/*, AwakeFromLoadQueue &queue*/);

    /// Set the GameObject Layer.
    /// This is used for collisions and messaging
    void SetLayer(int layerIndex);
    int GetLayer() const    { return m_Layer; }
    UInt32 GetLayerMask() const { return 1 << m_Layer; }

    void TransformParentHasChanged();

    /// Send a message identified by messageID to all components if they can handle it
    void SendMessageAny(const MessageIdentifier& messageID, MessageData& messageData);

    /// Send a message identified by messageID to all components if they can handle it
    template<class T>
    void SendMessage(const MessageIdentifier& messageID, T messageData);
    void SendMessage(const MessageIdentifier& messageID);

    // VZ: JavaScript doesn't have generic, have to think another way to query component ...
    Transform* GetTransform();

private:
    enum ActivationState
    {
        kNotActivating          = 0,
        kActivatingChildren     = 1 << 0,
        kActivatingComponents   = 1 << 1,
        kDeactivatingChildren   = 1 << 2,
        kDeactivatingComponents = 1 << 3,
        kDestroying             = 1 << 4,
        kActivatingOrDeactivating = kActivatingChildren | kActivatingComponents | kDeactivatingChildren | kDeactivatingComponents,
    };
    ActivationState m_ActivationState;

    void FinalizeAddComponentInternal(BaseComponent* component, bool awake/*AwakeFromLoadQueue* queue = nullptr*/);

    template<class TransferFunction>
    void TransferComponents(TransferFunction& transfer);
    UInt32          m_Layer;
    UInt16          m_Tag;
    Container   m_Component;
    std::string  m_Name;

    bool            m_IsActive;
    mutable SInt8   m_IsActiveCached;

    ListNode<GameObject> m_ActiveGONode;
};

typedef List<ListNode<GameObject> > GameObjectList;

// A fast lookup for all tagged and active game objects
class GameObjectManager
{
public:
    static void StaticInitialize();
    static void StaticDestroy();

private:
    // Nodes that are tagged and active (excluding m_MainCameraTaggedNodes because nodes may only belong to one list)
    GameObjectList m_TaggedNodes;
    // Main Camera Nodes that are tagged and active
    GameObjectList m_MainCameraTaggedNodes;

public:
    // Nodes that are just active
    // (If you want to get all active nodes you need to go through tagged and active nodes)
    GameObjectList m_ActiveNodes;

    GameObjectList& GetTaggedNodes(UInt32 tag)
    {
        return (tag == kMainCameraTag) ? m_MainCameraTaggedNodes : m_TaggedNodes;
    }

    static GameObjectManager* s_Instance;
};
GameObjectManager& GetGameObjectManager();

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

template<class T> inline
T* GameObject::QueryComponentAtIndex(int index) const
{
    BaseComponent* comp = m_Component[index].GetComponentPtr();
    DebugAssert(comp != NULL);
    return dynamic_pptr_cast<T*>(comp);
}

template<class T> inline
void GameObject::SendMessage(const MessageIdentifier& messageID,
                             T messageData)
{
    MessageData data(messageData);
    SendMessageAny(messageID, data);
}

inline void GameObject::SendMessage(const MessageIdentifier& messageID)
{
    MessageData data;
    SendMessageAny(messageID, data);
}

#endif //HUAHUOENGINE_GAMEOBJECT_H
