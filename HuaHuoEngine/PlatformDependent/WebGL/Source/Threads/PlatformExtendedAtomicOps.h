#if SUPPORT_THREADS
#   include "ExtendedAtomicOps-webgl-supportthreads.h"
#else
#   include "Runtime/Threads/ExtendedAtomicOps-nothreads.h"
#endif
