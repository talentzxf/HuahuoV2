// Copyright (c) 2009-2012 Umbra Software Ltd.
// All rights reserved. www.umbrasoftware.com

#include "umbraOs.hpp"

#if UMBRA_IS_POSIX

#include <sys/time.h>
#include <pthread.h>

double Umbra::OS::getCurrentTime (void)
{
    timeval t;
    gettimeofday(&t, NULL);
    double cur = (double)t.tv_sec + .000001 * t.tv_usec;
    static double first = cur;
    return cur - first;
}

static pthread_key_t makeKey()
{
    pthread_key_t key;
    int success = pthread_key_create(&key, NULL);
    UMBRA_ASSERT(success == 0);
    UMBRA_UNREF(success);
    return key;
}

static pthread_key_t tlsKey = makeKey();

void* Umbra::OS::tlsGetValue()
{
    return pthread_getspecific(tlsKey);
}

void Umbra::OS::tlsSetValue(void* value)
{
    int success = pthread_setspecific(tlsKey, value);
    UMBRA_ASSERT(success == 0);
    UMBRA_UNREF(success);
}

#endif // UMBRA_IS_POSIX
