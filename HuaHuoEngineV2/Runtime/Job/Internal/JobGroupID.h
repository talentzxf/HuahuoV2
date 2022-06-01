#pragma once

class JobGroup;

struct JobGroupID
{
    JobGroup*   group;
    unsigned    version;

    JobGroupID()
        : group(0)
        , version(0u)
    {}
};

inline bool operator==(const JobGroupID& lhs, const JobGroupID& rhs)
{
    return lhs.group == rhs.group && lhs.version == rhs.version;
}

inline bool operator!=(const JobGroupID& lhs, const JobGroupID& rhs)
{
    return lhs.group != rhs.group || lhs.version != rhs.version;
}

inline bool operator<(const JobGroupID& lhs, const JobGroupID& rhs)
{
    if (lhs.group != rhs.group)
        return lhs.group < rhs.group;
    else
        return lhs.version < rhs.version;
}
