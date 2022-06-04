//
// Created by VincentZhang on 5/1/2022.
//

#include "PathNamePersistentManager.h"
#include "SerializedFile.h"
#include "Utilities/Word.h"

void InitPathNamePersistentManager()
{
    SetPersistentManager(HUAHUO_NEW_AS_ROOT(PathNamePersistentManager , kMemManager, "Managers", "PathNameManager") ());
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

int PathNamePersistentManager::InsertFileIdentifierInternal(FileIdentifier file, FileIdentifier::InsertMode mode)
{
    bool create = (mode & FileIdentifier::kCreate) != 0;
    return InsertPathNameInternal(file.pathName, create);
}

std::string PathNamePersistentManager::PathIDToPathNameInternal(int pathID, bool /*trackNativeLoadedAsset*/) const
{
    Assert(pathID >= 0 && pathID < (int)m_PathNames.size());
    return std::string(m_PathNames[pathID]);
}

int PathNamePersistentManager::InsertPathNameInternal(std::string pathname, bool create)
{
            SET_ALLOC_OWNER(GetMemoryLabel());
    Assert(!(!pathname.empty() && (pathname[0] == '/' || pathname[0] == '\\')));

    std::string lowerCasePathName = ToLower(pathname);

    PathToStreamID::iterator found = m_PathToStreamID.find(lowerCasePathName);
    if (found != m_PathToStreamID.end())
        return found->second;

    if (create)
    {
        m_PathToStreamID.insert(std::make_pair(lowerCasePathName, m_PathNames.size()));
        m_PathNames.push_back(pathname);
        AddStream();
        return m_PathNames.size() - 1;
    }
    else
        return -1;
}
