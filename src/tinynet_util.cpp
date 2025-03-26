#include <cerrno>
#include <fcntl.h>
#include <string.h> // strerror_s
#include "tinynet_util.h"
#include "logging.h"

namespace tinynet
{

thread_local char error_str[512];

const char * error_to_str (int errnum)
{
    return strerror_r(errnum, error_str, sizeof(error_str));

}

void check_fd_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        LOG(ERROR) << "fcntl failed, err info: " << error_to_str(errno) << std::endl;
    }
    else
    {
        if (!(flags & O_NONBLOCK)) {
            LOG(ERROR) << "check fd nonblock failed!" << std::endl;
        }
    }
}

const char* errno_str(int _errno)
{
    const char *ret;
    switch(_errno)
    {
        case EACCES:
            ret = "EACCES";
            break;
        case EPERM:
            ret = "EPERM";
            break;
        case EADDRINUSE:
            ret = "EADDRINUSE";
            break;
        case EADDRNOTAVAIL:
            ret = "EADDRNOTAVAIL";
            break;
        case EAFNOSUPPORT:
            ret = "EAFNOSUPPORT";
            break;
        case EAGAIN:
            ret = "EAGAIN";
            break;
        case EALREADY:
            ret = "EALREADY";
            break;
        case EBADF:
            ret = "EBADF";
            break;
        case ECONNREFUSED:
            ret = "ECONNREFUSED";
            break;
        case EFAULT:
            ret = "EFAULT";
            break;
        case EINPROGRESS:
            ret = "EINPROGRESS";
            break;
        case EINTR:
            ret = "EINTR";
            break;
        case EISCONN:
            ret = "EISCONN";
            break;
        case ENETUNREACH:
            ret = "ENETUNREACH";
            break;
        case ENOTSOCK:
            ret = "ENOTSOCK";
            break;
        case EPROTOTYPE:
            ret = "EPROTOTYPE";
            break;
        case ETIMEDOUT:
            ret = "ETIMEDOUT";
            break;
        default:
            ret = "NULL";
            break;
    }

    return ret;
}

} // namespace tinynet