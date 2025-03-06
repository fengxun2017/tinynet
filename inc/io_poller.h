// io_poller.h
#ifndef _TINYNET_IO_POLLER_H_
#define _TINYNET_IO_POLLER_H_

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "io_poller_interface.h"

namespace tinynet
{

class IoPoller final : public IoPollerInterface
{
public:
    IoPoller(void);
    ~IoPoller() override;

    void poll(int timeout_ms, IoChannels &active_channels) override;
    void add_channel(IoChannelInterface &channel) override;
    void remove_channel(IoChannelInterface &channel) override;
    void update_channel(IoChannelInterface &channel) override;

private:
    int cfg_channel(int op, IoChannelInterface &channel);
    int _poll_fd;
    std::unordered_map<int, IoChannelInterface *> channel_map;
    std::vector<struct epoll_event> _event_wait;
};

}  // namespace tinynet

#endif  // _TINYNET_IO_POLLER_H_