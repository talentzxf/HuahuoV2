//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_STLALLOCATOR_H
#define HUAHUOENGINE_STLALLOCATOR_H

#include "Memory/MemoryMacros.h"
#include "Memory/AllocatorLabels.h"

#if COMPILER_SUPPORTS_STATEFUL_ALLOCATORS
#if PLATFORM_OSX && defined(__GLIBCXX__) // Mac uses old stdc++ where some things are pre-C++11 (TR1)
        #include <tr1/type_traits>
    #endif
#endif

#include <cstddef> // ptrdiff_t

// Use STL_ALLOCATOR macro when declaring custom std::containers
#define STL_ALLOCATOR(label, type) ::stl_allocator<type, label##Id>
#define STL_ALLOCATOR_ALIGNED(label, type, align) ::stl_allocator<type, label##Id, align>

#define UNITY_LIST(label, type) std::list<type, STL_ALLOCATOR(label, type) >

#define UNITY_SET(label, type) std::set<type, std::less<type>, STL_ALLOCATOR(label, type) >
#define UNITY_SET_CMP(label, type, compare) std::set<type, compare, STL_ALLOCATOR(label, type) >

#define UNITY_MULTISET(label, type) std::multiset<type, std::less<type>, STL_ALLOCATOR(label, type) >

#define UNITY_MAP(label, key, value) std::map<key, value, std::less<key>, stl_allocator< std::pair< key const, value>, label##Id > >
#define UNITY_MAP_CMP(label, key, value, compare) std::map<key, value, compare, stl_allocator< std::pair< key const, value>, label##Id > >

#define UNITY_MULTIMAP(label, key, value) std::multimap<key, value, std::less<key>, stl_allocator< std::pair< key const, value>, label##Id > >

#define UNITY_VECTOR(label, type) std::vector<type, STL_ALLOCATOR(label, type) >
#define UNITY_VECTOR_ALIGNED(label, type, align) std::vector<type, STL_ALLOCATOR_ALIGNED(label, type, align) >

#define UNITY_VECTOR_SET(label, type) vector_set<type, std::less<type>, stl_allocator<type, label##Id> >
#define UNITY_VECTOR_MAP(label, key, value) vector_map<key, value, std::less<key>, stl_allocator<std::pair<key, value>, label##Id> >

#define UNITY_TEMP_VECTOR(type) std::vector<type, STL_ALLOCATOR(kMemTempAlloc, type) >

#define UNITY_DENSE_HASH_MAP(label, key, value, hashFunctorType, equalToType) \
    dense_hash_map<key, value, hashFunctorType, equalToType, stl_allocator<std::pair<key const, value>, label##Id> >

#define UNITY_STACK(label, type) std::stack<type, std::deque<type, STL_ALLOCATOR(label, type) > >

#define UNITY_QUEUE(label, type) std::queue<type, std::deque<type, STL_ALLOCATOR(label, type) > >

#define UNITY_DEQUE(label, type) std::deque<type, STL_ALLOCATOR(label, type) >


template<typename T, MemLabelIdentifier memlabel = kMemSTLId, int align = kDefaultMemoryAlignment>
class stl_allocator
{
public:
    typedef size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;
    typedef T         value_type;

#if ENABLE_MEM_PROFILER
    AllocationRootWithSalt rootref;
    AllocationRootWithSalt get_root_ref() const { return rootref; }
    void set_root_ref(AllocationRootWithSalt ref) { rootref = ref; }
#else
    AllocationRootWithSalt get_root_ref() const { return 0; }
    void set_root_ref(AllocationRootWithSalt ref) {}
#endif
    template<typename U> struct rebind { typedef stl_allocator<U, memlabel, align> other; };

    stl_allocator()
    {
        set_root_ref(GET_CURRENT_ALLOC_ROOT_REFERENCE());
    }

    stl_allocator(const stl_allocator& alloc) throw ()
    {
        set_root_ref(alloc.get_root_ref());
    }

    template<typename U, MemLabelIdentifier _memlabel, int _align> stl_allocator(const stl_allocator<U, _memlabel, _align>& alloc) throw ()
    {
        set_root_ref(alloc.get_root_ref());
    }

    ~stl_allocator() throw ()
    {
    }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type count, void const* /*hint*/ = 0)
    {
        return (pointer)HUAHUO_MALLOC_ALIGNED(CreateMemLabel(memlabel, get_root_ref()), count * sizeof(T), align);
    }

    void deallocate(pointer p, size_type /*n*/)
    {
        HUAHUO_FREE(CreateMemLabel(memlabel, get_root_ref()), p);
    }

    template<typename U, MemLabelIdentifier _memlabel, int _align>
    bool operator==(stl_allocator<U, _memlabel, _align> const& a) const
    {
        return _memlabel == memlabel && get_root_ref() == a.get_root_ref();
    }

    template<typename U, MemLabelIdentifier _memlabel, int _align>
    bool operator!=(stl_allocator<U, _memlabel, _align> const& a) const
    {
        return _memlabel != memlabel || get_root_ref() != a.get_root_ref();
    }

    size_type max_size() const throw () {  return 0x7fffffff; }

    void construct(pointer p, const T& val) {  new(p) T(val); }

    void destroy(pointer p)                 {  p->~T(); }

    // C++11 added stateful allocators, and these traits have to be defined
    // in order for containers with different allocators to be able to swap()
    // for example.
#   if COMPILER_SUPPORTS_STATEFUL_ALLOCATORS
    #if PLATFORM_OSX && defined(__GLIBCXX__) // Mac uses old stdc++ where some things are pre-C++11 (TR1)
    using propagate_on_container_copy_assignment = std::tr1::true_type;
    using propagate_on_container_move_assignment = std::tr1::true_type;
    using propagate_on_container_swap = std::tr1::true_type;
    #else
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    #endif
#   endif // if COMPILER_SUPPORTS_STATEFUL_ALLOCATORS
};

#endif //HUAHUOENGINE_STLALLOCATOR_H
