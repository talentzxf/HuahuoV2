//
// Created by VincentZhang on 4/1/2022.
//

#ifndef PERSISTENTMANAGER_HUAHUOENGINE_H
#define PERSISTENTMANAGER_HUAHUOENGINE_H

#include <cstddef>
#include <vector>
#include "BaseClasses/BaseTypes.h"
#include "TypeSystem/Object.h"

struct HHHeader {
    UInt8 magic[4] = "HHH"; // Stands for HuaHuoHeader.
    UInt32 version = 0; // Version of this Header
    UInt32 objectCount = 0;
    UInt64 fileLength = 0;
    UInt8 headerEnd[4] = "HHE"; // Stands for HuaHuoEnd.
};

class ByteArray {
private:
    std::vector<unsigned char> pBuffer = std::vector<unsigned char>();
    UInt8 *mpArray;
public:
    ByteArray();

    UInt8 getByte(int idx);

    inline size_t getSize() {
        return pBuffer.size();
    }

    void write(UInt8 *pBuffer, int size);
};

class HuaHuoEngine {
private:
    ByteArray *pByteArray;

    void writeHeader();

    HuaHuoEngine();

    static HuaHuoEngine *gInstance;
public:
    static void InitEngine();

    inline static HuaHuoEngine *getInstance() {
        return gInstance;
    }

    inline ByteArray *getBuffer() {
        return this->pByteArray;
    }

    inline size_t getBufferSize() {
        return pByteArray->getSize();
    }
};


#endif //PERSISTENTMANAGER_HUAHUOENGINE_H
