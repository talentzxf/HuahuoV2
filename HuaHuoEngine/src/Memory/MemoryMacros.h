//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_MEMORYMACROS_H
#define HUAHUOENGINE_MEMORYMACROS_H
#include "AllocatorLabels.h"
#include "MemoryMacrosDetails.h"
#include <limits>
#include <cstdio>

enum
{
    kDefaultMemoryAlignment = 16
};

enum AllocateOptions
{
    kAllocateOptionNone = 0,                            // Fatal: Show message box with out of memory error and quit application
    kAllocateOptionReturnNullIfOutOfMemory = 1 << 0,    // Returns null if allocation fails (doesn't show message box)
};

#define NEW_AS_ROOT(type, areaName, objectName) new type
#define NEW(type)   new type
#define DELETE(ptr) {delete ptr; ptr = NULL;}
#define MEMCPY memcpy
#define FREE(ptr) free(ptr)
#define NEW_ARRAY(type, size) new type[size]
#define ALLOC_ARRAY(type, size) (type *)malloc(sizeof(type) * size)
#define REALLOC(type, ptr, size) (type *)realloc(ptr, size)
#define ALLOC(type, size) (type *)malloc(size)
//#define NEW_AS_ROOT(type, areaName, objectName) NewWithLabelConstructor<type>(alignof(type), __FILE__ , __LINE__).NEW_AS_ROOT_WITH_LABEL_CONSTRUCT

EXPORT_COREMODULE void* malloc_internal(size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
EXPORT_COREMODULE void* calloc_internal(size_t count, size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
EXPORT_COREMODULE void* realloc_internal(void* ptr, size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
void free_internal(void* ptr, const char* file, int line);
EXPORT_COREMODULE void  free_alloc_internal(void* ptr, MemLabelRef label, const char* file, int line);
inline void free_alloc_internal(const void* ptr, MemLabelRef label, const char* file, int line)
{
    free_alloc_internal(const_cast<void*>(ptr), label, file, line);
}

#define MALLOC(label, size)                      malloc_internal(size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define MALLOC_NULL(label, size)                 malloc_internal(size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define MALLOC_ALIGNED(label, size, align)       malloc_internal(size, align, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define MALLOC_ALIGNED_NULL(label, size, align)  malloc_internal(size, align, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define CALLOC(label, count, size)               calloc_internal(count, size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define CALLOC_NULL(label, count, size)          calloc_internal(count, size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
// #define REALLOC(label, ptr, size)                realloc_internal(ptr, size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define REALLOC_NULL(label, ptr, size)           realloc_internal(ptr, size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define REALLOC_ALIGNED(label, ptr, size, align) realloc_internal(ptr, size, align, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define REALLOC_ALIGNED_NULL(label, ptr, size, align) realloc_internal(ptr, size, align, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)

template<typename T>
inline bool DoesAdditionOverflow(T a, T b)
{
    return std::numeric_limits<T>::max() - a < b;
}
#endif //HUAHUOENGINE_MEMORYMACROS_H
