#pragma once

#include <dlfcn.h>
#include <cstring>

namespace DarwinApi
{
    static inline void Thread_SetNameForCurrentThread(const char* name)
    {
        typedef int(*pthread_setname_np_type)(const char*);
        pthread_setname_np_type dynamic_pthread_setname_np;
        dynamic_pthread_setname_np = reinterpret_cast<pthread_setname_np_type>(dlsym(RTLD_DEFAULT, "pthread_setname_np"));
        if (dynamic_pthread_setname_np)
            dynamic_pthread_setname_np(name);
    }
}
