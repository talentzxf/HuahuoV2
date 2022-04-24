//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_SERIALIZEUTILITY_H
#define PERSISTENTMANAGER_SERIALIZEUTILITY_H

#define TRANSFER_WITH_FLAGS(x, metaFlag) transfer.Transfer (x, #x, metaFlag)
#define TRANSFER_WITH_NAME(x, name) transfer.Transfer (x, name)
#define TRANSFER(x) TRANSFER_WITH_NAME(x, #x)

#endif //PERSISTENTMANAGER_SERIALIZEUTILITY_H
