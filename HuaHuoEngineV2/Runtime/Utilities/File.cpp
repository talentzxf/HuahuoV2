//
// Created by VincentZhang on 6/4/2022.
//

#include "File.h"
#include "PathNameUtility.h"
#include "Word.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "MemoryFileSystem.h"
#include "GUID.h"

namespace fs = std::filesystem;

std::string PathToAbsolutePath(std::string path)
{
    if(GetMemoryFileSystem()->IsMemoryFile(path))
        return path;
    return std::filesystem::absolute(path).string();
}

bool IsFileCreated(std::string path)
{
    if(GetMemoryFileSystem()->IsMemoryFile(path))
        return GetMemoryFileSystem()->IsFileCreated(path);

    return std::filesystem::exists(path);
}

size_t GetFileLength(std::string path){
    if(GetMemoryFileSystem()->IsMemoryFile(path))
        return GetMemoryFileSystem()->GetFileLength(path);

    return std::filesystem::file_size(path);
}

File::File(){
    isMemoryFile = false;
    m_FileAccessor = NULL;
}

bool File::IsValid() {
    return m_MemFileAccessor != NULL || m_FileAccessor != NULL;
}

bool File::Open(std::string path, FilePermission perm, FileAutoBehavior behavior) {
    if(GetMemoryFileSystem()->IsMemoryFile(path)){
        isMemoryFile = true;
        m_MemFileAccessor = GetMemoryFileSystem()->OpenFile(path, perm);
        return m_MemFileAccessor != NULL;
    }

    const char* permission;
    switch (perm) {
        case kReadPermission:
            permission = "rb";
            break;
        case kWritePermission:
            permission = "w+b";
            break;
        case kReadWritePermission:
            permission = "wb";
            break;
        case kAppendPermission:
            permission = "a";
            break;
    };

    m_FileAccessor = fopen(path.c_str(), permission);

    return m_FileAccessor != 0;
}

bool File::Write(const void *buffer, size_t size) {
    if(this->isMemoryFile){
        return this->m_MemFileAccessor->Write(buffer, size);
    }

    size_t written = fwrite(buffer, size, 1, m_FileAccessor);
    return written > 0;
}

bool File::Write(size_t pos, const void *buffer, size_t size) {
    if(this->isMemoryFile){
        return this->m_MemFileAccessor->Write(pos, buffer, size);
    }

    size_t written = fwrite((const char*)buffer + pos, size, 1, m_FileAccessor);
    return written > 0;
}

size_t File::Read(void* buffer, size_t size, FileReadFlags flags){
    if(this->isMemoryFile){
        return this->m_MemFileAccessor->Read(buffer, size);
    }

    size_t readSize = fread((void *) buffer, 1, size, this->m_FileAccessor);
    return readSize;
}

size_t File::Read(size_t position, void* buffer, size_t size, FileReadFlags flags){
    if(this->isMemoryFile){
        return this->m_MemFileAccessor->Read(position, buffer, size);
    }

    fseek(this->m_FileAccessor, position, SEEK_SET);
    size_t readSize = fread((void *) buffer, 1, size, this->m_FileAccessor);
    return readSize;
}

bool File::Close(){
    if(this->isMemoryFile){
        if(this->m_MemFileAccessor){
            GetMemoryFileSystem()->CloseFile(this->m_MemFileAccessor);
            this->m_MemFileAccessor = NULL;
        }
        return true;
    }
    int closeRes = fclose(m_FileAccessor);
    return closeRes == 0;
}

File::~File() {}

void CreateFile(std::string path){
    if(GetMemoryFileSystem()->IsMemoryFile(path))
        GetMemoryFileSystem()->CreateFile(path);
    else{
        std::fstream fs;
        fs.open(path, std::ios::out);
        fs.close();
    }
}

