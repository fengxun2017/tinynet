#include <cstddef>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "io_poller.h"
#include "io_channel_interface.h"
#include "logging.h"
#include "tinynet_util.h"

namespace tinynet
{

IoPoller::IoPoller(void) 
    :_event_wait(3)
{
    _poll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (!check_fd(_poll_fd))
    {
        LOG(ERROR) << "epoll_create1 failed in IoPoller::IoPoller, error info:" 
                << error_to_str(errno)
                << std::endl;
    }
}

IoPoller::~IoPoller()
{
    if(check_fd(_poll_fd))
    {
        close(_poll_fd);
    }
}

void IoPoller::poll(int timeout_ms, IoChannels &active_channels)
{
    int num;

    if(!check_fd(_poll_fd))
    {
        return ;
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
                LOG(WARNING) << "channel is null in IoPoller::poll" << std::endl;
            }
        }

        if (_event_wait.size() == num) 
        {
            LOG(INFO) << "resize _event_wait in IoPoller::poll" << std::endl;
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
        LOG(ERROR) << "epoll_wait failed in IoPoller::poll, error info:"
                << error_to_str(errno)
                << std::endl;
    }
}

int IoPoller::cfg_channel(int op, IoChannelInterface &channel)
{
    struct epoll_event event;
    int ret = 0;

    if(!check_fd(_poll_fd))
    {
        return -1;
    }

    event.events = channel.get_events_interested();
    event.data.ptr = &channel;
    if (0 > epoll_ctl(_poll_fd, op, channel.get_fd(), &event))
    {
        LOG(ERROR) << "epoll_ctl failed in IoPoller::update_channel, error info:"
                << error_to_str(errno)
                << " _poll_fd=" << _poll_fd << " fd=" << channel.get_fd()
                << std::endl;
        ret = -1;
    }

    return ret;
}

void IoPoller::add_channel(IoChannelInterface &channel)
{
    int ret;
    LOG(INFO) << channel.get_name() << ":EPOLL_CTL_ADD" << std::endl;
    ret = cfg_channel(EPOLL_CTL_ADD, channel);
    if (0 != ret)
    {
        LOG(ERROR) << "IoPoller::add_channel failed" << std::endl;
    }
}

void IoPoller::remove_channel(IoChannelInterface &channel)
{
    int ret;
    LOG(INFO) << channel.get_name() << ":EPOLL_CTL_DEL" << std::endl;
    ret = cfg_channel(EPOLL_CTL_DEL, channel);
    if (0 != ret)
    {
        LOG(ERROR) << "IoPoller::remove_channel failed" << std::endl;
    }
}

void IoPoller::update_channel(IoChannelInterface &channel)
{
    int ret;
    LOG(INFO) << channel.get_name() << ":EPOLL_CTL_MOD" << std::endl;
    ret = cfg_channel(EPOLL_CTL_MOD, channel);
    if (0 != ret)
    {
        LOG(ERROR) << "IoPoller::update_channel failed" << std::endl;
    }
}

}  // namespace tinynet