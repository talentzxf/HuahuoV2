//
// Created by VincentZhang on 6/4/2022.
//

#ifndef HUAHUOENGINEV2_PATHNAMEUTILITY_H
#define HUAHUOENGINEV2_PATHNAMEUTILITY_H

#include <string>


// Path names in Unity always use forward slash as a separator; and convert to other one (if needed)
// only in low-level file functions.
const char kPathNameSeparator = '\\';

// If absolutely required, this defines a platform specific path separator.
#if PLATFORM_WIN
const char kPlatformPathNameSeparator = '\\';
#else
const char kPlatformPathNameSeparator = '/';
#endif

std::string DeleteLastPathNameComponent(std::string pathName);
std::string DeleteLastPathNameComponent(std::string&& pathName) = delete;
//inline std::string DeleteLastPathNameComponent(const char* pathName) { return DeleteLastPathNameComponent(std::string(pathName)); }
inline std::string DeleteLastPathNameComponentManaged(std::string pathName) { return DeleteLastPathNameComponent(pathName); }


std::string DeletePathNameExtension(std::string pathName);
inline std::string DeletePathNameExtension(const char* pathName) { return DeletePathNameExtension(std::string(pathName)); }
inline std::string DeletePathNameExtensionManaged(std::string pathName) { return DeletePathNameExtension(pathName); }
std::string DeletePathNameExtension(std::string&& pathName) = delete;

/// Return extension *without* dot.
std::string GetPathNameExtension(std::string pathName);
inline std::string GetPathNameExtension(const char* pathName) { return GetPathNameExtension(std::string(pathName)); }
inline std::string GetPathNameExtensionManaged(std::string pathName) { return GetPathNameExtension(pathName); }
std::string GetPathNameExtension(std::string&& pathName) = delete;


#endif //HUAHUOENGINEV2_PATHNAMEUTILITY_H
