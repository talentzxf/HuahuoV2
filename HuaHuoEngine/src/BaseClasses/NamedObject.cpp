#include "NamedObject.h"
#include <cstring>

NamedObject::NamedObject(ObjectCreationMode mode)
    :   Super(mode)
{
}

//void NamedObject::ThreadedCleanup()
//{
//}

template<class TransferFunction>
void NamedObject::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    transfer.Transfer(m_Name, "m_Name", kHideInEditorMask | kIgnoreInMetaFiles);
}

void NamedObject::SetName(char const* name)
{
    if (strcmp(m_Name.c_str(), name) != 0)
    {
        m_Name.assign(name);
        SetDirty();
    }
}

IMPLEMENT_REGISTER_CLASS(NamedObject, 4);
IMPLEMENT_OBJECT_SERIALIZE(NamedObject);
INSTANTIATE_TEMPLATE_TRANSFER(NamedObject);
