//
// Created by VincentZhang on 4/28/2022.
//

#ifndef HUAHUOENGINE_ALLOCATORLABELS_H
#define HUAHUOENGINE_ALLOCATORLABELS_H

#include "Modules/ExportModules.h"

enum MemLabelIdentifier
{
    kMemTempLabels,

    // add temp labels first in the enum
#define DO_LABEL(Name)
#define DO_TEMP_LABEL(Name) kMem##Name##Id ,
#include "AllocatorLabelNames.h"
#undef DO_TEMP_LABEL
#undef DO_LABEL

    kMemRegularLabels,

    // then add regular labels
#define DO_LABEL(Name) kMem##Name##Id ,
#define DO_TEMP_LABEL(Name)
#include "AllocatorLabelNames.h"
#undef DO_TEMP_LABEL
#undef DO_LABEL

    kMemLabelCount
};

EXPORT_COREMODULE typedef int AllocationRootWithSalt;

struct EXPORT_COREMODULE MemLabelId
{
    AllocationRootWithSalt m_RootReferenceWithSalt;
    MemLabelIdentifier identifier;

private:
    // don't compare
    bool operator==(const MemLabelId& other);
    bool operator!=(const MemLabelId& other);
};

EXPORT_COREMODULE typedef MemLabelId MemLabelRef;
EXPORT_COREMODULE typedef int AllocationRootWithSalt;
EXPORT_COREMODULE typedef MemLabelId MemLabelRef;


// Used as a allocation root for an area which has non-persistent managers
struct EXPORT_COREMODULE MemLabelRootId
        {
                explicit MemLabelRootId(MemLabelRef label) : rootLabel(label) {}
                operator MemLabelId() const { return rootLabel; }
                MemLabelId rootLabel;
        };

inline EXPORT_COREMODULE MemLabelIdentifier GetLabelIdentifier(MemLabelRef label) {return label.identifier; }
inline EXPORT_COREMODULE AllocationRootWithSalt GetRootReference(MemLabelRef label) { return label.m_RootReferenceWithSalt; }
inline EXPORT_COREMODULE bool IsTempLabel(MemLabelRef label) { return GetLabelIdentifier(label) < kMemRegularLabels; }

inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelIdentifier id) { const MemLabelId memLabel = { id }; return memLabel; }
inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelIdentifier id, AllocationRootWithSalt rootRef) { const MemLabelId memLabel = { id }; return memLabel; }
inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelIdentifier id, void* memoryOwner) { const MemLabelId memLabel = { id }; return memLabel; }

inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelId lbl, AllocationRootWithSalt rootRef) { return CreateMemLabel(lbl.identifier); }
inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelId lbl, void* memoryOwner) { return CreateMemLabel(lbl.identifier); }
inline EXPORT_COREMODULE MemLabelId CreateMemLabel(MemLabelId lbl, MemLabelRef owner) { return lbl; }
inline EXPORT_COREMODULE MemLabelId SetCurrentMemoryOwner(MemLabelRef label) { return label; }


#if ENABLE_MEM_PROFILER
#define DO_LABEL_STRUCT(Name) extern EXPORT_COREMODULE MemLabelId Name;
#else
#define DO_LABEL_STRUCT(Name) const MemLabelId Name =  { Name##Id };
#endif
#define DO_LABEL(Name) DO_LABEL_STRUCT(kMem##Name)
#define DO_TEMP_LABEL(Name) DO_LABEL_STRUCT(kMem##Name)
#include "AllocatorLabelNames.h"
#undef DO_LABEL
#undef DO_TEMP_LABEL
#undef DO_LABEL_STRUCT
#endif //HUAHUOENGINE_ALLOCATORLABELS_H
