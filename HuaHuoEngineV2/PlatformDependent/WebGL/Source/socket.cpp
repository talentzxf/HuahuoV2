#include "UnityPrefix.h"
#include "PlatformDependent/WebGL/Source/socket.h"

int setsockopt_stub(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return 0;
}

int shutdown_stub(int fd, int how)
{
    return 0;
}
