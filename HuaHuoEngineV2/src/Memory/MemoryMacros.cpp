//
// Created by VincentZhang on 5/6/2022.
//

#include "MemoryMacros.h"
#include <cstdio>
#include <cstdlib>
#include "AllocatorLabels.h"
void* malloc_internal(size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line){
    return malloc(size);
}

