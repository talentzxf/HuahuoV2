//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"

namespace HuaHuo{
    PersistentManager* PersistentManager::pInst = NULL;

    PersistentManager::PersistentManager(){
        this->writeHeader();
    }

    void PersistentManager::writeHeader() {
        HHHeader hhHeader;
        auto const headerPtr = reinterpret_cast<unsigned char*>(&hhHeader);
        this->pBuffer.resize(sizeof(HHHeader));
        std::copy(headerPtr, headerPtr + sizeof(hhHeader), this->pBuffer.begin());
    }

    PersistentManager* PersistentManager::getInstance(){
        if(PersistentManager::pInst == NULL){
            PersistentManager::pInst = new PersistentManager();
        }

        return PersistentManager::pInst;
    }
}
