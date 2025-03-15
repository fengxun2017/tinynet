// io_poller.h
#ifndef _TINYNET_IO_POLLER_H_
#define _TINYNET_IO_POLLER_H_

#include <vector>
#include <unordered_map>
#include "io_poller_interface.h"
#include "poller_interface.h"
#include <memory>

namespace tinynet
{

class IoPoller final : public IoPollerInterface
{
public:
    IoPoller(void);

    ~IoPoller() override;

    int poll(int timeout_ms, IoChannels &active_channels) override;
    int add_channel(IoChannelInterface &channel) override;
    int remove_channel(IoChannelInterface &channel) override;
    int update_channel(IoChannelInterface &channel) override;

private:
    std::unique_ptr<PollerInterface> _poller;
};

}  // namespace tinynet

#endif  // _TINYNET_IO_POLLER_H_