#pragma once
#include <string>

//----------------------------------------------------------------------------------------------------------------------
// What is this: labels used when generating type tree nodes related to managed references.
//
// Motivation  : just to keep things tidy and easily located.
//----------------------------------------------------------------------------------------------------------------------
class SerializeReferenceLabels
{
public:
    static const std::string k1stReferencedNodeNameLabel;
    static const std::string kReferencedObjectIdLabel;
    static const std::string kManagedReferenceLabel;
    static const std::string kManagedRefArrayItemTypeLabel;
    static const std::string kRegistryTypeLabel;
    static const std::string kReferencedObjectTypeLabel;
    static const std::string kReferencedTypeLabel;
    static const std::string kReferencedObjectDataLabel;
    static const std::string kReferencedObjDataLabel;
    static const std::string kReferencedTypeTypeLabel;
    static const std::string kClassNameLabel;
    static const std::string kNameSpaceLabel;
    static const std::string kAssemblyLabel;
    static const std::string kRegistryLabel;
    static const std::string kRegistryVersionLabel;

    static const std::string kEndOfTypeListKlassName;
    static const std::string kEndOfTypeListNameSpace;
    static const std::string kEndOfTypeListAssembly;

    static const std::string kUnkownKlassName;
};
