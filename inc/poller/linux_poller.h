#ifndef _TINYNET_LINUX_POLLER_H_
#define _TINYNET_LINUX_POLLER_H_

#include <vector>
#include "poller_interface.h"
#include "io_channel_interface.h"

namespace tinynet
{

class LinuxPoller : public PollerInterface
{
public:
    LinuxPoller(void);
    ~LinuxPoller() override;

    int poll(int timeout_ms, IoChannels &active_channels) override;
    int add_channel(IoChannelInterface &channel) override;
    int remove_channel(IoChannelInterface &channel) override;
    int update_channel(IoChannelInterface &channel) override;

private:
    int cfg_channel(int op, IoChannelInterface &channel);

    int _poll_fd;
    std::vector<struct epoll_event> _event_wait;
};

}  // namespace tinynet

#endif  // _TINYNET_LINUX_POLLER_H_