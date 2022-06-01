//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_MEMORYMACROS_H
#define HUAHUOENGINE_MEMORYMACROS_H
#include "AllocatorLabels.h"

#include <limits>
#include <cstdio>

#include "Utilities/NonCopyable.h"
#include "Utilities/Annotations.h"
#include "Core/ConstructorUtility.h"
#include <limits>
#include <new>

//#if defined(_MSC_VER)
//#include <malloc.h> // _alloca
//#elif !PLATFORM_PLAYSTATION
//#include <alloca.h>     // used in ALLOC_TEMP_ALIGNED
//#endif

// __FILE_STRIPPED__ should be used in place of __FILE__ when you only want to embed the source filename for diagnostic
// purposes, and don't want it to take up space in a final release.
// (formerly part of CoreMacros)
#if ENABLE_STRIPPING_SOURCE_FILENAMES
#define __FILE_STRIPPED__ ""
#else
#define __FILE_STRIPPED__ __FILE__
#endif

enum
{
    kDefaultMemoryAlignment = 16
};

enum AllocateOptions
{
    kAllocateOptionNone = 0,                            // Fatal: Show message box with out of memory error and quit application
    kAllocateOptionReturnNullIfOutOfMemory = 1 << 0,    // Returns null if allocation fails (doesn't show message box)
};

// C++11 deprecated usage of throw exceptions specifiers leaving only 2 cases: all exceptions and no exceptions.
// Thus operator new should be overloaded as:
//   void* operator new (size_t size); - for C++11 and above;
//   void* operator new (size_t size) throw(std:bad_alloc); - for C++98;
#if (defined(_MSC_VER) && (_MSC_VER >= 1600)) || ((defined(__clang__) || defined(__GNUC__)) && (__cplusplus > 199711L))
#define THROWING_NEW_REQUIRE_THROW 0
#else
#define THROWING_NEW_REQUIRE_THROW 1
#endif

#if THROWING_NEW_REQUIRE_THROW
#if defined(EMSCRIPTEN)
        #define THROWING_NEW_THROW throw()
    #else
        #define THROWING_NEW_THROW throw(std::bad_alloc)
    #endif
#else
#define THROWING_NEW_THROW
#endif

#ifndef HUAHUO_ALLOC_ALLOW_NEWDELETE_OVERRIDE
#if defined(HUAHUO_PLATFORM_DECLARES_NEWDELETE)
#define HUAHUO_ALLOC_ALLOW_NEWDELETE_OVERRIDE !(HUAHUO_PLATFORM_DECLARES_NEWDELETE)
#else
#define HUAHUO_ALLOC_ALLOW_NEWDELETE_OVERRIDE !((PLATFORM_OSX || PLATFORM_LINUX) && HUAHUO_EDITOR)
#endif
#endif

#if ENABLE_MEMORY_MANAGER && HUAHUO_ALLOC_ALLOW_NEWDELETE_OVERRIDE

void* operator new(size_t size) THROWING_NEW_THROW;
void* operator new[](size_t size) THROWING_NEW_THROW;
void operator delete(void* p) throw ();
void operator delete[](void* p) throw ();

void* operator new(size_t size, const std::nothrow_t&) throw ();
void* operator new[](size_t size, const std::nothrow_t&) throw ();
void operator delete(void* p, const std::nothrow_t&) throw ();
void operator delete[](void* p, const std::nothrow_t&) throw ();

#endif // ENABLE_MEMORY_MANAGER && HUAHUO_ALLOC_ALLOW_NEWDELETE_OVERRIDE

#if ENABLE_MEM_PROFILER
class BaseAllocator;

EXPORT_COREMODULE AllocationRootWithSalt    get_root_reference(void* ptr, MemLabelRef label);
EXPORT_COREMODULE AllocationRootWithSalt    assign_allocation_root(void* root, size_t size,  MemLabelRef label, const char* areaName, const char* objectName);
EXPORT_COREMODULE void                      retain_root_reference(AllocationRootWithSalt root);
EXPORT_COREMODULE void                      release_root_reference(AllocationRootWithSalt root);

