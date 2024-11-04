#ifndef _TINYNET_UTIL_H_
#define _TINYNET_UTIL_H_

#include <string.h> // strerror_r
#include <cerrno>   // errno

namespace tinynet
{
static inline bool check_fd(int fd)
{
    return fd >= 0 ? true : false;
}

const char * error_to_str (int errnum);

void check_fd_nonblock(int fd);

}  // namespace tinynet

#endif  // _TINYNET_UTIL_H_