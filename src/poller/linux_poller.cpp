#include <cstddef>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "poller_interface.h"
#include "io_channel_interface.h"
#include "logging.h"
#include "tinynet_util.h"
#include "linux_poller.h"

namespace tinynet {

LinuxPoller::LinuxPoller(void) : 
_event_wait(3)
{
    _poll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (!check_fd(_poll_fd))
    {
        LOG(ERROR) << "epoll_create1 failed in LinuxPoller::LinuxPoller, error info:" 
                << error_to_str(errno)
                << std::endl;
    }
}

LinuxPoller::~LinuxPoller()
{
    if(check_fd(_poll_fd))
    {
        close(_poll_fd);
    }
}

int LinuxPoller::poll(int timeout_ms, IoChannels &active_channels)
{
    int num;
    int ret = 0;

    if(!check_fd(_poll_fd))
    {
        LOG(ERROR) << "check_fd failed in LinuxPoller::poll, _poll_fd = " << _poll_fd << std::endl;
        return -1;
    }

    num = epoll_wait(_poll_fd, _event_wait.data(), _event_wait.size(), timeout_ms);
    if (num > 0)
    {
        for (int index = 0; index < num; index++)
        {
            IoChannelInterface *channel = static_cast<IoChannelInterface *>(_event_wait[index].data.ptr);
            if (nullptr != channel)
            {
                channel->set_events_received(_event_wait[index].events);
                active_channels.push_back(channel);
            }
            else
            {
                LOG(WARNING) << "channel is null in LinuxPoller::poll" << std::endl;
            }
        }

        if (_event_wait.size() == num) 
        {
            LOG(INFO) << "resize _event_wait in LinuxPoller::poll" << std::endl;
            /* The descriptor being listened to may be larger than the size of the _event_wait */
            _event_wait.resize((size_t)(_event_wait.size()*3/2));
        }
    }
    else if (0 == num)
    {
        // LOG(DEBUG) << "epoll_wait timeout" << std::endl;
    }
    else
    {
        LOG(ERROR) << "epoll_wait failed in LinuxPoller::poll, error info:"
                << error_to_str(errno)
                << std::endl;
        ret = -1;
    }

    return ret;
}

int LinuxPoller::add_channel(IoChannelInterface &channel)
{
    return cfg_channel(EPOLL_CTL_ADD, channel);
}

int LinuxPoller::remove_channel(IoChannelInterface &channel)
{
    return cfg_channel(EPOLL_CTL_DEL, channel);
}

int LinuxPoller::update_channel(IoChannelInterface &channel)
{
    return cfg_channel(EPOLL_CTL_MOD, channel);
}

int LinuxPoller::cfg_channel(int op, IoChannelInterface &channel)
{
    struct epoll_event event;
    int ret = 0;

    if(!check_fd(_poll_fd))
    {
        LOG(ERROR) << "check_fd failed in LinuxPoller::cfg_channel, _poll_fd = " << _poll_fd << std::endl;
        return -1;
    }

    event.events = channel.get_events_interested();
    event.data.ptr = &channel;
    if (epoll_ctl(_poll_fd, op, channel.get_fd(), &event) < 0)
    {
        LOG(ERROR) << "epoll_ctl failed in LinuxPoller::cfg_channel, error info:"
                << error_to_str(errno)
                << std::endl;
        ret = -1;
    }

    return ret;
}

}   // namespace tinynet