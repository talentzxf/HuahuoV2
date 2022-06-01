//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_BEHAVIOUR_H
#define HUAHUOENGINE_BEHAVIOUR_H
#include "Components/BaseComponent.h"

class Behaviour: public BaseComponent{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(Behaviour);
    DECLARE_OBJECT_SERIALIZE();
public:
    Behaviour(MemLabelId label, ObjectCreationMode mode) : Super(label, mode) { m_Enabled = true; m_IsAdded = false; }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    virtual void Update() {}
    virtual void FixedUpdate() {}
    virtual void LateUpdate() {}

    static void InitializeClass();
    static void CleanupClass();

    /// Enable or disable updates of this behaviour
    virtual bool HasEnabled() const override { return true; }
    virtual bool GetEnabled() const override { return m_Enabled != 0; }
    virtual void SetEnabled(bool enab) override;

    bool IsAddedToManager() const { return m_IsAdded != 0; }

private:
    void UpdateEnabledState(bool active);

    ///@todo DO THIS PROPERLY. MORE SPACE EFFICIENT
    UInt8 m_Enabled;
    UInt8 m_IsAdded;
};


#endif //HUAHUOENGINE_BEHAVIOUR_H
