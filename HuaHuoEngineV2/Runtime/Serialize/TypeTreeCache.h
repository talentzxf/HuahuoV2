//
// Created by VincentZhang on 2023-04-27.
//

#ifndef HUAHUOENGINEV2_TYPETREECACHE_H
#define HUAHUOENGINEV2_TYPETREECACHE_H
#include "SerializationMetaFlags.h"

class Object;
class ScriptingObjectPtr;
class ScriptingClassPtr;
class TypeTree;

//----------------------------------------------------------------------------------------------------------------------
// What is this:
//  - Public interface to a Global (singleton) cache of TypeTree's that covers both Native and Managed types.
//  - Cached data is aware of transfer instruction flags.
//
// Motivation(s): Type tree's are expensive to generate all the time, so caching them saves time.
//
// Notes:
//  - Cache gets flushed on domain reloads.
//  - Caching uses lockless get when cache already contains data, on insert/resize a write lock is set, but does not
//      affect concurrent get's that fetch data already in cache.
//----------------------------------------------------------------------------------------------------------------------
namespace TypeTreeCache
{
    bool GetTypeTree(const Object *object, TransferInstructionFlags flags, TypeTree& outTypeTree);
    bool GetTypeTree(const ScriptingObjectPtr object, TransferInstructionFlags flags, TypeTree& outTypeTree);
    bool GetTypeTree(const ScriptingClassPtr klass, TransferInstructionFlags flags, TypeTree& outTypeTree);
    bool GetTypeTree(const std::string & className, const std::string & ns, const std::string asmx, TransferInstructionFlags flags, TypeTree& outTypeTree);

    bool RegisterTypeTree(const ScriptingClassPtr klass, TransferInstructionFlags flags, TypeTree& typeTree);
}


#endif //HUAHUOENGINEV2_TYPETREECACHE_H
