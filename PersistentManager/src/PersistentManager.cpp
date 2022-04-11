//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"

void PersistentManager::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();
}

PersistentManager *PersistentManager::gInstance = new PersistentManager();

PersistentManager::PersistentManager() {
    // TODO: Delete this when PersistentManager is destructed.
    this->pByteArray = new ByteArray();

    this->writeHeader();
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

