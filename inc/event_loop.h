#ifndef _TINYNET_EVENT_POOL_H_
#define _TINYNET_EVENT_POOL_H_

#include <vector>
#include <memory>
#include "io_channel.h"
#include "io_poller.h"

namespace tinynet
{

class EventLoop
{
public:
    EventLoop(void);
    ~EventLoop() {}

    void loop() ;

    void quit();

    void add_channel(IoChannel* channel);

    void remove_channel(IoChannel* channel);

    void update_channel(IoChannel* channel);

private:
    bool _quit = false;
    std::shared_ptr<IoPoller> _poller;
};

} // namespace tinynet



#endif // _TINYNET_EVENT_POOL_H_