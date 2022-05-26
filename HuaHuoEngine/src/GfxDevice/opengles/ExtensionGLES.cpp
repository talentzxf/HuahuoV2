//
// Created by VincentZhang on 5/25/2022.
//

#include "ExtensionsGLES.h"

#define DECLARE_EXT_GROUP(name)                                                                 \
typedef fixed_bitset<EnumTraits::StaticTraits<name##Ext>::Count, UInt32> name##ExtensionSet;    \
static name##ExtensionSet name##Extensions;                                                     \
                                                                                                \
bool HasExtension(name##Ext::ActualEnumType ext)                                                \
{                                                                                               \
    return name##Extensions.test(ext);                                                          \
}                                                                                               \
                                                                                                \
static void Initialize##name##Extension(std::string& ext)                                   \
{                                                                                               \
    const char* const* names = EnumTraits::GetNames<name##Ext>();                               \
    for (size_t i = 0; i < EnumTraits::StaticTraits<name##Ext>::Count; ++i)                     \
    {                                                                                           \
        if (ext == names[i] + 1)                                                                \
        {                                                                                       \
            name##Extensions.set(i);                                                            \
            return;                                                                             \
        }                                                                                       \
    }                                                                                           \
}

DECLARE_EXT_GROUP(GL);
DECLARE_EXT_GROUP(WebGL);

#undef DECLARE_EXT_GROUP

bool QueryExtensionSlow(GLExt::ActualEnumType ext)
{
    AssertMsg(gGL != NULL, "ApiGLES has to be initialized prior to querying extensions!");
    return gGL->QueryExtensionSlow(EnumTraits::GetNames<GLExt>()[ext] + 1);
}

std::string GetExtensionsString(const std::vector<std::string>& allExtensions)
{
    if (allExtensions.size() == 0)
        return std::string("");

    std::string ret;
    ret.reserve(allExtensions.size() * 32); // This should be enough to avoid additional allocations

    for (size_t i = 0; i < allExtensions.size(); ++i)
    {
        ret.append(" ");
        ret.append(allExtensions[i]);
    }

    return ret;
}

void InitializeExtensions(const std::vector<std::string>& allExtensions)
{
    GLExtensions.reset_all();
    WebGLExtensions.reset_all();

    for (size_t i = 0; i < allExtensions.size(); ++i)
    {
        if (allExtensions[i].starts_with("GL_") || allExtensions[i].starts_with("GLX_"))
            InitializeGLExtension(allExtensions[i]);
        else
            InitializeWebGLExtension(allExtensions[i]);
    }
}
