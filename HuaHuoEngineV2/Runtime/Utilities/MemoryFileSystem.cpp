//
// Created by VincentZhang on 6/6/2022.
//

#include "MemoryFileSystem.h"
#include "Logging/LogAssert.h"
#include <string>

MemoryFileSystem gMemoryFileSystem;

MemoryFileSystem* GetMemoryFileSystem(){
    return &gMemoryFileSystem;
}

bool MemoryFileSystem::IsMemoryFile(std::string path) {
    return path.starts_with("mem://");
}

bool MemoryFileSystem::IsFileCreated(std::string path) {
    return m_files.find(path) != m_files.end();
}

bool MemoryFileSystem::MoveFileOrDirectory(std::string fromPath, std::string toPath){
    auto fileItr = m_files.find(fromPath);
    if(fileItr == m_files.end())
        return false;
    m_files[toPath] = std::move(fileItr->second);
    m_files.erase(fromPath);
    return true;
}

size_t MemoryFileSystem::GetFileLength(std::string path){
    auto fileItr = m_files.find(path);
    if(fileItr == m_files.end()){
        LogStringMsg("Can't find file: %s", path.c_str());
        return -1;
    }

    return fileItr->second.GetFileLength();
}

bool MemoryFileSystem::DeleteFile(std::string path) {
    if(m_files.contains(path)){
        m_files.erase(path);
    }
    return true;
}

void MemoryFileSystem::TruncateFile(std::string path) {
    m_files.find(path)->second.Truncate();
}

bool MemoryFileSystem::CreateFile(std::string path, size_t reservedLength) {
    printf("%s,%d: Creating file:%s, length:%d\n", __FILE__, __LINE__, path.c_str(), reservedLength);
    if(this->IsFileCreated(path)){
        printf("%s,%d: File already exists!:%s, length:%d\n", __FILE__, __LINE__, path.c_str(), reservedLength);
        LogStringMsg("File:%s already exists!", path.c_str());
        return false;
    }
    printf("%s,%d: Creating file:%s, length:%d\n", __FILE__, __LINE__, path.c_str(), reservedLength);
    m_files.insert(std::pair<std::string, MemoryFile>(path, MemoryFile(path, reservedLength)));
    printf("%s,%d: Creating file:%s, length:%d\n", __FILE__, __LINE__, path.c_str(), reservedLength);
    return true;
}

MemoryFileAccessor* MemoryFileSystem::OpenFile(std::string path, FilePermission perm) {
    if(perm == FilePermission::kWritePermission){
        if(!this->IsFileCreated(path)){
            this->CreateFile(path);
        }
    }

    MemoryFile* memoryFile = &this->m_files.find(path)->second;

    return new MemoryFileAccessor(memoryFile);
}

bool MemoryFileSystem::CloseFile(MemoryFileAccessor* fileAccessor){
    delete fileAccessor;
    fileAccessor = NULL;
    return true;
}