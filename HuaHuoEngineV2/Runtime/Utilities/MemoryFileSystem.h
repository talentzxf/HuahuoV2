//
// Created by VincentZhang on 6/6/2022.
//

#ifndef HUAHUOENGINEV2_MEMORYFILESYSTEM_H
#define HUAHUOENGINEV2_MEMORYFILESYSTEM_H

#include <string>
#include "Configuration/IntegerDefinitions.h"
#include "File.h"
#include <vector>
#include <map>
#include <cstring>

class MemoryFileAccessor;

class MemoryFile{
public:
    size_t GetFileLength(){
        return data.size();
    }

    void Truncate(){
        data.clear();
    }

    friend class MemoryFileAccessor;
private:
    std::vector<UInt8> data;
};


class MemoryFileAccessor{
public:
    MemoryFileAccessor(MemoryFile* memoryFile){
        this->memoryFile = memoryFile;
        this->offset = 0;
    }

    bool Write(const void* buffer, size_t size){
        if(size < 0 || buffer == NULL)
            return false;

        size_t newSize = memoryFile->data.size() + size;
        memoryFile->data.resize(newSize);

        memcpy( (void*)memoryFile->data[offset], buffer, size);
        return true;
    }

    bool Write(size_t pos, const void* buffer, size_t size){
        size_t newSize = pos + size;
        if(newSize > memoryFile->GetFileLength()){
            memoryFile->data.resize(newSize);
        }

        memcpy( (void*)memoryFile->data[pos], buffer, size);
        offset = pos + size;
        return true;
    }
private:
    MemoryFile* memoryFile;
    size_t offset;
};

class MemoryFileSystem {
public:
    bool IsMemoryFile(std::string pathName);
    bool IsFileCreated(std::string path);
    size_t GetFileLength(std::string path);

    void DeleteFile(std::string path);
    void TruncateFile(std::string path);
    bool CreateFile(std::string path);

    bool CloseFile(MemoryFileAccessor* fileAccessor);

    MemoryFileAccessor* OpenFile(std::string path, FilePermission perm);
private:
    std::map<std::string, MemoryFile> m_files;
};

MemoryFileSystem* GetMemoryFileSystem();


#endif //HUAHUOENGINEV2_MEMORYFILESYSTEM_H
