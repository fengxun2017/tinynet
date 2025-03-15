// event_loop_test.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "event_loop.h"
#include "io_poller_interface.h"
#include "io_channel_interface.h"
#include "wakeup_poller_interface.h"
#include "wakeup_poller.h"
#include <thread>
#include <functional>
#include <memory>
#include <condition_variable>
#include <mutex>

using namespace tinynet;
using namespace testing;

// Mock IoPollerInterface
class MockIoPoller : public IoPollerInterface {
public:
    MOCK_METHOD2(poll, int(int, IoChannels&));
    MOCK_METHOD1(add_channel, int(IoChannelInterface&));
    MOCK_METHOD1(remove_channel, int(IoChannelInterface&));
    MOCK_METHOD1(update_channel, int(IoChannelInterface&));
};

// Mock IoChannelInterface
class MockIoChannel : public IoChannelInterface {
public:
    MOCK_METHOD1(set_read_callback, void(ChannelEventCallback));
    MOCK_METHOD1(set_write_callback, void(ChannelEventCallback));
    MOCK_METHOD1(set_close_callback, void(ChannelEventCallback));
    MOCK_METHOD1(set_error_callback, void(ChannelEventCallback));
    MOCK_METHOD0(handle_event, void());
    MOCK_METHOD0(enable_read, void());
    MOCK_METHOD0(enable_write, void());
    MOCK_METHOD0(disable_read, void());
    MOCK_METHOD0(disable_write, void());
    MOCK_METHOD0(disable_all, void());
    MOCK_METHOD0(is_writing, bool());
    MOCK_METHOD0(get_fd, int());
    MOCK_METHOD1(set_events_received, void(int));
    MOCK_METHOD0(get_name, std::string());
    MOCK_METHOD0(get_events_interested, uint32_t());
    MOCK_METHOD0(get_events_received, uint32_t());
};

// Mock PollerWakeupInterface
class MockPollerWakeup : public PollerWakeupInterface {
public:
    MOCK_METHOD0(wakeup, void());
};

// Test Fixture
class EventLoopTest : public Test {
public:
    void SetUp() override {
        _mock_poller = std::make_shared<MockIoPoller>();
        auto wakeup = std::make_unique<MockPollerWakeup>();
        _mock_wakeup = wakeup.get(); // 保存原始指针
        _event_loop = std::make_unique<EventLoop>(_mock_poller, std::move(wakeup));
    }

    // 使用原子计数器实现简单信号量
    std::atomic<int> sem_poll{0};

    std::shared_ptr<MockIoPoller> _mock_poller;
    MockPollerWakeup* _mock_wakeup; // 原始指针观察
    std::unique_ptr<EventLoop> _event_loop;
};

// 实现 poll 阻塞行为
ACTION_P(BlockUntilWake, test_fixture) {
    // 等待唤醒信号
    while(test_fixture->sem_poll.load() == 0) {
        std::this_thread::yield();
    }
    test_fixture->sem_poll--;

    // 模拟返回空事件列表
    IoChannels& channels = arg1;
    channels.clear();

    return 0;
}

// 实现 wakeup 唤醒行为
ACTION_P(NotifyWake, test_fixture) {
    // 发送唤醒信号
    test_fixture->sem_poll++;
}

// Test Case: Test loop method
TEST_F(EventLoopTest, LoopShouldRunInCorrectThread) {
    std::cout << "LoopShouldRunInCorrectThread" << std::endl;
    EXPECT_EQ(_event_loop->is_in_loop_thread(), true);
}

// Test Case: Test run_in_loop method when in the same thread
TEST_F(EventLoopTest, RunInLoopShouldExecuteCallbackDirectly) {
    bool callback_called = false;
    std::cout << "RunInLoopShouldExecuteCallbackDirectly" << std::endl;

    _event_loop->run_in_loop([&callback_called]() {
        callback_called = true;
    }, "test_callback");

    EXPECT_EQ(callback_called, true);
}

// Test Case: Test run_in_loop method when in a different thread
TEST_F(EventLoopTest, RunInLoopShouldQueueCallback) {
    bool callback_called = false;
    std::cout << "RunInLoopShouldQueueCallback" << std::endl;
    
    EXPECT_CALL(*_mock_poller, poll(_, _))
        .WillOnce(BlockUntilWake(this));
    EXPECT_CALL(*_mock_wakeup, wakeup())
            .WillOnce(NotifyWake(this));
    std::thread other_thread([this, &callback_called]() {
        _event_loop->run_in_loop([this, &callback_called]() {
            callback_called = true;
            //这里需要设置退出条件，否则下面的event_loop->loop()会在内部一直循环
            _event_loop->quit();
        }, "test_callback");
    });

    // Simulate event loop processing
    _event_loop->loop();
    // Wait for the other thread to finish
    other_thread.join();

    EXPECT_EQ(callback_called, true);
}

// Test Case: Test quit method
TEST_F(EventLoopTest, QuitShouldStopLoop) {
    std::cout << "QuitShouldStopLoop" << std::endl;
    bool loop_running = true;
    EXPECT_CALL(*_mock_poller, poll(_, _)).Times(AnyNumber());
    std::thread loop_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        loop_running = false;
        _event_loop->quit();
    });
    _event_loop->loop();

    // Wait for the loop to stop
    loop_thread.join();

    EXPECT_EQ(loop_running, false);
}

// Test Case: Test add_channel method
TEST_F(EventLoopTest, AddChannelShouldCallPollerAddChannel) {
    std::cout << "AddChannelShouldCallPollerAddChannel" << std::endl;

    MockIoChannel mock_channel;
    EXPECT_CALL(*_mock_poller, add_channel(Ref(mock_channel))).Times(1);

    _event_loop->get_poller()->add_channel(mock_channel);
}

// Test Case: Test remove_channel method
TEST_F(EventLoopTest, RemoveChannelShouldCallPollerRemoveChannel) {
    std::cout << "RemoveChannelShouldCallPollerRemoveChannel" << std::endl;
    MockIoChannel mock_channel;
    EXPECT_CALL(*_mock_poller, remove_channel(Ref(mock_channel))).Times(1);

    _event_loop->get_poller()->remove_channel(mock_channel);
}

// Test Case: Test update_channel method
TEST_F(EventLoopTest, UpdateChannelShouldCallPollerUpdateChannel) {
    std::cout << "UpdateChannelShouldCallPollerUpdateChannel" << std::endl;
    MockIoChannel mock_channel;
    EXPECT_CALL(*_mock_poller, update_channel(Ref(mock_channel))).Times(1);

    _event_loop->get_poller()->update_channel(mock_channel);
}