bool IsPathCreated(std::string path)
{
    // __FAKEABLE_FUNCTION__(IsPathCreated, (path));

    if(GetMemoryFileSystem()->IsMemoryFile(path))
        return true;

    if (path.empty())
        return false;

    std::string absolutePath = PathToAbsolutePath(path);
    return std::filesystem::exists(absolutePath);
}

bool IsDirectoryCreated(std::string path)
{
    if(GetMemoryFileSystem()->IsMemoryFile(path))
        return true;

    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool CreateDirectory(std::string pathName)
{
    if(GetMemoryFileSystem()->IsMemoryFile(pathName))
        return true;

    // __FAKEABLE_FUNCTION__(CreateDirectory, (pathName));

    std::string absolutePath = PathToAbsolutePath(pathName);

    std::filesystem::create_directory(absolutePath);
    return true;
}

#if HUAHUO_EDITOR
std::string GetUniqueTempPath(std::string basePath)
{
    std::string tmpFilePath; (kMemTempAlloc);

    do
    {
        HuaHuoGUID guid; guid.Init();
        tmpFilePath = basePath + GUIDToString(guid);
    }
    while (IsFileCreated(tmpFilePath));

    return tmpFilePath;
}

std::string GetUniqueTempPathInProject()
{
    // __FAKEABLE_FUNCTION__(GetUniqueTempPathInProject, ());
    return GetUniqueTempPath("Temp/UnityTempFile-");
}

std::string GenerateUniquePathSafe(std::string inPath){
    return GenerateUniquePath(inPath);
}

std::string GenerateUniquePath(std::string inPath)
{
    if (!IsDirectoryCreated(DeleteLastPathNameComponent(inPath)))
        return std::string();

    if (!IsPathCreated(inPath))
        return std::string(inPath);

    std::string pathNoExtension = DeletePathNameExtension(inPath);
    std::string extension = GetPathNameExtension(inPath);

    int i = 1;

    if (!pathNoExtension.empty())
    {
        std::string::size_type pos = pathNoExtension.size();
        while (IsDigit(*(pathNoExtension.begin() + pos - 1)))
            pos--;

        if (pos == pathNoExtension.size())
            pathNoExtension = pathNoExtension + " ";
        else
        {
            i = StringToInt(std::string(pathNoExtension.begin() + pos, pathNoExtension.end()));
            pathNoExtension = std::string(pathNoExtension.begin(), pathNoExtension.begin() + pos);
        }
    }
    else
        pathNoExtension = pathNoExtension + " ";

    int timeout = 0;
    while (timeout < 10000)
    {
        std::string uniquePath = pathNoExtension + IntToString(i);
        if (!extension.empty())
            uniquePath = uniquePath + std::string(".") + extension;

        if (!IsFileCreated(uniquePath) && !IsDirectoryCreated(uniquePath))
            return std::string(uniquePath);

        timeout++;
        i++;
    }

    return std::string();
}

bool MoveFileOrDirectory(std::string fromPath, std::string toPath){
    if(GetMemoryFileSystem()->IsMemoryFile(fromPath)){
        return GetMemoryFileSystem()->MoveFileOrDirectory(fromPath, toPath);
    }

    return std::filesystem::copy_file(fromPath, toPath);
}

bool DeleteFile(std::string fName){
    if(GetMemoryFileSystem()->IsMemoryFile(fName))
        return GetMemoryFileSystem()->DeleteFile(fName);

    std::filesystem::remove(fName);
    return true;
}

#if WEB_ENV
emscripten::val createMemFile(std::string fileName, size_t length){
    printf("%s,%d: Creating file:%s, length:%d\n", __FILE__, __LINE__, fileName.c_str(), length);
    bool createFileResult = GetMemoryFileSystem()->CreateFile(fileName, length);

    UInt8* bufferPtr = GetMemoryFileSystem()->GetDataPtr(fileName);

    return emscripten::val(
                emscripten::typed_memory_view(length, bufferPtr)
                );
}

EMSCRIPTEN_BINDINGS(HuaHuoEngineV2) {
    function("createMemFile", &createMemFile);
}
#endif

#endif