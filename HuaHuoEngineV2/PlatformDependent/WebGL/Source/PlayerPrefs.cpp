#include "UnityPrefix.h"
#include "Runtime/Utilities/PlayerPrefs.h"
#include "Runtime/Utilities/Word.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Utilities/File.h"
#include "Runtime/Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Runtime/Misc/SystemInfo.h"
#include "PlatformDependent/WebGL/Source/JSBridge.h"
#include <string.h>
#include <stdio.h>
#include <string>


#define FILE_MAJOR 1
#define FILE_MINOR 0

const char* kFileMagic = "UnityPrf";

struct PrefHeader
{
    char magic[8]; // UnityPrf
    UInt32 version;  // 1.0
    UInt32 maxSize; // Maximum allowed size for file
    UInt16 VerMajor() const { return version >> 16; }
    UInt16 VerMinor() const { return version & 0xffff; }
};

enum PrefType
{
    kPTError    = 255,
    kPTInt      = 254,
    kPTFloat    = 253,
    // We leave room in [129; 252] for more types in the future
    // Everything below 128 is string.
    kPTString   = 128, // when seen in file, the size of the string is encoded as 32 bits after this flag
    kPTShortString = 127 // any value n, 0 <= n < 128 is the length and the value follows directly afterwards
};

const UInt32 kDefaultMaxFileSize = 1024 * 1024; // 1 megabyte

struct PrefValue
{
    PrefType type;
    union
    {
        int intVal;
        float floatVal;
    };
    core::string stringVal;

    PrefValue(int value) : type(kPTInt), intVal(value) {}
    PrefValue(float value) : type(kPTFloat), floatVal(value) {}
    PrefValue(const core::string& value) : type(kPTString), stringVal(value) {}
    PrefValue() : type(kPTError) {}

    int GetInt(int def) { if (type == kPTInt) return intVal; return def; }
    float GetFloat(float def) { if (type == kPTFloat) return floatVal; return def; }
    core::string GetString(const core::string& def) { if (type == kPTString) return stringVal; return def; }
};

static UInt32 gCurrentSize = 0;
static UInt32 gMaxFileSize = 0;
static bool gPrefsDirty = false;

typedef std::map<core::string, PrefValue> PlayerPrefsMap;

static PlayerPrefsMap* s_PlayerPrefs = NULL;
static core::string gPlayerURL;


inline PlayerPrefsMap& GetPlayerPrefsMap()
{
    Assert(s_PlayerPrefs != NULL);
    return *s_PlayerPrefs;
}

static core::string EncodeValue(const PrefValue& in)
{
    switch (in.type)
    {
        case kPTInt:
        case kPTFloat:
        {
            char result[sizeof(char) + sizeof(SInt32)];
            result[0] = (unsigned char)in.type;
            memcpy(result + 1, &(in.intVal), sizeof(SInt32));
            return core::string(result, sizeof(char) + sizeof(SInt32));
        }
        default:
        {
            int size = in.stringVal.size();
            if (size <= kPTShortString)
            {       // a minor size optimisation for small strings; encode the size as part of the type flag if the string is <= 127 chars
                core::string c = " " + in.stringVal;
                c[0] = (unsigned char)size;
                return c;
            }
            else
            {
                char result[sizeof(char) + sizeof(SInt32)];
                result[0] = (unsigned char)kPTString;
                memcpy(result + 1, &size, sizeof(UInt32));
                return core::string(result, sizeof(char) + sizeof(SInt32)) + in.stringVal;
            }
        }
    }
}

#define BOUNDS_CHECK(b, e) if( (b) > (e) ) return PrefValue()

static PrefValue DecodeValue(const char** buffer, const char* end)
{
    UInt32 type_or_size = *reinterpret_cast<const unsigned char*>((*buffer)++);
    const char* data_start = *buffer;
    BOUNDS_CHECK(*buffer, end);
    if (type_or_size > kPTString && type_or_size < kPTFloat)
        return PrefValue();
    switch (type_or_size)
    {
        case kPTInt:
        {
            *buffer += sizeof(SInt32);
            BOUNDS_CHECK(*buffer, end);
            SInt32 tmp;
            // Use memcpy here instead of asignment for alignment in WebGL
            memcpy(&tmp, data_start, sizeof(tmp));
            return PrefValue((int)tmp);
        }
        case kPTFloat:
        {
            *buffer += sizeof(float);
            BOUNDS_CHECK(*buffer, end);
            float tmp;
            // Use memcpy here instead of asignment for alignment in WebGL
            memcpy(&tmp, data_start, sizeof(tmp));
            return PrefValue(tmp);
        }
        case kPTString:
            // Use memcpy here instead of asignment for alignment in WebGL
            memcpy(&type_or_size, data_start, sizeof(type_or_size));

            *buffer += sizeof(UInt32);
            BOUNDS_CHECK(*buffer, end);
            data_start = *buffer;
        // falls through to the default case
        default:
            *buffer += type_or_size;
            BOUNDS_CHECK(*buffer, end);
            return PrefValue(core::string(data_start, type_or_size));
    }
}

