//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"
#include "Serialize/PathNamePersistentManager.h"

void HuaHuoEngine::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();

    InitPathNamePersistentManager();
}

HuaHuoEngine *HuaHuoEngine::gInstance = new HuaHuoEngine();

HuaHuoEngine::HuaHuoEngine() {
    // TODO: Delete this when HuaHuoEngine is destructed.
    this->pByteArray = new ByteArray();

    this->writeHeader();
}

void HuaHuoEngine::writeHeader() {
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
