#ifndef _TINYNET_POLLER_INTERFACE_H_
#define _TINYNET_POLLER_INTERFACE_H_

#include "io_channel_interface.h"

namespace tinynet
{

class IoPollerInterface
{
public:
    virtual ~IoPollerInterface() = default;

    virtual int poll(int timeout_ms, IoChannels &active_channels) = 0;
    virtual int add_channel(IoChannelInterface &channel) = 0;
    virtual int remove_channel(IoChannelInterface &channel) = 0;
    virtual int update_channel(IoChannelInterface &channel) = 0;
};

}  // namespace tinynet

#endif  // _TINYNET_POLLER_INTERFACE_H_