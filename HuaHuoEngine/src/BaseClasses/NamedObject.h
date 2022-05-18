#pragma once

#include "Components/BaseComponent.h"
#include <string>

class EXPORT_COREMODULE NamedObject : public BaseComponent
{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(NamedObject);
    DECLARE_OBJECT_SERIALIZE();
public:

    virtual char const* GetName() const override { return m_Name.c_str(); }
    virtual void SetName(char const* name) override;
    NamedObject(ObjectCreationMode mode);

//    inline void MarkGameObjectAndComponentDependencies(GarbageCollectorThreadState& gc) const {}

protected:

    std::string m_Name;
};
