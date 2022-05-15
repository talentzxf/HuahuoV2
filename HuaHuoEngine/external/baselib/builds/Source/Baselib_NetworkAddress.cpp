#include "Include/Baselib.h"
#include "Include/C/Baselib_NetworkAddress.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Baselib_ErrorState_Utils.h"
#include "Baselib_NetworkAddress_Utils.h"
#include "ArgumentValidator.h"

namespace detail
{
    //
    // Adopted from musl (http://git.musl-libc.org)
    // 2019-04-11 - c442f5e77fe87952bb51a958f830f9308419804e
    //
    static const char *baselib_inet_ntop(Baselib_NetworkAddress_Family af, const void* a0, char* s, size_t l)
    {
        const uint8_t *a = (uint8_t*)a0;
        size_t i, j, max, best;
        char buf[100];

        switch (af)
        {
            case Baselib_NetworkAddress_Family_IPv4:
                if ((size_t)snprintf(s, l, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]) < l)
                    return s;
                break;
            case Baselib_NetworkAddress_Family_IPv6:
                if (memcmp(a, "\0\0\0\0\0\0\0\0\0\0\377\377", 12))
                    snprintf(buf, sizeof buf,
                        "%x:%x:%x:%x:%x:%x:%x:%x",
                        256 * a[0] + a[1], 256 * a[2] + a[3],
                        256 * a[4] + a[5], 256 * a[6] + a[7],
                        256 * a[8] + a[9], 256 * a[10] + a[11],
                        256 * a[12] + a[13], 256 * a[14] + a[15]);
                else
                    snprintf(buf, sizeof buf,
                        "%x:%x:%x:%x:%x:%x:%d.%d.%d.%d",
                        256 * a[0] + a[1], 256 * a[2] + a[3],
                        256 * a[4] + a[5], 256 * a[6] + a[7],
                        256 * a[8] + a[9], 256 * a[10] + a[11],
                        a[12], a[13], a[14], a[15]);
                /* Replace longest /(^0|:)[:0]{2,}/ with "::" */
                for (i = best = 0, max = 2; buf[i]; i++)
                {
                    if (i && buf[i] != ':') continue;
                    j = strspn(buf + i, ":0");
                    if (j > max) best = i, max = j;
                }
                if (max > 3)
                {
                    buf[best] = buf[best + 1] = ':';
                    memmove(buf + best + 2, buf + best + max, i - best - max + 1);
                }
                if (strlen(buf) < l)
                {
                    strcpy(s, buf);
                    return s;
                }
                break;
            case Baselib_NetworkAddress_Family_Invalid:
                return 0;
        }
        return 0;
    }

    static int baselib_hexval(unsigned c)
    {
        if (c - '0' < 10) return c - '0';
        c |= 32;
        if (c - 'a' < 6) return c - 'a' + 10;
        return -1;
    }

    static int baselib_inet_pton(Baselib_NetworkAddress_Family af, const char* s, void* a0)
    {
        uint16_t ip[8];
        uint8_t* a = (uint8_t *)(a0);
        int i, j, v, d, brk = -1, need_v4 = 0;

        if (af == Baselib_NetworkAddress_Family_IPv4)
        {
            for (i = 0; i < 4; i++)
            {
                for (v = j = 0; j < 3 && isdigit(s[j]); j++)
                    v = 10 * v + s[j] - '0';
                if (j == 0 || (j > 1 && s[0] == '0') || v > 255) return 0;
                a[i] = v;
                if (s[j] == 0 && i == 3) return 1;
                if (s[j] != '.') return 0;
                s += j + 1;
            }
            return 0;
        }
        else if (af != Baselib_NetworkAddress_Family_IPv6)
        {
            return -1;
        }

        if (*s == ':' && *++s != ':') return 0;

        for (i = 0;; i++)
        {
            if (s[0] == ':' && brk < 0)
            {
                brk = i;
                ip[i & 7] = 0;
                if (!*++s) break;
                if (i == 7) return 0;
                continue;
            }
            for (v = j = 0; j < 4 && (d = baselib_hexval(s[j])) >= 0; j++)
                v = 16 * v + d;
            if (j == 0) return 0;
            ip[i & 7] = v;
            if (!s[j] && (brk >= 0 || i == 7)) break;
            if (i == 7) return 0;
            if (s[j] != ':')
            {
                if (s[j] != '.' || (i < 6 && brk < 0)) return 0;
                need_v4 = 1;
                i++;
                break;
            }
            s += j + 1;
        }
        if (brk >= 0)
        {
            memmove(ip + brk + 7 - i, ip + brk, 2 * (i + 1 - brk));
            for (j = 0; j < 7 - i; j++)
                ip[brk + j] = 0;
        }
        for (j = 0; j < 8; j++)
        {
            *a++ = ip[j] >> 8;
            *a++ = ip[j] & 0xff;
        }
        if (need_v4 && baselib_inet_pton(Baselib_NetworkAddress_Family_IPv4, s, a - 4) <= 0) return 0;
        return 1;
    }

    static inline uint16_t baselib_bswap_16(uint16_t __x)
    {
        return __x << 8 | __x >> 8;
    }

    static uint16_t baselib_htons(uint16_t n)
    {
        union { int i; char c; } u = { 1 };
        return u.c ? baselib_bswap_16(n) : n;
    }

    static uint16_t baselib_ntohs(uint16_t n)
    {
        union { int i; char c; } u = { 1 };
        return u.c ? baselib_bswap_16(n) : n;
    }
}

