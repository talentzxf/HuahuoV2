#pragma once

#include "Include/Baselib.h"

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Socket.h"
#include "Include/Cpp/Algorithm.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"
#include "Source/Baselib_NetworkAddress_Utils.h"
#include "Source/WinApi/ErrorStateBuilder_WinApi.inl.h"
#include "Source/ArgumentValidator.h"

#include <ws2tcpip.h>
#include <excpt.h>

namespace WinApi
{
namespace detail
{
    struct socketaddr : sockaddr_storage
    {
        socketaddr()
        {
            memset(this, 0, sizeof(*this));
        }

        socketaddr(const Baselib_NetworkAddress* address, Baselib_ErrorState* errorState)
        {
            memset(this, 0, sizeof(*this));

            errorState |= Validate(AsPointer(address));
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;

            static_assert(sizeof(in4()->sin_port)  == sizeof(address->port), "ensure same size");
            static_assert(sizeof(in6()->sin6_port) == sizeof(address->port), "ensure same size");
            static_assert(sizeof(in4()->sin_addr)  == sizeof(address->ipv4), "ensure same size");
            static_assert(sizeof(in6()->sin6_addr) == sizeof(address->ipv6), "ensure same size");

            switch (address->family)
            {
                case Baselib_NetworkAddress_Family_IPv4:
                    in4()->sin_family = AF_INET;
                    memcpy(&in4()->sin_port, address->port, sizeof(address->port));
                    memcpy(&in4()->sin_addr, address->ipv4, sizeof(address->ipv4));
                    break;
                case Baselib_NetworkAddress_Family_IPv6:
                    in6()->sin6_family = AF_INET6;
                    memcpy(&in6()->sin6_port, address->port, sizeof(address->port));
                    memcpy(&in6()->sin6_addr, address->ipv6, sizeof(address->ipv6));
                    break;
                default:
                    errorState |= RaiseInvalidArgument(address);
                    break;
            }
        }

        void toBaselib(Baselib_NetworkAddress* dst, Baselib_ErrorState* errorState) const
        {
            errorState |= Validate(AsPointer(dst));
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;

            switch (addr()->sa_family)
            {
                case AF_INET:
                    dst->family = Baselib_NetworkAddress_Family_IPv4;
                    memcpy(dst->port, &in4()->sin_port, sizeof(dst->port));
                    memcpy(dst->ipv4, &in4()->sin_addr, sizeof(dst->ipv4));
                    memset(dst->data + sizeof(dst->ipv4), 0, sizeof(dst->data) - sizeof(dst->ipv4)); // Don't leave in an undefined state.
                    break;
                case AF_INET6:
                    dst->family = Baselib_NetworkAddress_Family_IPv6;
                    memcpy(dst->port, &in6()->sin6_port, sizeof(dst->port));
                    memcpy(dst->ipv6, &in6()->sin6_addr, sizeof(dst->ipv6));
                    break;
                default:
                    BaselibAssert(false, "sa_family = %d", addr()->sa_family);
                    errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError);
                    break;
            }
            dst->_padding = 0;
        }

        struct sockaddr*     addr() {return (struct sockaddr*)this;}
        struct sockaddr_in*  in4()  {return (struct sockaddr_in*)this;}
        struct sockaddr_in6* in6()  {return (struct sockaddr_in6*)this;}
        SOCKADDR_INET*       inet() {return (SOCKADDR_INET*)this;}
        const struct sockaddr*     addr() const {return (const struct sockaddr*)this;}
        const struct sockaddr_in*  in4()  const {return (const struct sockaddr_in*)this;}
        const struct sockaddr_in6* in6()  const {return (const struct sockaddr_in6*)this;}
        const SOCKADDR_INET*       inet() const {return (SOCKADDR_INET*)this;}

        uint16_t family() const
        {
            return addr()->sa_family;
        }

        socklen_t size() const
        {
            switch (family())
            {
                case AF_INET:  return sizeof(sockaddr_in);
                case AF_INET6: return sizeof(sockaddr_in6);
                default:       return sizeof(*this);
            }
        }
    };

