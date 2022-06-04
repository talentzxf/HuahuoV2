//
// Created by VincentZhang on 6/4/2022.
//

#include <string>
#include "PathNameUtility.h"

std::string GetPathNameExtension(std::string pathName)
{
    const size_t length = pathName.size();
    for (size_t i = 0; i < length; i++)
    {
        if (pathName[length - i - 1] == kPathNameSeparator)
            return std::string();
        if (pathName[length - i - 1] == '.')
            return pathName.substr(length - i);
    }

    return std::string();
}

std::string DeletePathNameExtension(std::string pathName)
{
    std::string::size_type slash = pathName.rfind(kPathNameSeparator);
    std::string::size_type dot = pathName.rfind('.');

    if (dot != std::string::npos)
    {
        if (slash == std::string::npos || dot > slash)
            return pathName.substr(0, dot);
    }

    return pathName;
}

static std::string DeleteLastPathNameComponentImpl(std::string pathName, const char* separators)
{
    // strip the trailing slash (get the position of the last non-slash)
    std::string::size_type pos = pathName.find_last_not_of(separators);
    if (pos == std::string::npos)
        return std::string();

    // strip one component (get the position of the last slash ignoring trailing slashes)
    pos = pathName.find_last_of(separators, pos);
    if (pos == std::string::npos)
        return std::string();

    // strip possibly multiple slashes separating the last component from the one before
    pos = pathName.find_last_not_of(separators, pos);
    if (pos == std::string::npos)
        return "/"; // don't strip the first slash

    return pathName.substr(0, pos + 1);
}

static const char kDeleteLastPathNameComponentSeparators[] = { kPathNameSeparator, '\0' };
static const char kDeleteLastPathNameComponentPlatformSeparators[] = { kPlatformPathNameSeparator, '\0' };
static const char kDeleteLastPathNameComponentAllSeparators[] = { kPlatformPathNameSeparator, kPathNameSeparator, '\0' };

std::string DeleteLastPathNameComponent(std::string pathName)
{
    return DeleteLastPathNameComponentImpl(pathName, kDeleteLastPathNameComponentSeparators);
}

std::string DeleteLastPathNameComponents(std::string pathName, unsigned int count)
{
    std::string result = pathName;
    for (unsigned int i = 0; i < count; i++)
        result = DeleteLastPathNameComponentImpl(result, kDeleteLastPathNameComponentSeparators);
    return result;
}


#include "PathNameUtility.h"
