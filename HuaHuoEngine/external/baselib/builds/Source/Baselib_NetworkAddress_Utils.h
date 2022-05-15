#pragma once

#include "Include/Baselib.h"
#include "Include/C/Baselib_NetworkAddress.h"
#include "Source/Baselib_ErrorState_Utils.h"

#include "Source/ArgumentValidator.h"

#include <ostream>

static inline bool IsValid(Baselib_NetworkAddress_Family family)
{
    switch (family)
    {
        case Baselib_NetworkAddress_Family_IPv4:
        case Baselib_NetworkAddress_Family_IPv6:
            return true;
        default:
            return false;
    }
}

static inline std::ostream& operator<<(std::ostream& os, const Baselib_NetworkAddress_Family& family)
{
    switch (family)
    {
        case Baselib_NetworkAddress_Family_Invalid: os << "invalid"; break;
        case Baselib_NetworkAddress_Family_IPv4: os << "ipv4"; break;
        case Baselib_NetworkAddress_Family_IPv6: os << "ipv6"; break;
        default: os << "<unknown>"; break;
    }
    return os;
}

static inline std::ostream& operator<<(std::ostream& os, const Baselib_NetworkAddress& address)
{
    if (address.family == Baselib_NetworkAddress_Family_Invalid)
        return os << "invalid family network address";

    Baselib_NetworkAddress_Family family;
    char ip[Baselib_NetworkAddress_IpMaxStringLength];
    uint16_t port;

    Baselib_ErrorState errorState = Baselib_ErrorState_Create();
    Baselib_NetworkAddress_Decode(&address, &family, ip, sizeof(ip), &port, &errorState);
    if (Baselib_ErrorState_ErrorRaised(&errorState))
        Baselib_Debug_Break();
    BaselibAssert(Baselib_ErrorState_ErrorRaised(&errorState) == false);

    return os << "network address " << family << " " << ip << ":" << port;
}

static inline std::ostream& operator<<(std::ostream& os, const Baselib_NetworkAddress* address)
{
    if (address)
        return os << *address;
    else
        return os << "nullptr network address";
}
