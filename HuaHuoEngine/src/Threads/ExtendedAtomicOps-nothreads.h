static inline void atomic_thread_fence(int)
{
}

static inline atomic_word atomic_load_explicit(const volatile atomic_word* p, int)
{
    return *p;
}

static inline atomic_word2 atomic_load_explicit(const volatile atomic_word2* p, int)
{
    atomic_word2 v;
    v.v = p->v;
    return v;
}

static inline void atomic_store_explicit(volatile atomic_word* p, atomic_word v, int)
{
    *p = v;
}

static inline void atomic_store_explicit(volatile atomic_word2* p, atomic_word2 v, int)
{
    p->v = v.v;
}

static inline void atomic_init_safe_explicit(volatile atomic_word2* p, atomic_word v, int x)
{
    atomic_word2 w;
    w.lo = v;
    w.hi = 0;
    p->v = w.v;
}

static inline atomic_word atomic_exchange_explicit(volatile atomic_word* p, atomic_word val, int)
{
    atomic_word res = *p;
    *p = val;
    return res;
}

static inline atomic_word2 atomic_exchange_explicit(volatile atomic_word2* p, atomic_word2 val, int)
{
    atomic_word2 res;
    res.v = p->v;
    p->v = val.v;
    return res;
}

static inline bool atomic_compare_exchange_strong_explicit(volatile atomic_word* p, atomic_word* oldval, atomic_word newval, int, int)
{
    if (*oldval == *p)
    {
        *p = newval;
        return true;
    }
    else
    {
        *oldval = *p;
        return false;
    }
}

static inline bool atomic_compare_exchange_strong_explicit(volatile atomic_word2* p, atomic_word2* oldval, atomic_word2 newval, int, int)
{
    if (oldval->v == p->v)
    {
        p->v = newval.v;
        return true;
    }
    else
    {
        oldval->v = p->v;
        return false;
    }
}

static inline bool atomic_compare_exchange_weak_explicit(volatile atomic_word* p, atomic_word* oldval, atomic_word newval, int, int)
{
    return atomic_compare_exchange_strong_explicit(p, oldval, newval, ::memory_order_seq_cst, ::memory_order_seq_cst);
}

static inline bool atomic_compare_exchange_weak_explicit(volatile atomic_word2* p, atomic_word2* oldval, atomic_word2 newval, int, int)
{
    return atomic_compare_exchange_strong_explicit(p, oldval, newval, ::memory_order_seq_cst, ::memory_order_seq_cst);
}

static inline bool atomic_compare_exchange_safe_explicit(volatile atomic_word2* p, atomic_word2* oldval, atomic_word newlo, int, int)
{
    atomic_word2 newval;
    newval.lo = newlo;
    newval.hi = oldval->hi + 1;
    return atomic_compare_exchange_strong_explicit(p, oldval, newval, memory_order_seq_cst, memory_order_seq_cst);
}

static inline atomic_word atomic_fetch_add_explicit(volatile atomic_word *p, atomic_word val, int)
{
    atomic_word res = *p;
    *p += val;
    return res;
}

static inline atomic_word atomic_fetch_sub_explicit(volatile atomic_word *p, atomic_word val, int mo)
{
    return atomic_fetch_add_explicit(p, -val, mo);
}

#ifdef UNITY_ATOMIC_INT_OVERLOAD

static inline int atomic_load_explicit(const volatile int* p, int)
{
    return *p;
}

static inline void atomic_store_explicit(volatile int* p, int v, int)
{
    *p = v;
}

static inline int atomic_exchange_explicit(volatile int* p, int val, int)
{
    int res = *p;
    *p = val;
    return res;
}

static inline bool atomic_compare_exchange_strong_explicit(volatile int* p, int* oldval, int newval, int, int)
{
    if (*oldval == *p)
    {
        *p = newval;
        return true;
    }
    else
        return false;
}

static inline bool atomic_compare_exchange_weak_explicit(volatile int* p, int* oldval, int newval, int, int)
{
    return atomic_compare_exchange_strong_explicit(p, oldval, newval, ::memory_order_seq_cst, ::memory_order_seq_cst);
}

static inline atomic_word atomic_exchange_safe_explicit(volatile atomic_word2* p, atomic_word v, int)
{
    atomic_word2 oldval = atomic_load_explicit(p, memory_order_relaxed);
    p->lo = v;
    p->hi = oldval.hi + 1;
    return oldval.lo;
}

static inline int atomic_fetch_add_explicit(volatile int *p, int val, int)
{
    int res = *p;
    p += val;
    return res;
}

static inline int atomic_fetch_sub_explicit(volatile int *p, int val, int mo)
{
    return atomic_fetch_add_explicit(p, -val, mo);
}

#endif


/*
 *  extensions
 */

static inline void atomic_retain(volatile int *p)
{
    (*p)++;
}

static inline bool atomic_release(volatile int *p)
{
    return --(*p) == 0;
}
