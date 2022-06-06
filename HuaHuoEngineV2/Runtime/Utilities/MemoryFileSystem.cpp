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

size_t MemoryFileSystem::GetFileLength(std::string path){
    auto fileItr = m_files.find(path);
    if(fileItr == m_files.end()){
        LogStringMsg("Can't find file: %s", path.c_str());
        return -1;
    }

    return fileItr->second.GetFileLength();
}

void MemoryFileSystem::DeleteFile(std::string path) {
    if(m_files.contains(path)){
        m_files.erase(path);
    }
}

void MemoryFileSystem::TruncateFile(std::string path) {
    m_files.find(path)->second.Truncate();
}

bool MemoryFileSystem::CreateFile(std::string path) {
    if(this->IsFileCreated(path)){
        LogStringMsg("File:%s already exists!", path.c_str());
        return false;
    }

    m_files.insert(std::pair<std::string, MemoryFile>(path, MemoryFile()));
    return true;
}

MemoryFileAccessor* MemoryFileSystem::OpenFile(std::string path, FilePermission perm) {
    if(perm == FilePermission::kWritePermission){
        if(this->IsFileCreated(path)){
            this->TruncateFile(path);
        } else {
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