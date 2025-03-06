#include <cstdlib>
#include <mutex>
#include <errno.h>
#include <vector>
#include "event_loop.h"
#include "logging.h"
#include "tinynet_util.h"
#include "io_poller.h"
#include "wakeup_poller.h"

namespace tinynet
{

EventLoop::EventLoop(void) 
    : _quit(false),
    _poller(std::make_shared<IoPoller>()),
    _poller_wakeup(std::make_unique<PollerWakeup>(_poller, "eventloop_wakeup_channel"))

{
    _thread_id = std::this_thread::get_id();
    LOG(DEBUG) << "event loop created in thread:" << _thread_id << std::endl;
}

void EventLoop::run_in_loop(RunInLoopCallBack cb, std::string obj_desc)
{
    if (is_in_loop_thread())
    {
        LOG(DEBUG) << obj_desc << " runs directly in the current thread." << std::endl;
        cb();
    }
    else
    {
        LOG(DEBUG) << obj_desc << " will run in thread: " << _thread_id << std::endl;
        {
            std::lock_guard<std::mutex> lock(_pending_array_mutex);
            _pending_cb_array.push_back(cb);
        }
        
        if(!is_in_loop_thread())
        {
            _poller_wakeup->wakeup();
        }
    }
}

void EventLoop::exec_pending_cb(void)
{
    std::vector<RunInLoopCallBack> tmp_array;

    {
        std::lock_guard<std::mutex> lock(_pending_array_mutex);
        _pending_cb_array.swap(tmp_array);
    }

    for (auto &cb : tmp_array)
    {
        LOG(DEBUG) << "in thread: " << _thread_id << " exec pending function" << std::endl;
        cb();
    }
}

void EventLoop::loop()
{
    if (!is_in_loop_thread())
    {
        LOG(DEBUG) << "event loop run in thread:" << std::this_thread::get_id() << std::endl;
        LOG(ERROR) << "EventLoop must be created and run in the same thread!!" << std::endl;
        std::abort();
    }

    IoChannels active_channels;
    while (!_quit)
    {
        // 1 second timeout
        _poller->poll(1000, active_channels); 

        for (auto channel : active_channels) {
            channel->handle_event();
        }
        active_channels.clear();
        exec_pending_cb();
    }
}

void EventLoop::quit()
{
    _quit = true;
}

} // namespace tinynet