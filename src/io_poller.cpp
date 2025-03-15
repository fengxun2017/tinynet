#include <cstddef>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "io_poller.h"
#include "io_channel_interface.h"
#include "logging.h"
#include "tinynet_util.h"
#include "poller_factory.h"

namespace tinynet
{

IoPoller::IoPoller(void) 
{
    _poller = PollerFactory::create_poller();
}

IoPoller::~IoPoller()
{
    // Poller 实例的析构由 unique_ptr 自动处理
}

int IoPoller::poll(int timeout_ms, IoChannels &active_channels)
{
    return _poller->poll(timeout_ms, active_channels);
}

int IoPoller::add_channel(IoChannelInterface &channel)
{
    LOG(INFO) << "add channel: " <<channel.get_name() << std::endl;
    return _poller->add_channel(channel);
}

int IoPoller::remove_channel(IoChannelInterface &channel)
{
    LOG(INFO) << "remove channel: " <<channel.get_name() << std::endl;
    return _poller->remove_channel(channel);
}

int IoPoller::update_channel(IoChannelInterface &channel)
{
    LOG(INFO) << "update channel: " <<channel.get_name() << std::endl;
    return _poller->update_channel(channel);
}

}  // namespace tinynet