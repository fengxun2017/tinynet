
#include "event_loop.h"

namespace tinynet
{

EventLoop::EventLoop(void) : _quit(false)
{
    _poller = std::make_shared<IoPoller>();
}

void EventLoop::loop()
{
    while (!_quit)
    {
        Channels active_channels;
        // 1 second timeout
        _poller->poll(1000, active_channels); 

        for (auto channel : active_channels) {
            channel->handle_event();
        }
    }
}

void EventLoop::quit()
{
    _quit = true;
}


} // namespace tinynet