#if ENABLE_AUTO_SCOPED_ROOT
EXPORT_COREMODULE bool                      push_allocation_root(MemLabelId label, bool forcePush);
EXPORT_COREMODULE void                      pop_allocation_root();
EXPORT_COREMODULE AllocationRootWithSalt    get_current_allocation_root_reference_internal();
namespace memorymacros
{
namespace detail
{
    class AutoAllocationRoot
    {
    public:
        AutoAllocationRoot(MemLabelId label)
        {
            pushed = push_allocation_root(label, false);
        }

        ~AutoAllocationRoot()
        {
            if (pushed)
                pop_allocation_root();
        }

        bool pushed;
    };

    class AutoRetainRoot
    {
    public:
        AutoRetainRoot(AllocationRootWithSalt root) : root(root)
        {
            retain_root_reference(root);
        }

        ~AutoRetainRoot()
        {
            release_root_reference(root);
        }

        AllocationRootWithSalt root;
    };
}
}

#define GET_CURRENT_ALLOC_ROOT_REFERENCE()  get_current_allocation_root_reference_internal()
#define HAS_ROOT_REFERENCE(rootref)         (rootref.m_RootReferenceIndex != -1)
#define RETAIN_ROOT_REFERENCE(root)         retain_root_reference(root)
#define RELEASE_ROOT_REFERENCE(root)        release_root_reference(root)

#define AUTO_ALLOCATION_ROOT(ROOT_LABEL_)   memorymacros::detail::AutoAllocationRoot autoAllocationRoot(ROOT_LABEL_)
#define AUTO_RETAIN_ROOT_REFERENCE(ROOT)    memorymacros::detail::AutoRetainRoot autoRetainRoot(ROOT)
#else // ENABLE_AUTO_SCOPED_ROOT
// TODO: Remove and Fix all code that uses these 2 macros
#define GET_CURRENT_ALLOC_ROOT_REFERENCE()  ((AllocationRootReference*)NULL)
#define AUTO_ALLOCATION_ROOT(ROOT_)         PP_EMPTY_STATEMENT
#endif // ENABLE_AUTO_SCOPED_ROOT
#define RETAIN_ROOT_REFERENCE(root) retain_root_reference(root)
#define RELEASE_ROOT_REFERENCE(root) release_root_reference(root)

#define GET_ROOT_REFERENCE                  get_root_reference
#define SET_ROOT_REFERENCE                  set_root_reference
#define HUAHUO_TRANSFER_OWNERSHIP            transfer_ownership

#else // ENABLE_MEM_PROFILER

#define GET_CURRENT_ALLOC_ROOT_REFERENCE()  (0)
#define HAS_ROOT_REFERENCE(...)             false
#define AUTO_ALLOCATION_ROOT(...)           PP_EMPTY_STATEMENT
#define HUAHUO_TRANSFER_OWNERSHIP(...)       PP_EMPTY_STATEMENT
#define SET_ROOT_REFERENCE(...)             PP_EMPTY_STATEMENT

#endif // ENABLE_MEM_PROFILER

// deprecated - use AUTO_ALLOCATION_ROOT instead
#define SET_ALLOC_OWNER AUTO_ALLOCATION_ROOT
#define CLEAR_ALLOC_OWNER AUTO_ALLOCATION_ROOT( kMemDefault )

EXPORT_COREMODULE void* operator new(size_t size, MemLabelRef label, size_t align, const char* file, int line);
EXPORT_COREMODULE void* operator new[](size_t size, MemLabelRef label, size_t align, const char* file, int line);

EXPORT_COREMODULE void operator delete(void* p, MemLabelRef label, size_t align, const char* file, int line);
EXPORT_COREMODULE void operator delete[](void* p, MemLabelRef label, size_t align, const char* file, int line);

EXPORT_COREMODULE void* operator new(size_t size, MemLabelRef label, size_t align, const char* areaName, const char* objectName, const char* file, int line);
EXPORT_COREMODULE void operator delete(void* p, MemLabelRef label, size_t align, const char* areaName, const char* objectName, const char* file, int line);

EXPORT_COREMODULE void* operator new(size_t size, MemLabelRef label, size_t align, const char* objectName, const char* file, int line);
EXPORT_COREMODULE void operator delete(void* p, MemLabelRef label, size_t align, const char* objectName, const char* file, int line);

