#pragma once

#include <sys/socket.h>

// setsockopt is not implemented and will fail with error: Protocol not available(92)
#define setsockopt setsockopt_stub
// shutdown is unsupported
#define shutdown shutdown_stub

int setsockopt_stub(int fd, int level, int optname, const void *optval, socklen_t optlen);
int shutdown_stub(int fd, int how);
