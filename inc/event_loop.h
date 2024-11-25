#ifndef _TINYNET_EVENT_LOOP_H_
#define _TINYNET_EVENT_LOOP_H_

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

    void loop();

    void quit();

    std::shared_ptr<IoPoller> &get_poller(void) {return _poller;}
    

private:
    bool _quit;
    std::shared_ptr<IoPoller> _poller;
};

} // namespace tinynet



#endif // _TINYNET_EVENT_LOOP_H_