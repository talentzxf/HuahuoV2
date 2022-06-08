//
// Created by VincentZhang on 6/7/2022.
//

#ifndef HUAHUOENGINEV2_WRITEDATA_H
#define HUAHUOENGINEV2_WRITEDATA_H
#include <cstdlib>
#include <vector>
#include "TypeSystem/Object.h"
#include <map>

struct WriteLocation
{
    size_t offset;
    size_t size;
};

struct WriteInformation
{
    size_t headerOffset;
    std::vector<WriteLocation> locations;
};

struct WriteData
{
    LocalIdentifierInFileType localIdentifierInFile;

    // We usually use instanceID to identify objects we are about to write.
    // If objectPtr is defined it will be used.

    // InstanceID is a usually good choice since it is more robust (deleting objects during delete etc)
    // But when creating / writing objects from another thread we need to be able to write when the object has not been added to the IDToPointer map
    InstanceID              instanceID;
    Object*                 objectPtr;

    WriteData() : localIdentifierInFile(0), instanceID(InstanceID_None), objectPtr(NULL) {}

    WriteData(LocalIdentifierInFileType local, InstanceID instance)
            : localIdentifierInFile(local), instanceID(instance), objectPtr(NULL)
    {}

    friend bool operator<(const WriteData& lhs, const WriteData& rhs)
    {
//        if (lhs.buildGroup != rhs.buildGroup)
//            return lhs.buildGroup < rhs.buildGroup;
//        else
        {
            if (lhs.localIdentifierInFile == rhs.localIdentifierInFile)
                return lhs.instanceID < rhs.instanceID;
            else
                return lhs.localIdentifierInFile < rhs.localIdentifierInFile;
        }
    }
};

typedef std::vector<WriteData> WriteDataArray;
typedef std::map<InstanceID, const WriteData*> WriteDataArrayIndex;
typedef std::pair<InstanceID, const WriteData*> WriteDataArrayIndexPair;

inline void MakeWriteDataArrayIndex(const WriteDataArray &writeDataArray, WriteDataArrayIndex &writeDataArrayIndex)
{
    for (WriteDataArray::const_iterator itr = writeDataArray.begin(); itr != writeDataArray.end(); itr++){
        const WriteData* pData = &(*itr);
        writeDataArrayIndex.insert(WriteDataArrayIndexPair(itr->instanceID, pData));
    }
}
#endif //HUAHUOENGINEV2_WRITEDATA_H