#undef BOUNDS_CHECK

static UInt32 EncodedSize(const core::string& value)
{
    UInt32 tmp = value.size();
    return sizeof(char) + (tmp <= kPTShortString ? 0 : sizeof(UInt32)) + tmp;
}

static UInt32 EncodedSize(const PrefValue& value)
{
    switch (value.type)
    {
        case kPTFloat:
        case kPTInt:
            return sizeof(char) + sizeof(UInt32);
        default:
            return EncodedSize(value.stringVal);
    }
}

static UInt32 CalculateNewSize(const core::string& name, const PrefValue& value)
{
    if (GetPlayerPrefsMap().count(name))
        return gCurrentSize - EncodedSize(GetPlayerPrefsMap()[name]) + EncodedSize(value);
    else
        return gCurrentSize + EncodedSize(name) + EncodedSize(value);
}

// Returns the full path where the preference file is stored. If createParent is true, it will make sure that the parent directory exists before returning.
static core::string GetPrefFilePath(bool createParent)
{
    return AppendPathName(systeminfo::GetPersistentDataPath(), "PlayerPrefs");
}

static void ReadPrefsData(const char* data, size_t size)
{
    gPrefsDirty = true; // Ensure the config file will be overwritten if it's invalid
    const char * start = data;
    const char * end = start + size;
    if (size < 8 + sizeof(UInt32) + sizeof(UInt32))
        return;
    PrefHeader header;
    memcpy(header.magic, start, 8);

    start += 8;
    header.version = *reinterpret_cast<const UInt32*>(start);
    start += sizeof(UInt32);
    header.maxSize = *reinterpret_cast<const UInt32*>(start);
    start += sizeof(UInt32);

    if (memcmp(kFileMagic, header.magic, 8) != 0)
        return;
    if (header.VerMajor() != FILE_MAJOR || header.VerMinor() > FILE_MINOR)
        return;

    gMaxFileSize = header.maxSize;

    while (start < end)
    {
        PrefValue name = DecodeValue(&start, end);
        if (name.type != kPTString || start >= end)
            break;
        PrefValue value = DecodeValue(&start, end);
        if (value.type == kPTError)
            break;

        Assert(!GetPlayerPrefsMap().count(name.stringVal));
        GetPlayerPrefsMap()[name.stringVal] = value;
        gCurrentSize += EncodedSize(name) + EncodedSize(value);
    }

    Assert(gCurrentSize == size);
    Assert(gCurrentSize <= gMaxFileSize);
    gPrefsDirty = false; // If we reach this point, the config file is valid and does not need to be overwritten
}

static void InitializeIfNeeded()
{
    if (gCurrentSize != 0)
        return;

    core::string filePath = GetPrefFilePath(false);

    GetPlayerPrefsMap().clear();
    gPrefsDirty = false;
    gCurrentSize = 8 + sizeof(UInt32) + sizeof(UInt32); // The size of the file header
    gMaxFileSize = kDefaultMaxFileSize;

    InputString contents;
    if (ReadStringFromFile(&contents, filePath))
        ReadPrefsData(contents.c_str(), contents.size());
}

static bool SetPrefValue(const core::string& name, const PrefValue& value)
{
    InitializeIfNeeded();
    UInt32 newSize = CalculateNewSize(name, value);
    if (newSize > gMaxFileSize)
        return false;
    gPrefsDirty = true;
    gCurrentSize = newSize;
    GetPlayerPrefsMap()[name] = value;

    // Since WebGL will not currently send us any shutdown notficiation when closing the player
    // always prefs immediately.
    PlayerPrefs::Sync();
    return true;
}

