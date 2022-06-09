//
// Created by VincentZhang on 5/1/2022.
//

#include "PathNamePersistentManager.h"
#include "SerializedFile.h"
#include "Utilities/Word.h"

void InitPathNamePersistentManager()
{
    printf("Creating persistentManager\n");
    SetPersistentManager(HUAHUO_NEW_AS_ROOT(PathNamePersistentManager , kMemManager, "Managers", "PathNameManager") ());
    printf("PersistentManager created\n");
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

    printf("%s,%d\n", __FILE__, __LINE__);
    std::string lowerCasePathName = ToLower(pathname);
    printf("%s,%d\n", __FILE__, __LINE__);
    PathToStreamID::iterator found = m_PathToStreamID.find(lowerCasePathName);
    if (found != m_PathToStreamID.end())
        return found->second;

    printf("%s,%d\n", __FILE__, __LINE__);
    if (create)
    {
        printf("%s,%d\n", __FILE__, __LINE__);
        m_PathToStreamID.insert(std::make_pair(lowerCasePathName, m_PathNames.size()));
        m_PathNames.push_back(pathname);
        AddStream();
        printf("%s,%d\n", __FILE__, __LINE__);
        return m_PathNames.size() - 1;
    }
    else
        return -1;
}
