#ifndef _TINYNET_EVENT_POOL_H_
#define _TINYNET_EVENT_POOL_H_


#include "event_loop.h"

namespace tinynet
{

EventLoop::EventLoop(void)
{
    _poller = std::make_shared<IoPoller>()
}

void EventLoop::loop()
{
    while (!_quit)
    {
        Channels active_channels;
        _poller->poll(1000, active_channels); // 1 second timeout

        for (auto channel : active_channels) {
            channel->handle_event();
        }
    }
}

void EventLoop::quit()
{
    _quit = true;
}

void EventLoop::add_channel(IoChannel* channel)
{
    _poller->add_channel(*channel);
}

void EventLoop::remove_channel(IoChannel* channel)
{
    _poller->remove_channel(*channel);
}

void EventLoop::update_channel(IoChannel* channel)
{
    _poller->update_channel(*channel);
}

} // namespace tinynet



#endif // _TINYNET_EVENT_POOL_H_