//
// Created by VincentZhang on 5/1/2022.
//

#include "PathNamePersistentManager.h"
#include "SerializedFile.h"

void InitPathNamePersistentManager()
{
    SetPersistentManager(NEW(PathNamePersistentManager /*, kMemManager, "Managers", "PathNameManager"*/) ());
    // InitializeStdConverters();
}

FileIdentifier PathNamePersistentManager::PathIDToFileIdentifierInternal(int pathID) const
{
    // __FAKEABLE_METHOD__(PathNamePersistentManager, PathIDToFileIdentifierInternal, (pathID));

    Assert(pathID >= 0 && pathID < (int)m_PathNames.size());
    FileIdentifier f;
    f.pathName = m_PathNames[pathID];
    return f;
}