    static Baselib_ErrorCode SockErrorToErrorCode(int error)
    {
        switch (error)
        {
            case WSAEMSGSIZE:
                return Baselib_ErrorCode_InvalidBufferSize;
            case WSAEADDRINUSE:
                return Baselib_ErrorCode_AddressInUse;
            case WSAEADDRNOTAVAIL:
            case WSAECONNREFUSED:
            case WSAEHOSTDOWN:
            case WSAEHOSTUNREACH:
            case WSAETIMEDOUT:
                return Baselib_ErrorCode_AddressUnreachable;
            case WSAECONNRESET:
                return Baselib_ErrorCode_Disconnected;
            case WSAEACCES:
            case WSAEAFNOSUPPORT:
            case WSAEALREADY:
            case WSAEBADF:
            case WSAEDESTADDRREQ:
            case WSAEFAULT:
            case WSAEINVAL:
            case WSAEISCONN:
            case WSAENETDOWN:
            case WSAENETUNREACH:
            case WSAENOTCONN:
            case WSAENOTSOCK:
            case WSAEOPNOTSUPP:
            case WSAEPROTOTYPE:
            case WSANOTINITIALISED:
                // TODO: Positional argument information (Baselib_ErrorState_RaiseInvalidArgument)
                return Baselib_ErrorCode_InvalidArgument;
            default:
                return Baselib_ErrorCode_UnexpectedError;
        }
    }

    // Based on IL2CPP source:
    // https://github.cds.internal.unity3d.com/unity/il2cpp/blame/master/libil2cpp/os/Win32/SocketImpl.cpp
    static constexpr DWORD SocketExceptionFilter(DWORD exceptionCode)
    {
        // Sometimes, we call the socket functions and close the socket right after,
        // and in some rare cases, it throws EXCEPTION_INVALID_HANDLE SEH exception
        // rather than returning an error code. Although this is undocumented on MSDN,
        // it causes a crash just because it thinks we gave it an invalid handle.
        // We guard against it by wrapping every socket call with __try/__except
        return exceptionCode == EXCEPTION_INVALID_HANDLE ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
    }

    static void SetNonblocking(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        u_long nonBlocking = 1;
        int ioctlsocketResult = SOCKET_ERROR;
        __try
        {
            ioctlsocketResult = ::ioctlsocket(::detail::AsNativeType<SOCKET>(socket), FIONBIO, &nonBlocking);
        }
        __except (SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (ioctlsocketResult == SOCKET_ERROR)
        {
            errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithGetLastError(WSAGetLastError());
            Baselib_Socket_Close(socket);
        }
    }

    static void SetReuseaddr(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        const BOOL reuseAddress = 1;
        int setsockoptResult = SOCKET_ERROR;
        __try
        {
            setsockoptResult = ::setsockopt(::detail::AsNativeType<SOCKET>(socket), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuseAddress), sizeof(reuseAddress));
        }
        __except (SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (setsockoptResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
        }
    }
}

