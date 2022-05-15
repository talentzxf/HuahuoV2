#include "Include/Baselib.h"
#include "Include/C/Baselib_RegisteredNetwork.h"

BASELIB_C_INTERFACE
{
    Baselib_RegisteredNetwork_BufferSlice Baselib_RegisteredNetwork_BufferSlice_Create(
        Baselib_RegisteredNetwork_Buffer buffer,
        uint32_t                         offset,
        uint32_t                         size
    )
    {
        Baselib_RegisteredNetwork_BufferSlice slice;
        slice.id = buffer.id;
        slice.data = (char*)buffer.allocation.ptr + offset;
        slice.offset = offset;
        slice.size = size;
        return slice;
    }

    Baselib_RegisteredNetwork_BufferSlice Baselib_RegisteredNetwork_BufferSlice_Empty(void)
    {
        Baselib_RegisteredNetwork_BufferSlice slice;
        slice.id = Baselib_RegisteredNetwork_Buffer_Id_Invalid;
        slice.data = NULL;
        slice.offset = 0;
        slice.size = 0;
        return slice;
    }

    Baselib_RegisteredNetwork_Endpoint Baselib_RegisteredNetwork_Endpoint_Empty(void)
    {
        Baselib_RegisteredNetwork_Endpoint endpoint;
        endpoint.slice = Baselib_RegisteredNetwork_BufferSlice_Empty();
        return endpoint;
    }
}
