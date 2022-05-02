//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_INSTANCEID_H
#define HUAHUOENGINE_INSTANCEID_H

#include "baselib/include/IntegerDefinitions.h"

// With this switched on, you should get a compile error when you mix InstanceIDs with regular ints, or
// do arithmetic to them, etc. Fixing the entire codebase to handle InstanceIDs correctly is quite a big
// change to make and will be difficult to do all in one go, so we have it turned off for now, allowing
// people to use the InstanceID typedef but not enforcing it.
//
// Once all the code is switched over to using the InstanceID type correctly, we will turn this on for all
// debug builds. We *might* even turn it on for all release builds too, but we need to check that it does
// not degrade performance in any way when we do that.
#define ENFORCE_INSTANCEID_TYPE_SAFETY 0

#if ENFORCE_INSTANCEID_TYPE_SAFETY

struct InstanceID
{
    // The default constructor does not initialize 'value' to 0 - this is by design, for perf reasons.
    InstanceID() {}

    bool operator==(const InstanceID& other) const { return value == other.value; }
    bool operator!=(const InstanceID& other) const { return value != other.value; }

    bool operator<(const InstanceID& other) const { return value < other.value; }
    bool operator>(const InstanceID& other) const { return value > other.value; }
    bool operator<=(const InstanceID& other) const { return value <= other.value; }
    bool operator>=(const InstanceID& other) const { return value >= other.value; }

private:
    SInt32 value;
    InstanceID(SInt32 v) : value(v) {}

    friend InstanceID       InstanceID_Make(SInt32);
    friend SInt32&          InstanceID_AsSInt32Ref(InstanceID&);
    friend const SInt32&    InstanceID_AsSInt32Ref(const InstanceID&);
};

inline InstanceID InstanceID_Make(SInt32 value) { return InstanceID(value); }
static const InstanceID InstanceID_None = InstanceID_Make(0);

inline SInt32& InstanceID_AsSInt32Ref(InstanceID& id) { return id.value; }
inline const SInt32& InstanceID_AsSInt32Ref(const InstanceID& id) { return id.value; }

namespace core
{
    template<class T>
    struct hash;

    template<>
    struct hash<InstanceID>
    {
        UInt32 operator()(InstanceID i) const;
    };
}

#if ENABLE_UNIT_TESTS
inline UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const InstanceID& instanceID)
{
    return stream << InstanceID_AsSInt32Ref(instanceID);
}

#endif

#else

typedef SInt32 InstanceID;

inline InstanceID InstanceID_Make(SInt32 value) { return value; }
enum { InstanceID_None = 0 };

inline SInt32& InstanceID_AsSInt32Ref(InstanceID& id) { return id; }
inline const SInt32& InstanceID_AsSInt32Ref(const InstanceID& id) { return id; }

#endif

// GetInstanceIDFrom(x) retrieves an InstanceID value from a 'thing', intended for use in macros.
// We overload it to deal with all the things we want to get InstanceIDs from. Importantly it is
// implemented for InstanceID itself (i.e. just return the input) and Object (in BaseObject.h)
// which should cover most cases, but you may need to implement it for other non-Object-derived
// types that can have an InstanceID retrieved from them in a meaningful way.

inline InstanceID GetInstanceIDFrom(InstanceID id = InstanceID_None) { return id; }

#endif //HUAHUOENGINE_INSTANCEID_H
