#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Socket.h"
#include "Include/Cpp/Algorithm.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_NetworkAddress_Utils.h"
#include "Source/Baselib_Utilities.h"
#include "Source/Posix/ErrorStateBuilder_PosixApi.inl.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

namespace PosixApi
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
        const struct sockaddr*     addr() const {return (const struct sockaddr*)this;}
        const struct sockaddr_in*  in4()  const {return (const struct sockaddr_in*)this;}
        const struct sockaddr_in6* in6()  const {return (const struct sockaddr_in6*)this;}

        uint16_t family() const
        {
            return addr()->sa_family;
        }

        uint16_t port() const
        {
            switch (family())
            {
                case AF_INET:  return in4()->sin_port;
                case AF_INET6: return in6()->sin6_port;
                default:       return 0;
            }
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

    static Baselib_ErrorCode SocketErrorToErrorCode(int error)
    {
        switch (error)
        {
            case EMSGSIZE:
                return Baselib_ErrorCode_InvalidBufferSize;
            case EADDRINUSE:
                return Baselib_ErrorCode_AddressInUse;
            case EADDRNOTAVAIL:
            case ECONNREFUSED:
            case EHOSTDOWN:
            case EHOSTUNREACH:
            case ETIMEDOUT:
                return Baselib_ErrorCode_AddressUnreachable;
            case ECONNRESET:
            case EPIPE:
                return Baselib_ErrorCode_Disconnected;
            case EACCES: // android returns EACCES when calling listen() on a UDP socket
            case EAFNOSUPPORT:
            case EALREADY:
            case EBADF:
            case EDESTADDRREQ:
            case EFAULT:
            case EINVAL:
            case EISCONN:
            case ENETDOWN:
            case ENETUNREACH:
            case ENOTCONN:
            case ENOTSOCK:
            case EOPNOTSUPP:
            case EPERM:
            case EPROTOTYPE:
                // TODO: Positional argument information (Baselib_ErrorState_RaiseInvalidArgument)
                return Baselib_ErrorCode_InvalidArgument;
            default:
                return Baselib_ErrorCode_UnexpectedError;
        }
    }

    static void SetNonblocking(int socketFd, Baselib_ErrorState* errorState)
    {
        const int currentFdFlags = ::fcntl(socketFd, F_GETFL);
        if ((currentFdFlags == -1) || (::fcntl(socketFd, F_SETFL, currentFdFlags | O_NONBLOCK) == -1))
        {
            errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithErrno(errno);
            ::close(socketFd);
        }
    }

    static void SetReuseaddr(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        const int socketFd = ::detail::AsNativeType<int>(socket);
        const int reuseAddress = 1;
        if (::setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) != 0)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }
        #ifdef SO_REUSEPORT
        if (::setsockopt(socketFd, SOL_SOCKET, SO_REUSEPORT, &reuseAddress, sizeof(reuseAddress)) != 0)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }
        #endif
    }
}

    BASELIB_INLINE_IMPL Baselib_Socket_Handle Baselib_Socket_Create(Baselib_NetworkAddress_Family family, Baselib_Socket_Protocol protocol, Baselib_ErrorState* errorState)
    {
        auto addressFamily = family == Baselib_NetworkAddress_Family_IPv4 ? AF_INET : AF_INET6;
        auto socketType = protocol == Baselib_Socket_Protocol_UDP ? SOCK_DGRAM : SOCK_STREAM;
        auto socketProtocol = protocol == Baselib_Socket_Protocol_UDP ? IPPROTO_UDP : IPPROTO_TCP;
        auto socketFd = ::socket(addressFamily, socketType, socketProtocol);
        if (socketFd == -1)
        {
            const int error = errno;
            Baselib_ErrorCode errorCode = error == EAFNOSUPPORT
                ? (Baselib_ErrorCode)Baselib_ErrorCode_AddressFamilyNotSupported
                : (Baselib_ErrorCode)Baselib_ErrorCode_UnexpectedError;
            errorState |= RaiseError(errorCode) | WithErrno(error);
            return Baselib_Socket_Handle_Invalid;
        }

        detail::SetNonblocking(socketFd, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        // to get platform parity we want strict address family separation
        if (family == Baselib_NetworkAddress_Family_IPv6)
        {
            const int ipv6Only = 1;
            if (::setsockopt(socketFd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6Only, sizeof(ipv6Only)) != 0)
            {
                errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithErrno(errno);
                ::close(socketFd);
                return Baselib_Socket_Handle_Invalid;
            }
        }

        return ::detail::AsBaselibHandle<Baselib_Socket_Handle>(socketFd);
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Close(Baselib_Socket_Handle socket)
    {
        if (socket.handle == Baselib_Socket_Handle_Invalid.handle)
            return;

        ::close(::detail::AsNativeType<int>(socket));
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Bind(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        if (addressReuse)
        {
            detail::SetReuseaddr(socket, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return;
        }

        detail::socketaddr posixAddress(address, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        const int socketFd = ::detail::AsNativeType<int>(socket);
        if (::bind(socketFd, posixAddress.addr(), posixAddress.size()) != 0)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_TCP_Connect(Baselib_Socket_Handle socket, const Baselib_NetworkAddress* address, Baselib_NetworkAddress_AddressReuse addressReuse, Baselib_ErrorState* errorState)
    {
        const int socketFd = ::detail::AsNativeType<int>(socket);

        int socketType;
        socklen_t socketTypeLen = sizeof(socketType);
        if (::getsockopt(socketFd, SOL_SOCKET, SO_TYPE, (void*)&socketType, &socketTypeLen) != 0)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
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

        detail::socketaddr posixAddress(address, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;

        if (::connect(socketFd, posixAddress.addr(), posixAddress.size()) != 0)
        {
            const int error = errno;
            // EINPROGRESS => "The socket is nonblocking and the connection cannot be completed immediately."
            if (error != EINPROGRESS)
            {
                errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
                return;
            }
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_Poll(
        Baselib_Socket_PollFd* sockets,
        uint32_t               socketsCount,
        uint32_t               timeoutInMilliseconds,
        Baselib_ErrorState*    errorState)
    {
        // TODO: This should probably be a temp alloc.
        std::unique_ptr<struct pollfd[]> pollFds(new struct pollfd[socketsCount]);
        for (size_t i = 0; i < socketsCount; i++)
        {
            pollFds[i].fd = ::detail::AsNativeType<int>(sockets[i].handle);
            pollFds[i].events = 0;
            pollFds[i].revents = 0;
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Readable)
            {
                pollFds[i].events |= POLLIN;
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Writable || sockets[i].requestedEvents & Baselib_Socket_PollEvents_Connected)
            {
                pollFds[i].events |= POLLOUT;
            }
        }

        if (::poll(pollFds.get(), socketsCount, (int)timeoutInMilliseconds) == -1)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }

        for (size_t i = 0; i < socketsCount; i++)
        {
            if (pollFds[i].revents & POLLERR)
            {
                int socketErr;
                socklen_t len = sizeof(socketErr);
                if (::getsockopt(pollFds[i].fd, SOL_SOCKET, SO_ERROR, (void*)&socketErr, &len) != 0)
                {
                    const int error = errno;
                    errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
                    return;
                }
                sockets[i].errorState |= RaiseError(detail::SocketErrorToErrorCode(socketErr)) | WithErrno(socketErr);
            }
            else if (pollFds[i].revents & POLLHUP)
            {
                sockets[i].errorState |= RaiseError(Baselib_ErrorCode_Disconnected);
            }
            else if (pollFds[i].revents & ~(POLLIN | POLLOUT))
            {
                // POLLNVAL
                sockets[i].errorState |= RaiseInvalidArgument(sockets);
            }

            sockets[i].resultEvents = (Baselib_Socket_PollEvents)0;
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Readable && (pollFds[i].revents & POLLIN) != 0)
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Readable);
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Writable && (pollFds[i].revents & POLLOUT) != 0)
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Writable);
            }
            if (sockets[i].requestedEvents & Baselib_Socket_PollEvents_Connected && (pollFds[i].revents & POLLOUT) != 0 && !Baselib_ErrorState_ErrorRaised(sockets[i].errorState))
            {
                sockets[i].resultEvents = (Baselib_Socket_PollEvents)(sockets[i].resultEvents | Baselib_Socket_PollEvents_Connected);
            }
        }
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_GetAddress(Baselib_Socket_Handle socket, Baselib_NetworkAddress* address, Baselib_ErrorState* errorState)
    {
        const int socketFd = ::detail::AsNativeType<int>(socket);
        detail::socketaddr posixAddress;
        socklen_t size = posixAddress.size();
        if (::getsockname(socketFd, posixAddress.addr(), &size) != 0)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }

        // check if socket was unbound
        if (posixAddress.port() == 0)
        {
            errorState |= RaiseInvalidArgument(socket);
        }

        posixAddress.toBaselib(address, errorState);
    }

    BASELIB_INLINE_IMPL void Baselib_Socket_TCP_Listen(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        int result = ::listen(::detail::AsNativeType<int>(socket), SOMAXCONN);
        if (result == -1)
        {
            const int error = errno;
            errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return;
        }
    }

    BASELIB_INLINE_IMPL Baselib_Socket_Handle Baselib_Socket_TCP_Accept(Baselib_Socket_Handle socket, Baselib_ErrorState* errorState)
    {
        int result = ::accept(::detail::AsNativeType<int>(socket), NULL, NULL);
        if (result == -1)
        {
            const int error = errno;
            if (error != EAGAIN && error != EWOULDBLOCK)
                errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return Baselib_Socket_Handle_Invalid;
        }

        detail::SetNonblocking(result, errorState);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return Baselib_Socket_Handle_Invalid;

        return ::detail::AsBaselibHandle<Baselib_Socket_Handle>(result);
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_UDP_Send(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        const int socketFd = ::detail::AsNativeType<int>(socket);
        for (uint32_t i = 0; i < messagesCount; ++i)
        {
            const auto& message = messages[i];

            detail::socketaddr socketAddr(message.address, errorState);
            if (Baselib_ErrorState_ErrorRaised(errorState))
                return i;

            ssize_t bytesTransferred = ::sendto(
                socketFd,
                message.data, message.dataLen,
                0,
                socketAddr.addr(), socketAddr.size()
            );

            if (bytesTransferred < 0)
            {
                const int error = errno;
                if (error != EAGAIN && error != EWOULDBLOCK)
                    errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
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
        const int socketFd = ::detail::AsNativeType<int>(socket);
        ssize_t bytesTransferred = ::send(socketFd, data, dataLen, 0);

        if (bytesTransferred < 0)
        {
            const int error = errno;
            if (error != EAGAIN && error != EWOULDBLOCK)
                errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return 0;
        }

        return (uint32_t)bytesTransferred;
    }

    BASELIB_INLINE_IMPL uint32_t Baselib_Socket_UDP_Recv(Baselib_Socket_Handle socket, Baselib_Socket_Message* messages, uint32_t messagesCount, Baselib_ErrorState* errorState)
    {
        const int socketFd = ::detail::AsNativeType<int>(socket);
        for (uint32_t i = 0; i < messagesCount; ++i)
        {
            auto& message = messages[i];

            // We can't determine whether we received a zero size packet or nothing at all
            // if there is no address written because on BSD there is no EAGAIN in this case.
            // In thise case we use srcAddressSize to check if anything was read from the socket.
            bool zeroSizedPacket = (message.data == nullptr) || (message.dataLen == 0);

            detail::socketaddr srcAddress;
            socklen_t srcAddressSize = srcAddress.size();
            const ssize_t bytesTransferred = ::recvfrom(
                socketFd,
                message.data, message.dataLen,
                0,
                srcAddress.addr(),
                &srcAddressSize
            );

            if (bytesTransferred == -1)
            {
                const int error = errno;
                if (error != EAGAIN && error != EWOULDBLOCK)
                    errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
                return i;
            }

            if (bytesTransferred == 0 && zeroSizedPacket && srcAddressSize == 0)
                return i;

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
        const int socketFd = ::detail::AsNativeType<int>(socket);
        ssize_t bytesTransferred = ::recv(socketFd, data, dataLen, 0);

        if (bytesTransferred == 0)
        {
            if (dataLen != 0)
            {
                // connection closed
                errorState |= RaiseError(Baselib_ErrorCode_Disconnected);
            }
            return 0;
        }
        else if (bytesTransferred < 0)
        {
            const int error = errno;
            if (error != EAGAIN && error != EWOULDBLOCK)
                errorState |= RaiseError(detail::SocketErrorToErrorCode(error)) | WithErrno(error);
            return 0;
        }

        return (uint32_t)bytesTransferred;
    }
}
