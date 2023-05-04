#include "MdFourGenerator.h"
#include <string.h>
#include "Memory/MemoryMacros.h"
#include "openssl/md4.h"

//#define DEBUG_MD4_GENERATOR
#ifdef DEBUG_MD4_GENERATOR
void DebugBytes(const char* bytes, int len)
{
    printf_console("%d bytes {", len);
    for (int i = 0; i < len && i < 20; i++)
        printf_console("%02hhx", bytes[i]);
    if (len > 20)
        printf_console("...");
    printf_console("}\n");
}

#define DebugFormat printf_console
#else
void DebugBytes(const char* bytes, int len) {}
void DebugFormat(const char* log, ...) {}
#endif

MdFourGenerator::MdFourGenerator()
{
    m_Acc = HUAHUO_NEW(MD4_CTX, kMemTempAlloc);
    MD4_Init(m_Acc);
    m_Done = false;
}

MdFourGenerator::~MdFourGenerator()
{
    HUAHUO_DELETE(m_Acc, kMemTempAlloc);
}

void MdFourGenerator::Feed(const void* bytes, UInt64 length)
{
    DebugBytes((const char*)bytes, length);
    MD4_Update(m_Acc, bytes, length);
}

void MdFourGenerator::Feed(SInt32 data)
{
    DebugFormat("Feeding int %08x to hash: ", data);
    Feed(reinterpret_cast<char *>(&data) , sizeof(int));
}

void MdFourGenerator::Feed(UInt32 data)
{
    DebugFormat("Feeding int %08x to hash: ", data);
    Feed(reinterpret_cast<char *>(&data) , sizeof(UInt32));
}

void MdFourGenerator::Feed(ColorRGBA32 data)
{
    DebugFormat("Feeding ColorRGBA32 #%02hhx%02hhx%02hhx%02hhx to hash: ", data.r, data.g, data.b, data.a);
    Feed(reinterpret_cast<char *>(&data) , sizeof(ColorRGBA32));
}

void MdFourGenerator::Feed(float data)
{
    DebugFormat("Feeding float %f to hash: ", data);
    if (std::numeric_limits<float>::has_infinity && data == std::numeric_limits<float>::infinity())
    {
        Feed("+inf" , 4);
    }
    else if (std::numeric_limits<float>::has_infinity && data == -std::numeric_limits<float>::infinity())
    {
        Feed("-inf" , 4);
    }
    else if (std::numeric_limits<float>::has_quiet_NaN && data == std::numeric_limits<float>::quiet_NaN())
    {
        Feed("qNaN" , 4);
    }
    else if (std::numeric_limits<float>::has_signaling_NaN && data == std::numeric_limits<float>::signaling_NaN())
    {
        Feed("sNaN" , 4);
    }
    else
    {
        SInt64 quantisized = (double)data * 1000.0;
        Feed(reinterpret_cast<char *>(&quantisized) , sizeof(SInt64));
    }
}

void MdFourGenerator::Feed(double data)
{
    DebugFormat("Feeding float %f to hash: ", data);
    if (std::numeric_limits<double>::has_infinity && data == std::numeric_limits<double>::infinity())
    {
        Feed("+inf" , 4);
    }
    else if (std::numeric_limits<double>::has_infinity && data == -std::numeric_limits<double>::infinity())
    {
        Feed("-inf" , 4);
    }
    else if (std::numeric_limits<double>::has_quiet_NaN && data == std::numeric_limits<double>::quiet_NaN())
    {
        Feed("qNaN" , 4);
    }
    else if (std::numeric_limits<double>::has_signaling_NaN && data == std::numeric_limits<double>::signaling_NaN())
    {
        Feed("sNaN" , 4);
    }
    else
    {
        SInt64 quantisized = (double)data * 1000.0;
        Feed(reinterpret_cast<char *>(&quantisized) , sizeof(SInt64));
    }
}

void MdFourGenerator::Feed(UInt64 data)
{
    DebugFormat("Feeding uint64 %016llx to hash: ", data);
    Feed(reinterpret_cast<char *>(&data) , sizeof(UInt64));
}

void MdFourGenerator::Feed(SInt64 data)
{
    DebugFormat("Feeding uint64 %016llx to hash: ", data);
    Feed(reinterpret_cast<char *>(&data) , sizeof(SInt64));
}

void MdFourGenerator::Feed(const std::string& data)
{
    DebugFormat("Feeding string \"%s\" to hash: ", data.c_str());
    Feed(data.c_str(), data.size());
}

void MdFourGenerator::Feed(char const* string)
{
    DebugFormat("Feeding string \"%s\" to hash: ", string);
    Feed(string, strlen(string));
}

void MdFourGenerator::Feed(Hash128 hash)
{
    DebugFormat("Feeding hash \"%s\" to hash: ", Hash128ToString(hash).c_str());
    Feed(hash.hashData.bytes, sizeof(hash));
}
//
//UInt64 MdFourGenerator::FeedFromFile(const core::string& pathName)
//{
//    DebugFormat("Feeding file contents of \"%s\" into hash: ", pathName.c_str());
//    File fh;
//    UInt64 processedBytes = 0;
//    if (!fh.Open(pathName, kReadPermission, kSilentReturnOnOpenFail))
//    {
//        return ~0ULL;
//    }
//
//    const int block_size = 16 * 1024;
//    dynamic_array<UInt8> buffer(block_size, kMemTempAlloc);
//    size_t bytes_read;
//    while ((bytes_read = fh.Read(buffer.data(), block_size)) > 0)
//    {
//        processedBytes += bytes_read;
//        Feed(buffer.data(), bytes_read);
//    }
//
//    fh.Close();
//
//    return processedBytes;
//}

Hash128 MdFourGenerator::Finish()
{
    Hash128 result;
    if (!m_Done)
    {
        m_Done = true;
        MD4_Final(result.hashData.bytes, m_Acc);
    }

    return result;
}

//Hash128 MdFourFile(const std::string& filename)
//{
//    MdFourGenerator generator;
//    generator.FeedFromFile(filename);
//    return generator.Finish();
//}