#ifdef __cplusplus
BASELIB_C_INTERFACE
{
#endif

void Baselib_NetworkAddress_Encode(
    Baselib_NetworkAddress*       dstAddress,
    Baselib_NetworkAddress_Family family,
    const char*                   ip,
    uint16_t                      port,
    Baselib_ErrorState*           errorState
)
{
    errorState |= Validate(AsPointer(dstAddress));
    if (Baselib_ErrorState_ErrorRaised(errorState))
        return;

    *dstAddress = Baselib_NetworkAddress_Empty();

    errorState |= Validate(AsPointer(ip));
    errorState |= Validate(family);
    if (Baselib_ErrorState_ErrorRaised(errorState))
        return;

    if ((ip != nullptr) && (detail::baselib_inet_pton(family, ip, (void*)dstAddress->data) != 1))
    {
        errorState |= RaiseInvalidArgument(ip);
        return;
    }

    static_assert(sizeof(dstAddress->port) == sizeof(uint16_t), "ensure port is just exactly two bytes");
    uint16_t portn = detail::baselib_htons(port);
    memcpy(dstAddress->port, (uint8_t*)&portn, sizeof(uint16_t));

    dstAddress->family = (uint8_t)family;
}

void Baselib_NetworkAddress_Decode(
    const Baselib_NetworkAddress*  srcAddress,
    Baselib_NetworkAddress_Family* family,
    char*                          ipAddressBuffer,
    uint32_t                       ipAddressBufferLen,
    uint16_t*                      port,
    Baselib_ErrorState*            errorState
)
{
    errorState |= Validate(AsPointer(srcAddress));
    if (Baselib_ErrorState_ErrorRaised(errorState))
        return;

    if (ipAddressBuffer && ipAddressBufferLen > 0)
    {
        if (srcAddress->family == Baselib_NetworkAddress_Family_Invalid)
            errorState |= RaiseInvalidArgument(srcAddress);
        else if (detail::baselib_inet_ntop((Baselib_NetworkAddress_Family)srcAddress->family, (const void*)srcAddress->data, ipAddressBuffer, ipAddressBufferLen) == nullptr)
            errorState |= RaiseError(Baselib_ErrorCode_InvalidBufferSize);
    }

    if (port)
    {
        static_assert(sizeof(*port) == sizeof(srcAddress->port), "ensure same size");
        memcpy(port, srcAddress->port, sizeof(srcAddress->port));
        *port = detail::baselib_ntohs(*port);
    }

    if (family)
        *family = (Baselib_NetworkAddress_Family)srcAddress->family;
}

#ifdef __cplusplus
}
#endif
