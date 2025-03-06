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

using namespace tinynet;
using namespace testing;

// Mock IoPollerInterface
class MockIoPoller : public IoPollerInterface {
public:
    MOCK_METHOD2(poll, void(int, IoChannels&));
    MOCK_METHOD1(add_channel, void(IoChannelInterface&));
    MOCK_METHOD1(remove_channel, void(IoChannelInterface&));
    MOCK_METHOD1(update_channel, void(IoChannelInterface&));
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
protected:
    void SetUp() override {
        mock_poller = std::make_shared<MockIoPoller>();
        mock_wakeup = std::make_unique<MockPollerWakeup>();
        event_loop = std::make_unique<EventLoop>(mock_poller, std::move(mock_wakeup));
    }

    std::shared_ptr<MockIoPoller> mock_poller;
    std::unique_ptr<MockPollerWakeup> mock_wakeup;
    std::unique_ptr<EventLoop> event_loop;
};

// Test Case: Test loop method
TEST_F(EventLoopTest, LoopShouldRunInCorrectThread) {
    std::cout << "LoopShouldRunInCorrectThread" << std::endl;
    EXPECT_EQ(event_loop->is_in_loop_thread(), true);
}

// Test Case: Test run_in_loop method when in the same thread
TEST_F(EventLoopTest, RunInLoopShouldExecuteCallbackDirectly) {
    bool callback_called = false;
    std::cout << "RunInLoopShouldExecuteCallbackDirectly" << std::endl;

    event_loop->run_in_loop([&callback_called]() {
        callback_called = true;
    }, "test_callback");

    EXPECT_EQ(callback_called, true);
}

// Test Case: Test run_in_loop method when in a different thread
TEST_F(EventLoopTest, RunInLoopShouldQueueCallback) {
    bool callback_called = false;
    std::cout << "RunInLoopShouldQueueCallback" << std::endl;

    std::thread other_thread([this, &callback_called]() {
        event_loop->run_in_loop([this, &callback_called]() {
            callback_called = true;
            //这里需要设置退出条件，否则下面的event_loop->loop()会在内部一直循环
            event_loop->quit();
        }, "test_callback");
    });

    // Wait for the other thread to finish
    other_thread.join();

    // Simulate event loop processing
    event_loop->loop();

    EXPECT_EQ(callback_called, true);
}

// Test Case: Test quit method
TEST_F(EventLoopTest, QuitShouldStopLoop) {
    std::cout << "QuitShouldStopLoop" << std::endl;
    bool loop_running = true;
    std::thread loop_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        loop_running = false;
        event_loop->quit();
    });
    event_loop->loop();

    // Wait for the loop to stop
    loop_thread.join();

    EXPECT_EQ(loop_running, false);
}

// Test Case: Test add_channel method
TEST_F(EventLoopTest, AddChannelShouldCallPollerAddChannel) {
    std::cout << "AddChannelShouldCallPollerAddChannel" << std::endl;

    MockIoChannel mock_channel;
    EXPECT_CALL(*mock_poller, add_channel(Ref(mock_channel))).Times(1);

    event_loop->get_poller()->add_channel(mock_channel);
}

// Test Case: Test remove_channel method
TEST_F(EventLoopTest, RemoveChannelShouldCallPollerRemoveChannel) {
    std::cout << "RemoveChannelShouldCallPollerRemoveChannel" << std::endl;
    MockIoChannel mock_channel;
    EXPECT_CALL(*mock_poller, remove_channel(Ref(mock_channel))).Times(1);

    event_loop->get_poller()->remove_channel(mock_channel);
}

// Test Case: Test update_channel method
TEST_F(EventLoopTest, UpdateChannelShouldCallPollerUpdateChannel) {
    std::cout << "UpdateChannelShouldCallPollerUpdateChannel" << std::endl;
    MockIoChannel mock_channel;
    EXPECT_CALL(*mock_poller, update_channel(Ref(mock_channel))).Times(1);

    event_loop->get_poller()->update_channel(mock_channel);
}