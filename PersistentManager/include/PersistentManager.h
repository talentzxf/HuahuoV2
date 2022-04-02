//
// Created by VincentZhang on 4/1/2022.
//

#ifndef PERSISTENTMANAGER_PERSISTENTMANAGER_H
#define PERSISTENTMANAGER_PERSISTENTMANAGER_H

#include <cstddef>
#include <vector>
#include "Types.h"

namespace HuaHuo{
    struct HHHeader{
        BYTE magic[4] = "HHH"; // Stands for HuaHuoHeader.
        INT32 version = 0; // Version of this Header
        INT32 objectCount = 0;
        INT64 fileLength = 0;
        BYTE headerEnd[4] = "HHE"; // Stands for HuaHuoEnd.
    };

    class PersistentManager {
    private:
        std::vector<unsigned char> pBuffer = std::vector<unsigned char>();
        PersistentManager();

        void writeHeader();

        static PersistentManager* pInst;
    public:
        static PersistentManager* getInstance();

        inline BYTE* getBuffer(){
            return pBuffer.data();
        }

        inline UINT32 getBufferSize(){
            return pBuffer.size();
        }
    };
}

#endif //PERSISTENTMANAGER_PERSISTENTMANAGER_H
