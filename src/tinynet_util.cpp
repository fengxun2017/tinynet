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

} // namespace tinynet