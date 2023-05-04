#include "SerializeReferenceLabels.h"
#include <string>

const std::string SerializeReferenceLabels::k1stReferencedNodeNameLabel = "00000000";
const std::string SerializeReferenceLabels::kReferencedObjectIdLabel = "id";
const std::string SerializeReferenceLabels::kManagedReferenceLabel = "managedReference";
const std::string SerializeReferenceLabels::kManagedRefArrayItemTypeLabel = "managedRefArrayItem";
const std::string SerializeReferenceLabels::kRegistryTypeLabel = "ManagedReferencesRegistry";
const std::string SerializeReferenceLabels::kRegistryLabel = "references";
const std::string SerializeReferenceLabels::kRegistryVersionLabel = "version";
const std::string SerializeReferenceLabels::kReferencedObjectTypeLabel = "ReferencedObject";
const std::string SerializeReferenceLabels::kReferencedTypeLabel = "type";
const std::string SerializeReferenceLabels::kReferencedObjectDataLabel = "ReferencedObjectData";
const std::string SerializeReferenceLabels::kReferencedObjDataLabel = "data";
const std::string SerializeReferenceLabels::kClassNameLabel = "class";
const std::string SerializeReferenceLabels::kNameSpaceLabel = "ns";
const std::string SerializeReferenceLabels::kAssemblyLabel = "asm";
const std::string SerializeReferenceLabels::kReferencedTypeTypeLabel = "ReferencedManagedType";

const std::string SerializeReferenceLabels::kEndOfTypeListKlassName = "Terminus";
const std::string SerializeReferenceLabels::kEndOfTypeListNameSpace = "UnityEngine.DMAT";
const std::string SerializeReferenceLabels::kEndOfTypeListAssembly = "FAKE_ASM";

const std::string SerializeReferenceLabels::kUnkownKlassName = "{null}";
