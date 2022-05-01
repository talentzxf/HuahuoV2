//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFERBASE_H
#define PERSISTENTMANAGER_TRANSFERBASE_H

#include "Serialize/SerializationMetaFlags.h"

extern const char * kTransferNameIdentifierBase;
class TransferBase {
public:
    void AddMetaFlag(int /*mask*/) {}

    /// Internal function. Should only be called from SerializeTraits
    template<class T>
    void TransferBasicData(T&) {}

    bool IsReading() { return false; }
    bool IsReadingPPtr() { return false; }

    bool IsWriting() { return false; }
    bool IsWritingPPtr() { return false; }

    /// @name Predicates
    /// @{

    /// Get the TransferInstructionFlags.
    /// Commonly used to special case transfer functions for specific operations.
    TransferInstructionFlags GetFlags() const  { return m_Flags; }

    bool NeedsInstanceIDRemapping()          { return m_Flags & kReadWriteFromSerializedFile; }
protected:
    TransferInstructionFlags          m_Flags;
    void*                             m_UserData;
};


#endif //PERSISTENTMANAGER_TRANSFERBASE_H
