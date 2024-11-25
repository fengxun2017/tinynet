#ifndef _TINYNET_EVENT_LOOP_POOL_H_
#define _TINYNET_EVENT_LOOP_POOL_H_

#include <vector>
#include <memory>
#include <future>
#include <thread>
#include "event_loop.h"

namespace tinynet
{

class EventLoopPool
{
public:
    EventLoopPool(EventLoop *master_loop, std::string name);
    ~EventLoopPool();

    EventLoop *get_next_loop(void);    
    void create_pool(uint8_t pool_size);
private:
    void loop_thread(std::promise<EventLoop *> &promise, std::string name);

    EventLoop *_master_loop;
    uint8_t _pool_size;
    std::string _name;
    std::vector<EventLoop *> _loops;
    std::vector<std::thread> _threads;
};

} // namespace tinynet



#endif // _TINYNET_EVENT_LOOP_POOL_H_