    BASELIB_INLINE_IMPL Baselib_Socket_Handle Baselib_Socket_Create_WSASocket(
        Baselib_NetworkAddress_Family family,
        Baselib_Socket_Protocol protocol,
        DWORD socketCreateFlags,
        Baselib_ErrorState* errorState
    )
    {
        auto addressFamily = family == Baselib_NetworkAddress_Family_IPv4 ? AF_INET : AF_INET6;
        auto socketType = protocol == Baselib_Socket_Protocol_UDP ? SOCK_DGRAM : SOCK_STREAM;
        auto socketProtocol = protocol == Baselib_Socket_Protocol_UDP ? IPPROTO_UDP : IPPROTO_TCP;
        SOCKET socketFd = INVALID_SOCKET;
        __try
        {
            socketFd = WSASocketW(addressFamily, socketType, socketProtocol, NULL, 0, socketCreateFlags);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (socketFd == INVALID_SOCKET)
        {
            const int error = WSAGetLastError();
            Baselib_ErrorCode errorCode = error == WSAEAFNOSUPPORT
                ? Baselib_ErrorCode_AddressFamilyNotSupported
                : Baselib_ErrorCode_UnexpectedError;
            errorState |= RaiseError(errorCode) | WithGetLastError(error);
            return Baselib_Socket_Handle_Invalid;
        }

        return ::detail::AsBaselibHandle<Baselib_Socket_Handle>(socketFd);
    }

    BASELIB_INLINE_IMPL Baselib_Socket_Handle Baselib_Socket_Create(
        Baselib_NetworkAddress_Family family,
        Baselib_Socket_Protocol protocol,
        Baselib_ErrorState* errorState
    )
    {
        Baselib_Socket_Handle socket = Baselib_Socket_Create_WSASocket(family, protocol, 0, errorState);
        if (socket.handle == Baselib_Socket_Handle_Invalid.handle)
            return socket;

        detail::SetNonblocking(socket, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        return socket;
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Close(Baselib_Socket_Handle socket)
    {
        if (socket.handle == Baselib_Socket_Handle_Invalid.handle)
            return;

        __try
        {
            ::shutdown(::detail::AsNativeType<SOCKET>(socket), SD_BOTH);
            ::closesocket(::detail::AsNativeType<SOCKET>(socket));
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Bind(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        if (addressReuse)
        {
            detail::SetReuseaddr(socket, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;
        }

        detail::socketaddr sockAddress(address, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        int bindResult = SOCKET_ERROR;
        __try
        {
            bindResult = ::bind(::detail::AsNativeType<SOCKET>(socket), sockAddress.addr(), sockAddress.size());
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (bindResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return;
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_TCP_Connect(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        SOCKET fd = ::detail::AsNativeType<SOCKET>(socket);

        int socketType;
        int socketTypeLen = sizeof(socketType);
        int getsockoptResult = SOCKET_ERROR;
        __try
        {
            getsockoptResult = ::getsockopt(fd, SOL_SOCKET, SO_TYPE, (char*)&socketType, &socketTypeLen);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (getsockoptResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return;
        }
        if (socketType != SOCK_STREAM)
        {
            errorState |= RaiseInvalidArgument(socket);
            return;
        }

        if (addressReuse)
        {
            detail::SetReuseaddr(socket, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;
        }

        detail::socketaddr sockAddress(address, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        int connectResult = SOCKET_ERROR;
        __try
        {
            connectResult = ::connect(fd, sockAddress.addr(), sockAddress.size());
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (connectResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            // https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
            // "For connection-oriented, nonblocking sockets, it is often not
            // possible to complete the connection immediately. In such a case,
            // this function returns the error WSAEWOULDBLOCK. However, the
            // operation proceeds."
            if (error != WSAEWOULDBLOCK)
                errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Poll(
        Baselib_Socket_PollFd* sockets,
        uint32_t               socketsCount,
        uint32_t               timeoutInMilliseconds,
        Baselib_ErrorState*    errorState)
    {
        // WSAPoll does not detect connection failures. So, use select() instead.
        // https://daniel.haxx.se/blog/2012/10/10/wsapoll-is-broken/

        // windows doesn't use a bitmask, but rather a list, so the limit isn't
        // highest fd, it's number of fds.
        if (socketsCount >= FD_SETSIZE)
        {
            errorState |= RaiseInvalidArgument(socketsCount);
            return;
        }

        fd_set readableSet;
        fd_set writableSet;
        fd_set exceptSet;
        FD_ZERO(&readableSet);
        FD_ZERO(&writableSet);
        FD_ZERO(&exceptSet);

        for (uint32_t i = 0; i < socketsCount; i++)
        {
            SOCKET fd = ::detail::AsNativeType<SOCKET>(sockets[i].handle);
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Readable)
            {
                FD_SET(fd, &readableSet);
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Writable || sockets[i].requestedEvents & Baselib_Socket_PollEvents_Connected)
            {
                FD_SET(fd, &writableSet);
            }
            FD_SET(fd, &exceptSet);
        }

        struct timeval tv;
        tv.tv_sec = timeoutInMilliseconds / 1000;
        tv.tv_usec = (timeoutInMilliseconds % 1000) * 1000;
        // First parameter (https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select):
        // "Ignored. The nfds parameter is included only for compatibility with Berkeley sockets."
        int selectResult = ::select(0, &readableSet, &writableSet, &exceptSet, &tv);
        if (selectResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return;
        }
        // Ignore count returned by select()

        for (uint32_t i = 0; i < socketsCount; i++)
        {
            SOCKET fd = ::detail::AsNativeType<SOCKET>(sockets[i].handle);
            sockets[i].resultEvents = (Baselib_Socket_PollEvents)0;

            if (FD_ISSET(fd, &exceptSet))
            {
                DWORD so_error;
                int len = sizeof(so_error);
                int getsockoptResult = SOCKET_ERROR;
                __try
                {
                    getsockoptResult = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
                }
                __except (detail::SocketExceptionFilter(GetExceptionCode()))
                {
                }
                if (getsockoptResult == SOCKET_ERROR)
                {
                    const int error = WSAGetLastError();
                    errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
                }
                else
                {
                    sockets[i].errorState |= RaiseError(detail::SockErrorToErrorCode(so_error)) | WithGetLastError(so_error);
                }
            }

            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Readable && FD_ISSET(fd, &readableSet) != 0)
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Readable);
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Writable && FD_ISSET(fd, &writableSet) != 0)
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Writable);
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Connected && FD_ISSET(fd, &writableSet) != 0 && !Baselib_ErrorState_ErrorRaised(sockets[i].errorState))
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Connected);
            }
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_GetAddress(Baselib_Socket_Handle socket, Baselib_NetworkAddress* address, Baselib_ErrorState* errorState)
    {
        detail::socketaddr sockAddress;
        socklen_t size = sockAddress.size();
        int getsocknameResult = SOCKET_ERROR;
        __try
        {
            getsocknameResult = ::getsockname(::detail::AsNativeType<SOCKET>(socket), sockAddress.addr(), &size);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (getsocknameResult == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return;
        }

        sockAddress.toBaselib(address, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_TCP_Listen(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        int result = SOCKET_ERROR;
        __try
        {
            result = ::listen(::detail::AsNativeType<SOCKET>(socket), SOMAXCONN);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (result == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
        }
    }

    BASELIB_INLINE_IMPL Baselib_Socket_Handle Baselib_Socket_TCP_Accept(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        SOCKET resultFd = INVALID_SOCKET;
        __try
        {
            resultFd = ::accept(::detail::AsNativeType<SOCKET>(socket), NULL, NULL);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }
        if (resultFd == INVALID_SOCKET)
        {
            const int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
                errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return Baselib_Socket_Handle_Invalid;
        }

        Baselib_Socket_Handle result = ::detail::AsBaselibHandle<Baselib_Socket_Handle>(resultFd);

        detail::SetNonblocking(result, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        return result;
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_UDP_Send(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        for (uint32_t i = 0; i < messagesCount; ++i)
        {
            const auto& message = messages[i];

            detail::socketaddr socketAddr(message.address, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return i;

            int bytesTransferred = SOCKET_ERROR;
            __try
            {
                bytesTransferred = ::sendto(
                    ::detail::AsNativeType<SOCKET>(socket),
                    static_cast<const char*>(message.data), message.dataLen,
                    0,
                    socketAddr.addr(), socketAddr.size());
            }
            __except (detail::SocketExceptionFilter(GetExceptionCode()))
            {
            }
            if (bytesTransferred == SOCKET_ERROR)
            {
                const int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK)
                    errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
                return i;
            }

            // by definition UDP datagram should be send/recv as a whole or none
            // so amount of bytes should never be different
            if ((uint32_t)bytesTransferred != message.dataLen)
            {
                errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError);
                return i;
            }
        }
        return messagesCount;
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_TCP_Send(Baselib_Socket_Handle socket, void* data, uint32_t dataLen, Baselib_ErrorState* errorState)
    {
        int bytesTransferred = SOCKET_ERROR;
        __try
        {
            bytesTransferred = ::send(
                ::detail::AsNativeType<SOCKET>(socket),
                static_cast<const char*>(data), dataLen,
                0);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }

        if (bytesTransferred == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
                errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return 0;
        }

        return bytesTransferred;
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_UDP_Recv(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        for (uint32_t i = 0; i < messagesCount; ++i)
        {
            auto& message = messages[i];

            detail::socketaddr srcAddress;
            socklen_t srcAddressSize = srcAddress.size();

            int bytesTransferred = SOCKET_ERROR;
            __try
            {
                bytesTransferred = ::recvfrom(
                    ::detail::AsNativeType<SOCKET>(socket),
                    static_cast<char*>(message.data), message.dataLen,
                    0,
                    srcAddress.addr(),
                    &srcAddressSize
                );
            }
            __except (detail::SocketExceptionFilter(GetExceptionCode()))
            {
            }
            if (bytesTransferred == SOCKET_ERROR)
            {
                const int error = WSAGetLastError();

                // posix dictates UDP message overflow should be silently discarded
                if (error == WSAEMSGSIZE)
                {
                    if (message.address && srcAddress.family() != 0)
                    {
                        srcAddress.toBaselib(message.address, errorState);
                        if (Baselib_ErrorState_ErrorRaised(errorState))
                            return i;
                    }

                    continue;
                }

                if (error != WSAEWOULDBLOCK && error != WSAEINVAL /* other platforms treat unbound sockets as WSAEWOULDBLOCK */)
                    errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
                return i;
            }

            message.dataLen = baselib::Algorithm::ClampToType<uint32_t>(bytesTransferred);
            // If `socket` is a TCP socket, `srcAddress` is not written to, and retains its default value 0.
            if (message.address && srcAddress.family() != 0)
            {
                srcAddress.toBaselib(message.address, errorState);
                if (Baselib_ErrorState_ErrorRaised(errorState))
                    return i;
            }
        }
        return messagesCount;
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_TCP_Recv(Baselib_Socket_Handle socket, void* data, uint32_t dataLen, Baselib_ErrorState* errorState)
    {
        int bytesTransferred = SOCKET_ERROR;
        __try
        {
            bytesTransferred = ::recv(
                ::detail::AsNativeType<SOCKET>(socket),
                static_cast<char*>(data), dataLen,
                0);
        }
        __except (detail::SocketExceptionFilter(GetExceptionCode()))
        {
        }

        if (bytesTransferred == 0)
        {
            if (dataLen != 0)
            {
                // connection closed
                errorState |= RaiseError(Baselib_ErrorCode_Disconnected);
            }
            return 0;
        }
        else if (bytesTransferred == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
                errorState |= RaiseError(detail::SockErrorToErrorCode(error)) | WithGetLastError(error);
            return 0;
        }

        return bytesTransferred;
    }
}
