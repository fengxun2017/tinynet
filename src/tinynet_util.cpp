#include <fcntl.h>
#include "tinynet_util.h"
#include "logging.h"

namespace tinynet
{

thread_local char error_str[512];

const char * error_to_str (int errnum)
{
    if(strerror_s(errnum, error_str, sizeof(error_str)) !=0)
    {
        LOG(ERROR) << "error_to_str failed" << std::endl;
        return "None";
    }

    return error_str;
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