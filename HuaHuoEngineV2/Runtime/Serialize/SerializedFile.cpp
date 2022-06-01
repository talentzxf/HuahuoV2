//
// Created by VincentZhang on 5/1/2022.
//

#include "SerializedFile.h"

void SerializedFile::AddExternalRef(const FileIdentifier& pathName)
{
    // Dont' check for pathname here - it can be empty, if we are getting a GUID from
    // a text serialized file, and the file belonging to the GUID is missing. In that
    // case we just keep the GUID.
#if SUPPORT_SERIALIZE_WRITE
    Assert(m_CachedWriter != NULL);
#endif
    m_Externals.push_back(pathName);
    m_Externals.back().CheckValidity();
}