EXPORT_COREMODULE void* malloc_internal(size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
EXPORT_COREMODULE void* calloc_internal(size_t count, size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
EXPORT_COREMODULE void* realloc_internal(void* ptr, size_t size, size_t align, MemLabelRef label, AllocateOptions allocateOptions, const char* file, int line);
void free_internal(void* ptr, const char* file, int line);
EXPORT_COREMODULE void  free_alloc_internal(void* ptr, MemLabelRef label, const char* file, int line);
inline void free_alloc_internal(const void* ptr, MemLabelRef label, const char* file, int line)
{
    free_alloc_internal(const_cast<void*>(ptr), label, file, line);
}

void transfer_ownership(void* source, MemLabelRef label, AllocationRootWithSalt newRootRef);
EXPORT_COREMODULE bool try_to_transfer_between_label(void* ptr, MemLabelRef orgLabel, MemLabelRef newLabel, size_t size, size_t align, AllocateOptions allocateOptions, const char* file, int line);

#define HUAHUO_MALLOC(label, size)                      malloc_internal(size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_MALLOC_NULL(label, size)                 malloc_internal(size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_MALLOC_ALIGNED(label, size, align)       malloc_internal(size, align, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_MALLOC_ALIGNED_NULL(label, size, align)  malloc_internal(size, align, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_CALLOC(label, count, size)               calloc_internal(count, size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_CALLOC_NULL(label, count, size)          calloc_internal(count, size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_REALLOC(label, ptr, size)                realloc_internal(ptr, size, kDefaultMemoryAlignment, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_REALLOC_NULL(label, ptr, size)           realloc_internal(ptr, size, kDefaultMemoryAlignment, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_REALLOC_ALIGNED(label, ptr, size, align) realloc_internal(ptr, size, align, label, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_REALLOC_ALIGNED_NULL(label, ptr, size, align) realloc_internal(ptr, size, align, label, kAllocateOptionReturnNullIfOutOfMemory, __FILE_STRIPPED__, __LINE__)
#define HUAHUO_FREE(label, ptr)                         free_alloc_internal(ptr, label, __FILE_STRIPPED__, __LINE__)

#define HUAHUO_TRY_TRANSFER_LABEL(ptr, labelOld, labelNew, size)  try_to_transfer_between_label(ptr, labelOld, labelNew, size, kDefaultMemoryAlignment, kAllocateOptionNone, __FILE_STRIPPED__, __LINE__)

template<typename T>
inline void delete_internal(T* ptr, MemLabelRef label, const char* file, int line)
{
    if (ptr)
    {
        ptr->~T();
        free_alloc_internal((void*)ptr, label, file, line);
    }
}

#define HUAHUO_NEW(type, label)                          new (label, alignof(type), __FILE_STRIPPED__, __LINE__) type
#define HUAHUO_NEW_ALIGNED(type, label, align)           new (label, align, __FILE_STRIPPED__, __LINE__) type

#define HUAHUO_NEW_AUTOLABEL(type, label)                          AutoLabelConstructor<type>::construct(HUAHUO_MALLOC_ALIGNED(label, sizeof(type), alignof(type)), label)
#define HUAHUO_NEW_AUTOLABEL_ALIGNED(type, label, align)           AutoLabelConstructor<type>::construct(HUAHUO_MALLOC_ALIGNED(label, sizeof(type), align), label)

#define HUAHUO_DELETE(ptr, label)                        { delete_internal(ptr, label, __FILE_STRIPPED__, __LINE__); ptr = NULL; }

#if ENABLE_MEM_PROFILER
// Check the integrity of the allocator backing a label. Use this to track down memory overwrites
void ValidateAllocatorIntegrity(MemLabelId label);

void register_external_gfx_allocation(void* ptr, size_t size, size_t related, const char* file, int line);
void register_external_gfx_deallocation(void* ptr, const char* file, int line);
void update_external_gfx_allocation(void* ptr, size_t related);

    #define HUAHUO_NEW_AS_ROOT(type, label, areaName, objectName) NewWithLabelConstructor<type>(label, alignof(type), areaName, objectName, __FILE_STRIPPED__, __LINE__).HUAHUO_NEW_AS_ROOT_WITH_LABEL_CONSTRUCT
    #define HUAHUO_NEW_AS_ROOT_ALIGNED(type, label, align, areaName, objectName) NewWithLabelConstructor<type>(label, align, areaName, objectName, __FILE_STRIPPED__, __LINE__).HUAHUO_NEW_AS_ROOT_WITH_LABEL_CONSTRUCT
// TODO: Remove NO_LABEL once we have MemLabelRef in all constructors
// NB: still uses default align because there is no way to call ctor with args otherwise
    #define HUAHUO_NEW_AS_ROOT_NO_LABEL(type, label, areaName, objectName) pop_allocation_root_after_new(new (label, kDefaultMemoryAlignment, areaName, objectName, __FILE_STRIPPED__, __LINE__) type )
    #define HUAHUO_NEW_AS_ROOT_ALIGNED_NO_LABEL(type, label, align, areaName, objectName) pop_allocation_root_after_new(new (label, align, areaName, objectName, __FILE_STRIPPED__, __LINE__) type )
    #define SET_PTR_AS_ROOT(ptr, size, label, areaName, objectName)     assign_allocation_root(ptr, size, label, areaName, objectName)
    #define REGISTER_EXTERNAL_GFX_ALLOCATION_REF(ptr, size, related)    register_external_gfx_allocation((void*)ptr, size, (size_t)related, __FILE_STRIPPED__, __LINE__)
    #define REGISTER_EXTERNAL_GFX_DEALLOCATION(ptr)                     register_external_gfx_deallocation((void*)ptr, __FILE_STRIPPED__, __LINE__)
    #define UPDATE_EXTERNAL_GFX_ALLOCATION(ptr, related)                update_external_gfx_allocation((void*)ptr, (size_t)related)
#else
#define HUAHUO_NEW_AS_ROOT(type, label, areaName, objectName) NewWithLabelConstructor<type>(label, alignof(type), __FILE_STRIPPED__, __LINE__).HUAHUO_NEW_AS_ROOT_WITH_LABEL_CONSTRUCT
#define HUAHUO_NEW_AS_ROOT_ALIGNED(type, label, align, areaName, objectName) NewWithLabelConstructor<type>(label, align, __FILE_STRIPPED__, __LINE__).HUAHUO_NEW_AS_ROOT_WITH_LABEL_CONSTRUCT
#define HUAHUO_NEW_AS_ROOT_NO_LABEL(type, label, areaName, objectName) new (label, kDefaultMemoryAlignment, __FILE_STRIPPED__, __LINE__) type
#define HUAHUO_NEW_AS_ROOT_ALIGNED_NO_LABEL(type, label, align, areaName, objectName) new (label, align, __FILE_STRIPPED__, __LINE__) type
#define SET_PTR_AS_ROOT(ptr, size, label, areaName, objectName)     0
#define REGISTER_EXTERNAL_GFX_ALLOCATION_REF(ptr, size, related)    PP_EMPTY_STATEMENT
#define REGISTER_EXTERNAL_GFX_DEALLOCATION(ptr)                     PP_EMPTY_STATEMENT
#define UPDATE_EXTERNAL_GFX_ALLOCATION(ptr, related)              PP_EMPTY_STATEMENT
#endif


#include "MemoryMacrosDetails.h"
#include "STLAllocator.h"

#if HUAHUO_USE_PLATFORM_MEMORYMACROS         // define HUAHUO_USE_PLATFORM_MEMORYMACROS in PlatformPrefixConfigure.h
#include "Allocator/PlatformMemoryMacros.h"
#endif

inline void* AlignPtr(void* p, size_t alignment)
{
    size_t a = alignment - 1;
    return (void*)(((size_t)p + a) & ~a);
}

template<typename T>
inline T constexpr AlignSize(T size, size_t alignment)
{
    return ((size + (alignment - 1)) & ~(alignment - 1));
}

template<typename T>
inline size_t GetTypeAlignmentFromPointer(T* const &)
{
    return alignof(T);
}

/// Return maximum alignment. Both alignments must be 2^n.
inline size_t MaxAlignment(size_t alignment1, size_t alignment2)
{
    return ((alignment1 - 1) | (alignment2 - 1)) + 1;
}

class AutoFree : NonCopyable
{
    void* m_mem;
    MemLabelId m_label;

public:
    AutoFree(void* takeMem, MemLabelId label) : m_mem(takeMem), m_label(label) {}
    AutoFree() : m_mem(NULL), m_label(kMemDefault) {}
    ~AutoFree() { HUAHUO_FREE(m_label, m_mem); }

    void* Init(void* takeMem, MemLabelId label)
    {
        //DebugAssertMsg(m_mem == NULL, "No double-init"); // uncomment this when dependency-free assert available -scobi 23-dec-15
        m_mem = takeMem;
        m_label = label;

        return takeMem;
    }
};

// ALLOC_TEMP_AUTO allocates temporary memory that stays alive only inside the function it was allocated in.
// It will automatically get freed!
//
// WARNING: ALLOC_TEMP_AUTO may use alloca() which means that lifetime is not bound to the scope that ALLOC_TEMP_AUTO is used
//          in but rather to the current stack frame (usually means the current function). So, do not use ALLOC_TEMP_AUTO
//          directly inside loops or you risk overflowing the stack. If you need a temporary allocation inside a loop
//          body, move the loop body into its own function.
//
// (Watch out that you don't place ALLOC_TEMP_AUTO inside an if block and use the memory after the if block.)
//
// e.g.
//  float* data;
//  ALLOC_TEMP_AUTO(data, 500);

#define ALLOC_TEMP_ALIGNED_AUTO(PTR_, COUNT_, ALIGNMENT_) \
    DETAIL__ALLOC_TEMP_AUTO(PTR_, (COUNT_) * sizeof(*PTR_), ALIGNMENT_)

#define ALLOC_TEMP_AUTO(PTR_, COUNT_) \
    ALLOC_TEMP_ALIGNED_AUTO(PTR_, COUNT_, GetTypeAlignmentFromPointer(PTR_))

#define MALLOC_TEMP_AUTO(PTR_, SIZE_) \
    DETAIL__ALLOC_TEMP_AUTO(PTR_, SIZE_, 1)

// MANUAL here means that, unlike ALLOC_TEMP_AUTO, it will not auto-free (and will not attempt to use alloca).
// You must pair each ALLOC_TEMP_MANUAL with a FREE_TEMP_MANUAL.

#define ALLOC_TEMP_MANUAL(TYPE_, COUNT_) \
    reinterpret_cast<TYPE_*>(HUAHUO_MALLOC_ALIGNED(kMemTempAlloc, (COUNT_) * sizeof(TYPE_), alignof(TYPE_)))

#define MALLOC_TEMP_MANUAL(SIZE_) \
    HUAHUO_MALLOC(kMemTempAlloc, SIZE_)

#define FREE_TEMP_MANUAL(PTR_) \
    HUAHUO_FREE(kMemTempAlloc, PTR_)

#define HUAHUO_MEMCPY memcpy

template<typename T>
inline bool DoesAdditionOverflow(T a, T b)
{
    return std::numeric_limits<T>::max() - a < b;
}

template<typename T>
inline bool DoesMultiplicationOverflow(T a, T b)
{
    return b != 0 && std::numeric_limits<T>::max() / b < a;
}

// ----- macro 'private' section -----

#define DETAIL__ALLOC_TEMP_AUTO(PTR_, SIZE_, ALIGNMENT_)                                \
    AutoFree autoFree_##PTR_;                                                   \
    {                                                                           \
        void* ptr_ = NULL;                                                      \
        const size_t size_ = SIZE_;                                             \
        const size_t alignment_ = ALIGNMENT_;                                   \
                                                                                \
        if (size_ != 0)                                                         \
        {                                                                       \
            const size_t kMAX_TEMP_STACK_SIZE = 2000;                           \
            const size_t allocaSize_ = size_ + alignment_ - 1;                  \
                                                                                \
            /* NOTE: alloca may return NULL on some platforms (like iOS) */     \
            if (allocaSize_ < kMAX_TEMP_STACK_SIZE)                             \
                ptr_ = alloca(allocaSize_);                                     \
                                                                                \
            if (ptr_ == NULL)                                                   \
            {                                                                   \
                ptr_ = HUAHUO_MALLOC_ALIGNED(kMemTempAlloc, size_, alignment_);  \
                autoFree_##PTR_.Init(ptr_, kMemTempAlloc);                      \
            }                                                                   \
        }                                                                       \
                                                                                \
        (void*&)(PTR_) = AlignPtr(ptr_, alignment_);                            \
    }                                                                           \
    ANALYSIS_ASSUME(PTR_)
#endif //HUAHUOENGINE_MEMORYMACROS_H
