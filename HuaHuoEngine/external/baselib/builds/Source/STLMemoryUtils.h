#pragma once

#include <limits>
#include <memory>
#include "Include/C/Baselib_Memory.h"

namespace baselib
{
    BASELIB_CPP_INTERFACE
    {
        // std library compatible deleter functior that uses Baselib_Memory_Free.
        template<typename T>
        struct AllocDeleter
        {
            void operator()(T* pointer)
            {
                pointer->~T();
                ::Baselib_Memory_Free(pointer);
            }
        };

        // unique_ptr but using Baselib_Memory_Allocate.
        template<typename T>
        using unique_ptr = std::unique_ptr<T, AllocDeleter<T> >;

        template<typename T, typename ... Args>
        inline unique_ptr<T> make_unique(Args&& ... args)
        {
            void* memory;
            if (alignof(T) <= Baselib_Memory_MinGuaranteedAlignment)
                memory = ::Baselib_Memory_Allocate(sizeof(T));
            else
                memory = ::Baselib_Memory_AlignedAllocate(sizeof(T), alignof(T));
            return unique_ptr<T>(new(memory) T(std::forward<Args>(args)...));
        }

        // std library compatible deleter functior that uses Baselib_Memory_AlignedFree.
        template<typename T>
        struct AlignedAllocDeleter
        {
            void operator()(T* pointer)
            {
                pointer->~T();
                if (alignof(T) <= Baselib_Memory_MinGuaranteedAlignment)
                    ::Baselib_Memory_Free(pointer);
                else
                    ::Baselib_Memory_AlignedFree(pointer);
            }
        };

        // A STL allocator using Baselib_Memory_Allocate
        template<typename T>
        class STLAllocator
        {
        public:
            // typedefs
            typedef T value_type;
            typedef value_type* pointer;
            typedef const value_type* const_pointer;
            typedef value_type& reference;
            typedef const value_type& const_reference;
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;

        public:
            template<typename U>
            struct rebind
            {
                typedef STLAllocator<U> other;
            };

        public:
            inline explicit STLAllocator() {}
            inline ~STLAllocator() {}
            inline explicit STLAllocator(STLAllocator const&) {}
            template<typename U>
            inline explicit STLAllocator(STLAllocator<U> const&) {}

            // address
            inline pointer address(reference r) { return &r; }
            inline const_pointer address(const_reference r) { return &r; }

            // memory allocation
            inline pointer allocate(size_type count, void const* /*hint*/ = 0)
            {
                return (pointer)(Baselib_Memory_Allocate(count * sizeof(T)));
            }

            inline void deallocate(pointer p, size_type)
            {
                Baselib_Memory_Free(p);
            }

            // size
            inline size_type max_size() const
            {
                return std::numeric_limits<size_type>::max() / sizeof(T);
            }

            // construction/destruction
            inline void construct(pointer p, const T& t) { new(p) T(t); }
            inline void destroy(pointer p) { p->~T(); }

            inline bool operator==(STLAllocator const&) { return true; }
            inline bool operator!=(STLAllocator const& a) { return !operator==(a); }
        };
    }
}
