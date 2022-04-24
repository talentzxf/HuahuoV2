//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFERBASE_H
#define PERSISTENTMANAGER_TRANSFERBASE_H

extern const char * kTransferNameIdentifierBase;
class TransferBase {
public:
    void AddMetaFlag(int /*mask*/) {}

    /// Internal function. Should only be called from SerializeTraits
    template<class T>
    void TransferBasicData(T&) {}
};


#endif //PERSISTENTMANAGER_TRANSFERBASE_H
