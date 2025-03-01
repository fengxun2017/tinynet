#ifndef _TINYNET_EVENT_LOOP_H_
#define _TINYNET_EVENT_LOOP_H_

#include <mutex>
#include <vector>
#include <memory>
#include <thread>
#include "io_channel.h"
#include "io_poller.h"

namespace tinynet
{

class EventLoop
{
public:
    using RunInLoopCallBack = std::function<void(void)> ;


    EventLoop(void);
    ~EventLoop() {/*FIXME: clear-up work*/}

    void loop();

    void quit();

    bool is_quit(){return _quit;}
    std::shared_ptr<IoPoller> &get_poller(void) {return _poller;}

    bool is_in_loop_thread(void) {return std::this_thread::get_id() == _thread_id;}

    void wakeup_loop(void);

    void run_in_loop(RunInLoopCallBack cb, std::string obj_desc);
    
private:
    int create_eventfd(void);
    void handle_recv(void);
    void exec_pending_cb(void);

    bool _quit;
    std::shared_ptr<IoPoller> _poller;
    std::thread::id _thread_id;
    int _event_fd;
    IoChannel _channel;
    std::vector<RunInLoopCallBack> _pending_cb_array;
    std::mutex _pending_array_mutex;
};

} // namespace tinynet



#endif // _TINYNET_EVENT_LOOP_H_