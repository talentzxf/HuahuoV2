//
// Created by VincentZhang on 4/28/2022.
//

#include "GameObject.h"
#include "Components/BaseComponent.h"
#include "Components/Transform/Transform.h"

void GameObject::SetName(char const* name)
{
    m_Name.assign(name);
//    if (s_SetGONameCallback)
//        s_SetGONameCallback(this);
    SetDirty();
}

void GameObject::AddComponentInternal(BaseComponent* com, bool awake /*, AwakeFromLoadQueue* queue*/)
{
    Assert(com != NULL);
    m_Component.push_back(ComponentPair::FromComponent(com));
    FinalizeAddComponentInternal(com, awake/*, queue*/);
}

void GameObject::SetHideFlags(HideFlags flags)
{
    Super::SetHideFlags(flags);
    for (size_t i = 0; i < m_Component.size(); i++)
    {
        BaseComponent& com = *m_Component[i].GetComponentPtr();
        com.SetHideFlags(flags);
    }
}

bool GameObject::IsActive() const
{
    if (m_IsActiveCached != -1)
        return m_IsActiveCached;

    // Calculate active state based on the hierarchy
    m_IsActiveCached = m_IsActive && !IsPersistent();
    Transform *trs = QueryComponent<Transform>();
    if (trs)
    {
        Transform *parent = trs->GetParent();
        if (parent && parent->GetGameObjectPtr())
        {
            m_IsActiveCached = m_IsActiveCached && parent->GetGameObjectPtr()->IsActive();
        }
    }

    return m_IsActiveCached;
}

void GameObject::FinalizeAddComponentInternal(BaseComponent* com, bool awake/*, AwakeFromLoadQueue* queue*/)
{
    // Make sure it isn't already added to another GO
    Assert(com->GetGameObject().GetInstanceID() == InstanceID_None || com->GetGameObjectPtr() == this);

    com->SetGameObjectInternal(this);
    com->SetHideFlags(GetHideFlags());

    // SetSupportedMessagesDirty();

#if UNITY_EDITOR
    if (queue)
        queue->Add(*com);
    else if (awake)
#endif
    {
        if (IsActive())
            com->AwakeFromLoad(kActivateAwakeFromLoad);
        else
            com->AwakeFromLoad(kDefaultAwakeFromLoad);
    }

    com->SetDirty();
    SetDirty();
}

BaseComponent* GameObject::QueryComponentByType(const HuaHuo::Type* type) const
{
    // Find a component with the requested ID
    Container::const_iterator i;
    Container::const_iterator end = m_Component.end();
    for (i = m_Component.begin(); i != end; ++i)
    {
        if (type->IsBaseOf(i->GetTypeIndex()))
            return i->GetComponentPtr();
    }

    return NULL;
}
