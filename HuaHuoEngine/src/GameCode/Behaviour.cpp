//
// Created by VincentZhang on 5/13/2022.
//

#include "Behaviour.h"

template<class TransferFunc>
void Behaviour::Transfer(TransferFunc& transfer)
{
    Super::Transfer(transfer);
    transfer.Transfer(m_Enabled, "m_Enabled", kHideInEditorMask | kTreatIntegerValueAsBoolean);
    transfer.Align();
}

void Behaviour::AwakeFromLoad(AwakeFromLoadMode awakeMode)
{
    Super::AwakeFromLoad(awakeMode);
    UpdateEnabledState(IsActive());
}

void Behaviour::SetEnabled(bool enab)
{
    if ((bool)m_Enabled == enab)
        return;
    m_Enabled = enab;
    UpdateEnabledState(IsActive());
    SetDirty();
}

void Behaviour::UpdateEnabledState(bool active)
{
    bool shouldBeAdded = active && m_Enabled;
    if (shouldBeAdded == (bool)m_IsAdded)
        return;

// VZ: Implement this later....

//    // Set IsAdded flag before adding/removing from manager. Otherwise if we get enabled update
//    // from inside of AddToManager/RemoveFromManager, we'll early out in the check above because
//    // flag is not set yet!
//    if (shouldBeAdded)
//    {
//        m_IsAdded = true;
//        AddToManager();
//    }
//    else
//    {
//        m_IsAdded = false;
//        RemoveFromManager();
//    }
}

void Behaviour::InitializeClass()
{
//    CreateInstanceBehaviourManager();
//    CreateInstanceFixedBehaviourManager();
//    CreateInstanceLateBehaviourManager();
//    CreateInstanceUpdateManager();
}

void Behaviour::CleanupClass()
{
//    ReleaseInstanceBehaviourManager();
//    ReleaseInstanceFixedBehaviourManager();
//    ReleaseInstanceLateBehaviourManager();
//    ReleaseInstanceUpdateManager();
}

IMPLEMENT_REGISTER_CLASS(Behaviour, 7);
IMPLEMENT_OBJECT_SERIALIZE(Behaviour);
INSTANTIATE_TEMPLATE_TRANSFER(Behaviour);