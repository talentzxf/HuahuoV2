//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_MEMORYMACROSDETAILS_H
#define HUAHUOENGINE_MEMORYMACROSDETAILS_H

#if ENABLE_AUTO_SCOPED_ROOT && ENABLE_MEM_PROFILER
template<typename T>
inline T* pop_allocation_root_after_new(T* p)
{
    // new is called when constructing the argument and p is set as root while in the constructor
    pop_allocation_root();
    return p;
}

#else
template<typename T>
inline T* pop_allocation_root_after_new(T* p)
{
    return p;
}

#endif
template<typename T>
class NewWithLabelConstructor
{
    MemLabelId m_Label;
    void* m_Ptr;

public:
#if ENABLE_MEM_PROFILER
    FORCE_INLINE NewWithLabelConstructor(MemLabelIdentifier label, int align, const char* areaName, const char* objectName, const char* file, int line)
    {
        {
            CLEAR_ALLOC_OWNER;
            m_Ptr = malloc_internal(sizeof(T), align, CreateMemLabel(label), kAllocateOptionNone, file, line);
        }
        AllocationRootWithSalt root = SET_PTR_AS_ROOT(m_Ptr, sizeof(T), CreateMemLabel(label), areaName, objectName);
        m_Label = CreateMemLabel(label, root);
#if ENABLE_AUTO_SCOPED_ROOT
        push_allocation_root(m_Label, true);
#endif
    }

    NewWithLabelConstructor(MemLabelId label, int align, const char* areaName, const char* objectName, const char* file, int line)
    {
        {
            CLEAR_ALLOC_OWNER;
            m_Ptr = malloc_internal(sizeof(T), align, CreateMemLabel(GetLabelIdentifier(label)), kAllocateOptionNone, file, line);
        }
        AllocationRootWithSalt root = SET_PTR_AS_ROOT(m_Ptr, sizeof(T), label, areaName, objectName);
        m_Label = CreateMemLabel(label, root);
#if ENABLE_AUTO_SCOPED_ROOT
        push_allocation_root(m_Label, true);
#endif
    }

#else
    FORCE_INLINE NewWithLabelConstructor(MemLabelId label, int align, const char* file, int line)
            : m_Label(label)
            , m_Ptr(malloc_internal(sizeof(T), align, label, kAllocateOptionNone, file, line))
    {
    }

#endif

    FORCE_INLINE T* construct() { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label)); }

    template<typename P1>
    FORCE_INLINE T* construct(const P1& p1) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1)); }

    template<typename P1, typename P2>
    FORCE_INLINE T* construct(const P1& p1, const P2& p2) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1, p2)); }

    template<typename P1, typename P2, typename P3>
    FORCE_INLINE T* construct(const P1& p1, const P2& p2, const P3& p3) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1, p2, p3)); }

    template<typename P1, typename P2, typename P3, typename P4>
    FORCE_INLINE T* construct(const P1& p1, const P2& p2, const P3& p3, const P4& p4) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1, p2, p3, p4)); }

    template<typename P1, typename P2, typename P3, typename P4, typename P5>
    FORCE_INLINE T* construct(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1, p2, p3, p4, p5)); }

    template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    FORCE_INLINE T* construct(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6) { return pop_allocation_root_after_new(new(m_Ptr) T(m_Label, p1, p2, p3, p4, p5, p6)); }
};
#define HUAHUO_NEW_AS_ROOT_WITH_LABEL_CONSTRUCT(...) construct(__VA_ARGS__)
#endif //HUAHUOENGINE_MEMORYMACROSDETAILS_H
