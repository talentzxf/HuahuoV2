//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_OBJECT_H
#define PERSISTENTMANAGER_OBJECT_H

#include "RTTI.h"
#include "ObjectDefines.h"
#include "Type.h"

class Object {
public:
    template<typename T>
    static T* Produce(InstanceID instanceID = InstanceID_None)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), TypeOf<T>(), instanceID));
    }

    template<typename T>
    static T* Produce(const HuaHuo::Type* type, InstanceID instanceID = InstanceID_None)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), type, instanceID));
    }

    struct kTypeFlags {
        enum {
            value = kTypeIsAbstract
        };
    };
    typedef Object ThisType;
protected:
    virtual ~Object();

private:
    static Object* Produce(const HuaHuo::Type* targetCastType, const HuaHuo::Type* produceType, InstanceID instanceID);

};

#endif //PERSISTENTMANAGER_OBJECT_H