bool PlayerPrefs::SetInt(const core::string& name, int value)
{
    return SetPrefValue(name, PrefValue(value));
}

bool PlayerPrefs::SetFloat(const core::string& name, float value)
{
    return SetPrefValue(name, PrefValue(value));
}

bool PlayerPrefs::SetString(const core::string& name, const core::string& value)
{
    return SetPrefValue(name, PrefValue(value));
}

int PlayerPrefs::GetInt(const core::string& name, int def)
{
    InitializeIfNeeded();
    if (GetPlayerPrefsMap().count(name))
        return GetPlayerPrefsMap()[name].GetInt(def);
    else
        return def;
}

float PlayerPrefs::GetFloat(const core::string& name, float def)
{
    InitializeIfNeeded();
    if (GetPlayerPrefsMap().count(name))
        return GetPlayerPrefsMap()[name].GetFloat(def);
    else
        return def;
}

core::string PlayerPrefs::GetString(const core::string& name, const core::string& def)
{
    InitializeIfNeeded();
    if (GetPlayerPrefsMap().count(name))
        return GetPlayerPrefsMap()[name].GetString(def);
    else
        return def;
}

bool PlayerPrefs::HasKey(const core::string& name)
{
    InitializeIfNeeded();
    return GetPlayerPrefsMap().count(name) > 0;
}

void PlayerPrefs::DeleteKey(const core::string& name)
{
    InitializeIfNeeded();
    std::map<core::string, PrefValue>::iterator found = GetPlayerPrefsMap().find(name);
    if (found != GetPlayerPrefsMap().end())
    {
        gPrefsDirty = true;
        gCurrentSize -= (EncodedSize(found->second) + EncodedSize(name));
        GetPlayerPrefsMap().erase(found);
        Assert(gCurrentSize <= gMaxFileSize);
    }

    // Since WebGL will not currently send us any shutdown notficiation when closing the player
    // always prefs immediately.
    PlayerPrefs::Sync();
}

void PlayerPrefs::DeleteAll()
{
    InitializeIfNeeded();
    gPrefsDirty = true;
    gCurrentSize = 8 + sizeof(UInt32) + sizeof(UInt32); // The size of the file header;
    GetPlayerPrefsMap().clear();
    Assert(gCurrentSize <= gMaxFileSize);

    // Since WebGL will not currently send us any shutdown notficiation when closing the player
    // always prefs immediately.
    PlayerPrefs::Sync();
}

void PlayerPrefs::Sync()
{
    if (!gPrefsDirty)
        return;

    core::string filePath = GetPrefFilePath(true);
    if (filePath == core::string())
        return;

    File fh;
    if (!fh.Open(filePath, kWritePermission))
    {
        ErrorString("Warning: PlayerPrefs will not be saved");
        return;
    }
#define write(_data, _size) fh.Write(_data,_size)

    PrefHeader header;
    memcpy(header.magic, kFileMagic, 8);
    header.version = FILE_MAJOR << 16 | FILE_MINOR;
    header.maxSize = gMaxFileSize;

    // Write out each member separately to avoid possible memory alignment differences.
    write(&header.magic, sizeof(header.magic));
    write(&header.version, sizeof(header.version));
    write(&header.maxSize, sizeof(header.maxSize));

    for (std::map<core::string, PrefValue>::iterator i = GetPlayerPrefsMap().begin(); i != GetPlayerPrefsMap().end(); ++i)
    {
        core::string encoded = EncodeValue(PrefValue(i->first));
        write(encoded.c_str(), encoded.size());
        encoded = EncodeValue(i->second);
        write(encoded.c_str(), encoded.size());
    }
    fh.Close();
    Assert(GetFileLength(filePath).Cast<UInt64>() == gCurrentSize);

    JS_FileSystem_Sync();

    gPrefsDirty = false;
}

namespace
{
    void StaticInitialize(void*)
    {
        s_PlayerPrefs = new PlayerPrefsMap();
    }

    // Since PlayerPrefs rely on static variables here, it must be destroyed
    // before the memory manager, otherwise we will get crashes
    void StaticDestroy(void*)
    {
        GetPlayerPrefsMap().clear();
        delete s_PlayerPrefs;
        s_PlayerPrefs = NULL;

        gPlayerURL.clear();
    }

    static RegisterRuntimeInitializeAndCleanup s_PlayerPrefsCallbacks(StaticInitialize, StaticDestroy);
}
