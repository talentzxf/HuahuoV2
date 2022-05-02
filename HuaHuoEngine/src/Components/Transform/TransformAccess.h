#pragma once

#include "baselib/include/IntegerDefinitions.h"

struct TransformHierarchy;

struct TransformAccessReadOnly
{
    TransformHierarchy* hierarchy;
    UInt32              index;

    TransformAccessReadOnly(const TransformHierarchy* i_hierarchy, UInt32 i_Index) : hierarchy(const_cast<TransformHierarchy*>(i_hierarchy)), index(i_Index) {}
    TransformAccessReadOnly() {}

    friend inline bool operator==(const TransformAccessReadOnly& lhs, const TransformAccessReadOnly& rhs)
    {
        return (lhs.hierarchy == rhs.hierarchy) & (lhs.index == rhs.index);
    }

    friend inline bool operator!=(const TransformAccessReadOnly& lhs, const TransformAccessReadOnly& rhs)
    {
        return (lhs.hierarchy != rhs.hierarchy) | (lhs.index != rhs.index);
    }

    friend inline bool operator<(const TransformAccessReadOnly& lhs, const TransformAccessReadOnly& rhs)
    {
        if (lhs.hierarchy != rhs.hierarchy)
            return lhs.hierarchy < rhs.hierarchy;
        else
            return lhs.index < rhs.index;
    }

    friend inline bool operator<=(const TransformAccessReadOnly& lhs, const TransformAccessReadOnly& rhs)
    {
        return !(rhs < lhs);
    }
};

struct TransformAccess : public TransformAccessReadOnly
{
    TransformAccess(const TransformHierarchy* i_hierarchy, UInt32 i_Index) : TransformAccessReadOnly(i_hierarchy, i_Index) {}
    TransformAccess() {}

    static TransformAccess Null() { return TransformAccess(NULL, 0); }
};


//BIND_MANAGED_TYPE_NAME(TransformAccess, UnityEngine_Jobs_TransformAccess);
