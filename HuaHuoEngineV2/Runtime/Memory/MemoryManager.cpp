//
// Created by VincentZhang on 5/22/2022.
//
#include <cstdlib>
#include "MemoryMacros.h"

void* operator new(size_t size, MemLabelRef label, size_t align, const char* areaName, const char* objectName, const char* file, int line)
{
    void* p = malloc_internal(size, align, label, kAllocateOptionNone, file, line);
#if ENABLE_MEM_PROFILER
    AllocationRootWithSalt root = GetMemoryProfiler()->RegisterRootAllocation(p, size, label, areaName, objectName);
#if ENABLE_AUTO_SCOPED_ROOT
    push_allocation_root(CreateMemLabel(label, root), true);
#endif
#endif
    return p;
}

void* operator new(size_t size, MemLabelRef label, size_t align, const char* file, int line)
{
    void* p = malloc_internal(size, align, label, kAllocateOptionNone, file, line);
    return p;
}

void* operator new[](size_t size, MemLabelRef label, size_t align, const char* areaName, const char* objectName, const char* file, int line)
{
    void* p = malloc_internal(size, align, label, kAllocateOptionNone, file, line);
#if ENABLE_MEM_PROFILER
    AllocationRootWithSalt root = GetMemoryProfiler()->RegisterRootAllocation(p, size, label, areaName, objectName);
#if ENABLE_AUTO_SCOPED_ROOT
    push_allocation_root(CreateMemLabel(label, root), true);
#endif
#endif
    return p;
}

void* operator new[](size_t size, MemLabelRef label, size_t align, const char* file, int line)
{
    void* p = malloc_internal(size, align, label, kAllocateOptionNone, file, line);
    return p;
}


void operator delete(void* p, MemLabelRef label, size_t /*align*/, const char* /*areaName*/, const char* /*objectName*/, const char* file, int line) { free_alloc_internal(p, label, file, line); }
void operator delete(void* p, MemLabelRef label, size_t /*align*/, const char* file, int line) { free_alloc_internal(p, label, file, line); }
void operator delete[](void* p, MemLabelRef label, size_t /*align*/, const char* /*areaName*/, const char* /*objectName*/, const char* file, int line) { free_alloc_internal(p, label, file, line); }
void operator delete[](void* p, MemLabelRef label, size_t /*align*/, const char* file, int line) { free_alloc_internal(p, label, file, line); }


void free_alloc_internal(void* ptr, MemLabelRef label, const char* file, int line)
{
    free(ptr);
}

void* realloc_internal(void* ptr, size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line)
{
    return realloc(ptr, size);
}

