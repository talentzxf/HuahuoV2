//
// Created by VincentZhang on 6/4/2022.
//

#include "File.h"
#include "PathNameUtility.h"
#include "Word.h"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

std::string PathToAbsolutePath(std::string path)
{
    return std::filesystem::absolute(path).string();
}

bool IsFileCreated(std::string path)
{
    return std::filesystem::exists(path);
}

size_t GetFileLength(std::string path){
    return std::filesystem::file_size(path);
}

File::File(){
    m_FileAccessor = NULL;
}

bool File::Open(std::string path, FilePermission perm, FileAutoBehavior behavior) {
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
    size_t written = fwrite(buffer, size, 1, m_FileAccessor);
    return written > 0;
}

bool File::Write(size_t pos, const void *buffer, size_t size) {
    size_t written = fwrite((const char*)buffer + pos, size, 1, m_FileAccessor);
    return written > 0;
}

bool File::Close(){
    int closeRes = fclose(m_FileAccessor);
    return closeRes == 0;
}

File::~File() {}

bool IsPathCreated(std::string path)
{
    // __FAKEABLE_FUNCTION__(IsPathCreated, (path));

    if (path.empty())
        return false;

    std::string absolutePath = PathToAbsolutePath(path);
    return std::filesystem::exists(absolutePath);
}

bool IsDirectoryCreated(std::string path)
{
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool CreateDirectory(std::string pathName)
{
    // __FAKEABLE_FUNCTION__(CreateDirectory, (pathName));

    std::string absolutePath = PathToAbsolutePath(pathName);

    std::filesystem::create_directory(absolutePath);
    return true;
}

#if HUAHUO_EDITOR
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
#endif