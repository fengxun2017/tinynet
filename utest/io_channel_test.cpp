#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "io_channel.h"
#include "io_poller_interface.h"
#include <memory>
#include <string>
#include <sys/epoll.h>

using namespace tinynet;
using namespace testing;


class MockIoPoller : public IoPollerInterface {
public:
    MOCK_METHOD(int, poll, (int timeout_ms, IoChannels &active_channels), (override));
    MOCK_METHOD(int, add_channel, (IoChannelInterface &channel), (override));
    MOCK_METHOD(int, remove_channel, (IoChannelInterface &channel), (override));
    MOCK_METHOD(int, update_channel, (IoChannelInterface &channel), (override));
};

class IoChannelTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_poller = std::make_shared<MockIoPoller>();
        channel = std::make_unique<IoChannel>(1, mock_poller, "test_channel");
    }

    std::shared_ptr<MockIoPoller> mock_poller;
    std::unique_ptr<IoChannel> channel;
};

TEST_F(IoChannelTest, TestEnableRead) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    channel->enable_read();
    EXPECT_EQ(channel->get_events_interested(), EPOLLIN | EPOLLPRI);
}

TEST_F(IoChannelTest, TestDisableRead) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    EXPECT_CALL(*mock_poller, remove_channel(_)).Times(1);
    channel->enable_read();
    channel->disable_read();
    EXPECT_EQ(channel->get_events_interested(), 0);
}

TEST_F(IoChannelTest, TestEnableReadTwice) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    EXPECT_CALL(*mock_poller, update_channel(_)).Times(1);
    channel->enable_read();
    channel->enable_read();
    EXPECT_EQ(channel->get_events_interested(), EPOLLIN | EPOLLPRI);
}

TEST_F(IoChannelTest, TestEnableWrite) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    channel->enable_write();
    EXPECT_EQ(channel->get_events_interested(), EPOLLOUT);
}

TEST_F(IoChannelTest, TestDisableWrite) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    channel->enable_write();
    EXPECT_CALL(*mock_poller, remove_channel(_)).Times(1);
    channel->disable_write();
    EXPECT_EQ(channel->get_events_interested(), 0);
}

TEST_F(IoChannelTest, TestDisableAll) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    EXPECT_CALL(*mock_poller, update_channel(_)).Times(1);
    channel->enable_read();
    channel->enable_write();
    EXPECT_CALL(*mock_poller, remove_channel(_)).Times(1);
    channel->disable_all();
    EXPECT_EQ(channel->get_events_interested(), 0);
}

TEST_F(IoChannelTest, TestIsWriting) {
    EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
    channel->enable_write();
    EXPECT_TRUE(channel->is_writing());

    EXPECT_CALL(*mock_poller, remove_channel(_)).Times(1);
    channel->disable_write();
    EXPECT_FALSE(channel->is_writing());
}

TEST_F(IoChannelTest, TestHandleEventRead) {
    bool callback_called = false;
    channel->set_read_callback([&callback_called]() { callback_called = true; });
    channel->set_events_received(EPOLLIN);
    channel->handle_event();
    EXPECT_TRUE(callback_called);
}

TEST_F(IoChannelTest, TestHandleEventWrite) {
    bool callback_called = false;
    channel->set_write_callback([&callback_called]() {callback_called = true;});
    channel->set_events_received(EPOLLOUT);
    channel->handle_event();
}
