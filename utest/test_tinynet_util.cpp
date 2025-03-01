#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>
#include "event_loop.h"
#include "tinynet_util.h"
#include "io_channel.h"

using namespace tinynet;
using namespace testing;

// Mock IoPoller
class MockIoPoller : public IoPoller {
public:
    MOCK_METHOD2(poll, int(int timeout, Channels& active_channels));
    MOCK_METHOD1(add_channel, void(IoChannel &channel));
    MOCK_METHOD1(remove_channel, void(IoChannel &channel));
    MOCK_METHOD1(update_channel, void(IoChannel &channel));
};

// Mock Channel
class MockChannel : public IoChannel {
public:
    MockChannel(int fd, std::shared_ptr<IoPoller> poller, const std::string& name)
        : IoChannel(fd, poller, name) {}

    MOCK_METHOD0(handle_event, void());
    MOCK_METHOD1(set_read_callback, void(std::function<void()> callback));
};

// Test EventLoop constructor
TEST(EventLoopTest, Constructor) {
    MockIoPoller mock_poller;
    MockChannel mock_channel(0, std::make_shared<MockIoPoller>(), "eventloop_wakeup_channel");

    // EXPECT_CALL(mock_poller, poll(_, _)).Times(AnyNumber());
    // EXPECT_CALL(mock_channel, set_read_callback(_)).Times(1);

    EventLoop loop;
    EXPECT_FALSE(loop.is_quit());
    EXPECT_NE(loop.get_poller(), nullptr);
    EXPECT_NE(loop.get_event_fd(), -1);
    EXPECT_EQ(loop.get_thread_id(), std::this_thread::get_id());
}




// Test EventLoop::run_in_loop
TEST(EventLoopTest, RunInLoop) {
    EventLoop loop;

    bool callback_called = false;
    auto callback = [&callback_called]() {
        callback_called = true;
    };

    loop.run_in_loop(callback, "test_callback");
    EXPECT_TRUE(callback_called);

}

// Test EventLoop::exec_pending_cb


// Test EventLoop::loop
TEST(EventLoopTest, Loop) {
    EventLoop loop;


    std::thread thread([&loop]() {
        loop.loop();
    });

    loop.quit();
    thread.join();

}

// Test EventLoop::quit
TEST(EventLoopTest, Quit) {
    EventLoop loop;

    loop.quit();
    EXPECT_TRUE(loop.is_quit());

}

// Test EventLoop::get_poller
TEST(EventLoopTest, GetPoller) {
    EventLoop loop;
    EXPECT_NE(loop.get_poller(), nullptr);
}

// Test EventLoop::is_in_loop_thread
TEST(EventLoopTest, IsInLoopThread) {
    EventLoop loop;
    EXPECT_TRUE(loop.is_in_loop_thread());
}

// Test EventLoop::add_channel
TEST(EventLoopTest, AddChannel) {
    EventLoop loop;
    MockIoPoller mock_poller;
    MockChannel mock_channel(0, std::make_shared<MockIoPoller>(), "test_channel");

    EXPECT_CALL(mock_poller, add_channel(_)).Times(1);

    loop.get_poller()->add_channel(mock_channel);
}

// Test EventLoop::remove_channel
TEST(EventLoopTest, RemoveChannel) {
    EventLoop loop;
    MockIoPoller mock_poller;
    MockChannel mock_channel(0, std::make_shared<MockIoPoller>(), "test_channel");

    EXPECT_CALL(mock_poller, remove_channel(_)).Times(1);

    loop.get_poller()->remove_channel(mock_channel);
}

// Test EventLoop::update_channel
TEST(EventLoopTest, UpdateChannel) {
    EventLoop loop;
    MockIoPoller mock_poller;
    MockChannel mock_channel(0, std::make_shared<MockIoPoller>(), "test_channel");

    EXPECT_CALL(mock_poller, update_channel(_)).Times(1);

    loop.get_poller()->update_channel(mock_channel);
}