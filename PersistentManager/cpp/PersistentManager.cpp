//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"

 namespace HuaHuo{
     PersistentManager* PersistentManager::gInstance = new PersistentManager();

    PersistentManager::PersistentManager(){
        this->writeHeader();

        // TODO: Delete this when PersistentManager is destructed.
        this->pByteArray = new ByteArray(pBuffer.data());
    }

    void PersistentManager::writeHeader() {
        HHHeader hhHeader;
        auto const headerPtr = reinterpret_cast<unsigned char*>(&hhHeader);
        this->pBuffer.resize(sizeof(HHHeader));
        std::copy(headerPtr, headerPtr + sizeof(hhHeader), this->pBuffer.begin());
    }

    ByteArray::ByteArray(BYTE* pByteArray){
        this->mpArray = pByteArray;
    }

    BYTE ByteArray::getByte(int idx){
        return this->mpArray[idx];
    }
 }
