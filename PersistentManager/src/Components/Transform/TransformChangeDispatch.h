//
// Created by VincentZhang on 4/24/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFORMCHANGEDISPATCH_H
#define PERSISTENTMANAGER_TRANSFORMCHANGEDISPATCH_H

#include "TransformHierarchy.h"
#include "BaseClasses/BaseTypes.h"
#include "Utilities/EnumFlags.h"
#include <string>
#include <unordered_map>

struct TransformChangeSystemHandle
{
    UInt32 index;

    enum { kInvalidIndex = 0xFFFFFFFF };

    TransformChangeSystemHandle() { index = kInvalidIndex; }
    TransformChangeSystemMask Mask() const { Assert(index != kInvalidIndex); return TransformChangeSystemMask(UInt64(1) << index); }
    bool IsValid() const { return index != kInvalidIndex; }
};

class TransformHierarchyChangeDispatch
{
public:
    typedef void DispatchFunc (const TransformAccess* transforms, unsigned count);

    // Describes events that systems can register an interest in.
    // Can be ORed together if system has multiple interests.
    enum InterestType
    {
        // Event is sent when the transform hierarchy or transform index changes.
        // This is useful for caching TransformAccess in low level code,
        // and recaching it after SetParent etc.
        // NOTE: Destruction of a game object does not trigger this event on the destroyed objects, use kInterestedInDestruction instead.
        kInterestedInTransformAccess = 1 << 0,

        // Event is sent if any direct or indirect children have been added,
        // removed, reparented, or sibling index changed.
        // NOTE: This event is not sent if the transform itself is being parented
        //       or any of it's parents are being reparented.
        // Basically this event is sent up the hierarchy not down.
        kInterestedInChildHierarchy = 1 << 1,

        // Event is sent if any direct or indirect parents have been added,
        // removed or reparented.
        kInterestedInParentHierarchy = 1 << 2,

        // Event is sent for each TransformAccess that will be destroyed, just before it will be destroyed.
        kInterestedInDestruction = 1 << 3,

        // Event is sent for each TransformAccess after it's Transform component has been replaced, e.g. with a RectTransform.
        kInterestedInReplacement = 1 << 4,

        // Event is sent for each TransformAccess before the hierarchy is removed but the Transform isn't being destroyed (yet).
        // Used for the very few cases where there is no corresponding component on the GameObject, e.g. TransformAccessArray.
        // NOTE: Only sent if kInterestedInDestruction has not been sent.
        kInterestedInHierarchyClear = 1 << 5,

        kInterestedCount = 6,

        kInterestedInEverything = (1 << kInterestedCount) - 1
    };

    // To help find bugs, one bit of the interest mask is used for poisoning.
    static const int kMaxSupportedSystems = sizeof(UInt32) * 8 - 1;
    static const UInt32 kPoisonMask = 1 << kMaxSupportedSystems;

    // Lifecycle.
    ~TransformHierarchyChangeDispatch();
    static void InitializeClass(void*);
    static void CleanupClass(void*);

//    // Register a transform hierarchy change system.
//    // When any events that the system is interested in involves transforms
//    // that it is interested in, the callback will be invoked with those transforms.
//    TransformHierarchyChangeSystemHandle RegisterSystem(const char* name, InterestType interestType, DispatchFunc* callback);
//
//    TransformHierarchyChangeSystemHandle RegisterPermanentInterestSystem(const char* name, const Unity::Type* type, InterestType interestType, DispatchFunc* callback);
//
//    // Unregister a transform hierarchy change system.
//    void UnregisterSystem(TransformHierarchyChangeSystemHandle& system);
//
//    // Is this a registered transform hierarchy change system?
//    bool IsRegisteredSystem(TransformHierarchyChangeSystemHandle system);
//
//    // The mask representing all registered systems.
//    UInt32 GetAllRegisteredSystemsMask() { return m_AllRegisteredSystemsMask; }
//
//    // The name of a registered system.
//    const char* GetSystemName(TransformHierarchyChangeSystemHandle system);
//
//    // The names of multiple registered systems.
//    core::string GetSystemNames(UInt32 systemMask);
//
//    // Add or remove this transform as an interest of this transform hierarchy change system.
//    static void SetSystemInterested(TransformAccess transform, TransformHierarchyChangeSystemHandle system, bool enable);
//
//    // Is this transform an interest of this transform hierarchy change system?
//    static bool GetSystemInterested(TransformAccessReadOnly transform, TransformHierarchyChangeSystemHandle system);
//
//    // Invoke appropriate callbacks for this transform only.
//    void DispatchSelfOnly(TransformAccess transform, InterestType interestType);
//
//    // Invoke appropriate callbacks for this transform and it's direct and
//    // indirect parents, i.e. up the hierarchy.
//    void DispatchSelfAndParents(TransformAccess transform, InterestType interestType);
//
    // Invoke appropriate callbacks for this transform and it's direct and
    // indirect children, i.e. down the hierarchy.
    void DispatchSelfAndAllChildren(TransformAccess transform, InterestType interestType);
//
//    UInt32 GetPermanentInterestMask() const { return m_PermanentInterestSystemsMask; }
//    void AddPermanentInterests(TransformAccess access, const Unity::Type* type);
//    void RemovePermanentInterests(TransformAccess access, const Unity::Type* type);
//    void RecalculatePermanentInterests(TransformAccess access);
//    void ValidateInterests(TransformAccessReadOnly access);

private:
    typedef std::unordered_map<RuntimeTypeIndex, UInt32> TypeSystemMaskMap;

    struct System
    {
        InterestType interestType;
        DispatchFunc* callback;
        std::string name;

        System() : interestType(InterestType(0)), callback(NULL), name("") {}
    };

    // Forbid general usage of constructors or assignment.
    TransformHierarchyChangeDispatch();
    TransformHierarchyChangeDispatch(const TransformHierarchyChangeDispatch&) {}
    TransformHierarchyChangeDispatch& operator=(const TransformHierarchyChangeDispatch&) { return *this; }

#if ENABLE_UNIT_TESTS
    friend struct TransformHierarchyChangeDispatchTestAccess;
#endif

    UInt32 m_AllRegisteredSystemsMask;
    System m_Systems[kMaxSupportedSystems];

    UInt32 m_PermanentInterestSystemsMask;
    TypeSystemMaskMap m_TypePermanentInterestMasks;
};

ENUM_FLAGS(TransformHierarchyChangeDispatch::InterestType);

inline TransformHierarchyChangeDispatch& GetTransformHierarchyChangeDispatch()
{
    extern TransformHierarchyChangeDispatch* gTransformHierarchyChangeDispatch;
    return *gTransformHierarchyChangeDispatch;
}


#endif //PERSISTENTMANAGER_TRANSFORMCHANGEDISPATCH_H
