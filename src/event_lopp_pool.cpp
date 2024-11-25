#include <vector>
#include <thread>
#include <cassert>
#include "event_loop_pool.h"
#include "logging.h"

namespace tinynet
{

void EventLoopPool::loop_thread(std::promise<EventLoop *> &promise, std::string name)
{
    EventLoop loop;

    LOG(INFO) << "thread: " << name << " running" << std::endl;
    promise.set_value(&loop);
    loop.loop();
}

void EventLoopPool::create_pool(uint8_t pool_size)
{
    _pool_size = pool_size;
    for (uint8_t index = 0; index < _pool_size; index++)
    {
        std::promise<EventLoop *> promise;
        std::future<EventLoop *> future = promise.get_future();
        std::string sub_name = _name + "_work_thread_" + std::to_string(index);
        std::thread t(&EventLoopPool::loop_thread, this, std::ref(promise), sub_name);
        auto loop = future.get();

        _threads.push_back(std::move(t));
        _loops.push_back(loop);
    }
}

EventLoopPool::EventLoopPool(EventLoop *master_loop, std::string name)
    : _master_loop(master_loop), _pool_size(0), _name(name)
{

}

EventLoopPool::~EventLoopPool()
{
    LOG(DEBUG) << "destruct EventLoopPool:" << _name << std::endl;
    for (auto &loop : _loops)
    {
        loop->quit();
    }

    for (auto &t : _threads)
    {
        t.join();
    }
}

EventLoop* EventLoopPool::get_next_loop(void)
{
    assert(_loops.size() == _pool_size);
    static uint8_t index = 0;
    EventLoop *ret = NULL;

    if (_pool_size > 0)
    {
        LOG(DEBUG) << "get loop:" << index%_pool_size << std::endl;
        ret = _loops[index++%_pool_size];
    }
    else
    {
        LOG(DEBUG) << "get master loop" << std::endl;
        ret = _master_loop;
    }

    return ret;
} 

} // namespace tinynet
