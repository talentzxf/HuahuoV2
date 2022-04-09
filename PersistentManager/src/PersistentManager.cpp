//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"

PersistentManager *PersistentManager::gInstance = new PersistentManager();

PersistentManager::PersistentManager() {
    this->writeHeader();

    // TODO: Delete this when PersistentManager is destructed.
    this->pByteArray = new ByteArray();
}

void PersistentManager::writeHeader() {
    HHHeader hhHeader;
    auto const headerPtr = reinterpret_cast<UInt8 *>(&hhHeader);
    this->pByteArray->write(headerPtr, sizeof(HHHeader));

}

ByteArray::ByteArray() {
    this->mpArray = pBuffer.data();
}

UInt8 ByteArray::getByte(int idx) {
    return this->mpArray[idx];
}

void ByteArray::write(UInt8 *pBuffer, int size) {
    int curPos = this->pBuffer.size();
    this->pBuffer.resize(sizeof(HHHeader));
    std::copy(pBuffer, pBuffer + size, this->pBuffer.begin() + curPos);
}

