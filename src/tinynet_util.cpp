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

} // namespace tinynet