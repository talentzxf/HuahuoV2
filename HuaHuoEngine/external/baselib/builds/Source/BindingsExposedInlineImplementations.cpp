// Set BASELIB_BINDING_GENERATION, so all BASELIB_FORCEINLINE_API/BASELIB_API become regular apis.

// All of this only makes sense for p/invoke based bindings (Unity bindings don't need exported symbols in the first place)
//
// P/invoke only work if inline namespaces are disabled.
// (Note that BASELIB_DYNAMICLIBRARY is not a requirement since il2cpp may be used in a fully static setup, directly linking with these functions)
#if !defined(BASELIB_INLINE_NAMESPACE)

#define BASELIB_BINDING_GENERATION 1
#include "Include/Baselib.h"
#include "Include/C/Baselib_ThreadLocalStorage.h"
#undef BASELIB_BINDING_GENERATION

#endif
