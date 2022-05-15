#pragma once

// A small COM smart pointer.
// Does all AddRef/Release magic on constructions, assignments,
// destructions etc.
//
// Just use it like SmartComPointer<MyInterface> and not worry about reference counting
// anymore.

#include <windef.h>
#include <unknwn.h>

// Disallow calling AddRef/Release on smart pointer like this: ptr->AddRef or ptr->Release.
template<class T>
class NoAddRefReleaseOnSmartComPtr : public T
{
private:
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
};


inline IUnknown* SmartComPtrAssign(IUnknown** pp, IUnknown* lp)
{
    if (pp == NULL)
        return NULL;

    if (lp != NULL)
        lp->AddRef();
    if (*pp)
        (*pp)->Release();
    *pp = lp;
    return lp;
}

template<class T>
class SmartComPointer
{
public:
    SmartComPointer() throw ()
        : p(NULL) {}
    SmartComPointer(int nNull) throw ()
        : p(NULL)
    {
        DebugAssert(nNull == 0);
        (void)nNull;
    }

    SmartComPointer(T* lp) throw ()
        : p(lp)
    {
        if (p != NULL)
            p->AddRef();
    }

    SmartComPointer(const SmartComPointer<T>& lp) throw ()
        : p(lp.p)
    {
        if (p != NULL)
            p->AddRef();
    }

    ~SmartComPointer() throw ()
    {
        if (p)
            p->Release();
    }

    operator T*() const throw ()
          {
              return p;
          }
          T& operator*() const throw ()
          {
              Assert(p != NULL);
              return *p;
          }

          T** operator&() throw ()
          {
              // The assert usually indicates a bug.  If this is really
              // what is needed, however, take the address of the p member
              // explicitly.
              Assert(p == NULL);
              return &p;
          }

          NoAddRefReleaseOnSmartComPtr<T>* operator->() const throw ()
          {
              Assert(p != NULL);
              return (NoAddRefReleaseOnSmartComPtr<T>*)p;
          }

          bool operator!() const throw ()
          {
              return (p == NULL);
          }

          bool operator<(T* pT) const throw ()
          {
              return p < pT;
          }

          bool operator==(T* pT) const throw ()
          {
              return p == pT;
          }

          // Release the interface and set to NULL
          void Release() throw ()
          {
              T* pTemp = p;
              if (pTemp)
              {
                  p = NULL;
                  pTemp->Release();
              }
          }

          // Attach to an existing interface (does not AddRef)
          void Attach(T* p2) throw ()
          {
              if (p)
                  p->Release();
              p = p2;
          }

          // Detach the interface (does not Release)
          T* Detach() throw ()
          {
              T* pt = p;
              p = NULL;
              return pt;
          }

          T* operator=(T* lp) throw () { return static_cast<T*>(SmartComPtrAssign((IUnknown**)&p, lp)); }
          T* operator=(const SmartComPointer<T>& lp) throw () { return static_cast<T*>(SmartComPtrAssign((IUnknown**)&p, lp)); }

      public:
          T* p;
};
