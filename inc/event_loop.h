#ifndef _TINYNET_EVENT_LOOP_H_
#define _TINYNET_EVENT_LOOP_H_

#include <mutex>
#include <vector>
#include <memory>
#include <thread>
#include "io_poller_interface.h"
#include "wakeup_poller_interface.h"
#include "wakeup_poller.h"

namespace tinynet
{

class EventLoop
{
public:
    using RunInLoopCallBack = std::function<void(void)>;

    // delete copy/move constructor and operator
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;
    EventLoop(EventLoop &&) = delete;
    EventLoop &operator=(EventLoop &&) = delete;

    EventLoop(void);

    /* This constructor is implemented only for the convenience of gtest testing,
       and the upper layer should not use the constructor, 
       but should use the "EventLoop(void)" function to return the object.
    */
    explicit EventLoop(std::shared_ptr<IoPollerInterface> poller, std::unique_ptr<PollerWakeupInterface> poller_wakeup);

    ~EventLoop() {/*FIXME: clear-up work*/}

    void loop();

    void quit();

    bool is_quit(){return _quit;}
    std::shared_ptr<IoPollerInterface> &get_poller(void) {return _poller;}

    bool is_in_loop_thread(void) {return std::this_thread::get_id() == _thread_id;}

    void run_in_loop(RunInLoopCallBack cb, std::string obj_desc);
    
private:
    void exec_pending_cb(void);

    bool _quit;
    std::shared_ptr<IoPollerInterface> _poller;
    std::thread::id _thread_id;
    std::unique_ptr<PollerWakeupInterface> _poller_wakeup;
    std::vector<RunInLoopCallBack> _pending_cb_array;
    std::mutex _pending_array_mutex;
};

} // namespace tinynet

#endif // _TINYNET_EVENT_LOOP_H_