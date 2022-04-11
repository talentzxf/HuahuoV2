//
// Created by VincentZhang on 4/1/2022.
//

#ifndef PERSISTENTMANAGER_PERSISTENTMANAGER_H
#define PERSISTENTMANAGER_PERSISTENTMANAGER_H

#include <cstddef>
#include <vector>
#include "BaseClasses/BaseTypes.h"

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

class PersistentManager {
private:
    ByteArray *pByteArray;

    void writeHeader();

    PersistentManager();

    static PersistentManager *gInstance;
public:
    static void InitEngine();

    inline static PersistentManager *getInstance() {
        return gInstance;
    }

    inline ByteArray *getBuffer() {
        return this->pByteArray;
    }

    inline size_t getBufferSize() {
        return pByteArray->getSize();
    }
};


#endif //PERSISTENTMANAGER_PERSISTENTMANAGER_H
