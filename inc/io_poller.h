#ifndef _TINYNET_POLLER_H_
#define _TINYNET_POLLER_H_

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "io_channel.h"

namespace tinynet
{

class IoPoller
{
public:
    typedef std::vector<IoChannel*> Channels;

    IoPoller(void);
    ~IoPoller() ;

    void poll(int timeout_ms, Channels &active_channels);
    void add_channel(IoChannel &channel);
    void remove_channel(IoChannel &channel);
    void update_channel(IoChannel &channel);
private:

    int cfg_channel(int op, IoChannel &channel);

    int _poll_fd;
     std::unordered_map<int, IoChannel *> channel_map;
    std::vector<struct epoll_event> _event_wait(10);
};

}  // namespace tinynet
#endif  // _TINYNET_POLLER_H_