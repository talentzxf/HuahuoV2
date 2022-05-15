#pragma once

#include "Include/C/Baselib_Socket.h"
#include "Source/Baselib_NetworkAddress_Utils.h"
#include "Source/Baselib_ErrorState_Utils.h"

static inline bool IsValid(const Baselib_Socket_Handle& socket)
{
    return socket.handle != Baselib_Socket_Handle_Invalid.handle;
}

static inline bool IsValid(const Baselib_Socket_Protocol& protocol)
{
    switch (protocol)
    {
        case Baselib_Socket_Protocol_UDP:
        case Baselib_Socket_Protocol_TCP:
            return true;
    }
    return false;
}

BASELIB_C_INTERFACE
{
    Baselib_Socket_Handle Baselib_Socket_Create(Baselib_NetworkAddress_Family family, Baselib_Socket_Protocol protocol, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(family);
        errorState |= Validate(protocol);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        return platform::Baselib_Socket_Create(family, protocol, errorState);
    }

    void Baselib_Socket_Bind(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(address));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Socket_Bind(socket, address, addressReuse, errorState);
    }

    void Baselib_Socket_TCP_Connect(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(address));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Socket_TCP_Connect(socket, address, addressReuse, errorState);
    }

    void Baselib_Socket_Poll(Baselib_Socket_PollFd* sockets, uint32_t socketsCount, uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(AsPointer(sockets));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        for (uint32_t i = 0; i < socketsCount; i++)
        {
            errorState |= Validate(sockets[i].handle);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;

            if ((sockets[i].requestedEvents & (Baselib_Socket_PollEvents_Readable | Baselib_Socket_PollEvents_Writable)) != 0 &&
                (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Connected) != 0)
            {
                errorState |= RaiseInvalidArgument(sockets);
                return;
            }
        }

        return platform::Baselib_Socket_Poll(sockets, socketsCount, timeoutInMilliseconds, errorState);
    }

    void Baselib_Socket_GetAddress(Baselib_Socket_Handle socket, Baselib_NetworkAddress* address, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(address));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Socket_GetAddress(socket, address, errorState);
    }

    void Baselib_Socket_TCP_Listen(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        return platform::Baselib_Socket_TCP_Listen(socket, errorState);
    }

    Baselib_Socket_Handle Baselib_Socket_TCP_Accept(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        return platform::Baselib_Socket_TCP_Accept(socket, errorState);
    }

    uint32_t Baselib_Socket_UDP_Send(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(messages));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;

        return platform::Baselib_Socket_UDP_Send(socket, messages, messagesCount, errorState);
    }

    uint32_t Baselib_Socket_TCP_Send(Baselib_Socket_Handle socket, void* data, uint32_t dataLen, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(data));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;

        if (dataLen == 0)
            return 0;

        return platform::Baselib_Socket_TCP_Send(socket, data, dataLen, errorState);
    }

    uint32_t Baselib_Socket_UDP_Recv(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(messages));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;

        return platform::Baselib_Socket_UDP_Recv(socket, messages, messagesCount, errorState);
    }

    uint32_t Baselib_Socket_TCP_Recv(Baselib_Socket_Handle socket, void* data, uint32_t dataLen, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(socket);
        errorState |= Validate(AsPointer(data));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return 0;

        return platform::Baselib_Socket_TCP_Recv(socket, data, dataLen, errorState);
    }

    void Baselib_Socket_Close(Baselib_Socket_Handle socket)
    {
        return platform::Baselib_Socket_Close(socket);
    }
}
