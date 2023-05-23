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
    MemoryFile(){

    }
    MemoryFile(std::string fName, size_t reservedLength){
        data.resize(reservedLength);
        this->fName = fName;
        printf("Reserving length:%zu for file:%s\n", reservedLength, fName.c_str());
    }

    size_t GetFileLength(){
        return data.size();
    }

    void Truncate(){
        data.clear();
    }

    UInt8* GetDataPtr(){
        return data.data();
    }

    friend class MemoryFileAccessor;
private:
    std::string fName;
    std::vector<UInt8> data;
};

template <class T>
T Min(T n1, T n2){
    if(n1 > n2) return n2;
    return n1;
}

template <class T>
T Max(T n1, T n2){
    if(n1 < n2) return n2;
    return n1;
}

class MemoryFileAccessor{
public:
    MemoryFileAccessor(MemoryFile* memoryFile){
        this->memoryFile = memoryFile;
        this->offset = 0;
    }

    bool Write(const void* buffer, size_t size){
        if(size < 0 || buffer == NULL)
            return false;

        if(offset + size > memoryFile->data.size()){
            memoryFile->data.resize(offset+size);
        }

        UInt8* pDstBuffer = memoryFile->data.data();
        memcpy( (void*)(pDstBuffer + offset), buffer, size);
        offset += size;
        return true;
    }

    size_t Read(size_t position, void* buffer, size_t size){
        size_t fileSize = memoryFile->GetFileLength();

        size_t actualReadSize = Min(size, fileSize - position);

        memcpy(buffer, memoryFile->data.data() + position, actualReadSize);
        offset = position + actualReadSize;

        return actualReadSize;
    }

    size_t Read(void* buffer, size_t size){
        size_t fileSize = memoryFile->GetFileLength();

        size_t actualReadSize = Min(size, fileSize - offset);

        memcpy(buffer, memoryFile->data.data() + offset, actualReadSize);
        offset += actualReadSize;
        return actualReadSize;
    }

    bool Write(size_t pos, const void* buffer, size_t size){
        size_t newSize = pos + size;
        if(newSize > memoryFile->GetFileLength()){
            memoryFile->data.resize(newSize);
        }

        memcpy( (void*)(memoryFile->data.data() + pos), buffer, size);
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

    bool DeleteFile(std::string path);
    void TruncateFile(std::string path);
    bool CreateFile(std::string path, size_t reservedLength = 0);

    bool CloseFile(MemoryFileAccessor* fileAccessor);

    MemoryFileAccessor* OpenFile(std::string path, FilePermission perm);

    UInt8* GetDataPtr(std::string path){
        auto fileItr = m_files.find(path);
        if(fileItr == m_files.end()){
            return NULL;
        }

        return fileItr->second.GetDataPtr();
    }

    bool MoveFileOrDirectory(std::string fromPath, std::string toPath);
private:
    std::map<std::string, MemoryFile> m_files;
};

MemoryFileSystem* GetMemoryFileSystem();


#endif //HUAHUOENGINEV2_MEMORYFILESYSTEM_H
