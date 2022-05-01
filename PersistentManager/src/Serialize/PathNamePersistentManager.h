//
// Created by VincentZhang on 5/1/2022.
//

#ifndef PERSISTENTMANAGER_PATHNAMEPERSISTENTMANAGER_H
#define PERSISTENTMANAGER_PATHNAMEPERSISTENTMANAGER_H

#include "PersistentManager.h"
#include <map>

class PathNamePersistentManager: public PersistentManager {
    typedef std::map<std::string, SInt32>          PathToStreamID;
    PathToStreamID      m_PathToStreamID; // Contains lower case pathnames
    std::vector<std::string>       m_PathNames;// Contains pathnames as they were given

protected:
    virtual FileIdentifier PathIDToFileIdentifierInternal(int pathID) const;
};

void InitPathNamePersistentManager();

#endif //PERSISTENTMANAGER_PATHNAMEPERSISTENTMANAGER_H
