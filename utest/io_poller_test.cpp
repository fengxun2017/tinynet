#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "io_poller.h"
#include "io_channel.h"
#include <memory>
#include <string>
#include <sys/epoll.h>

using namespace tinynet;
using namespace testing;

class MockIoChannel : public IoChannelInterface {
public:
    MOCK_METHOD(void, set_read_callback, (ChannelEventCallback read_cb), (override));
    MOCK_METHOD(void, set_write_callback, (ChannelEventCallback write_cb), (override));
    MOCK_METHOD(void, set_close_callback, (ChannelEventCallback close_cb), (override));
    MOCK_METHOD(void, set_error_callback, (ChannelEventCallback error_cb), (override));

    MOCK_METHOD(void, handle_event, (), (override));

    MOCK_METHOD(void, enable_read, (), (override));
    MOCK_METHOD(void, enable_write, (), (override));
    MOCK_METHOD(void, disable_read, (), (override));
    MOCK_METHOD(void, disable_write, (), (override));
    MOCK_METHOD(void, disable_all, (), (override));
    MOCK_METHOD(bool, is_writing, (), (override));

    MOCK_METHOD(int, get_fd, (), (override));
    MOCK_METHOD(void, set_events_received, (int events), (override));
    MOCK_METHOD(std::string, get_name, (), (override));

    MOCK_METHOD(uint32_t, get_events_interested, (), (override));
    MOCK_METHOD(uint32_t, get_events_received, (), (override));
};

class IoPollerTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_poller = std::make_unique<IoPoller>();
    }

    std::unique_ptr<IoPoller> io_poller;
};

TEST_F(IoPollerTest, TestAddChannel) {
    MockIoChannel mock_channel;
    EXPECT_CALL(mock_channel, get_fd()).WillOnce(Return(1));
    EXPECT_CALL(mock_channel, get_events_interested()).WillOnce(Return(1));
    EXPECT_CALL(mock_channel, get_name()).WillOnce(Return("test"));

    EXPECT_EQ(io_poller->add_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}

TEST_F(IoPollerTest, TestAddChannelFail) {
    MockIoChannel mock_channel;

    // An invalid file descriptor was used
    EXPECT_CALL(mock_channel, get_fd()).WillOnce(Return(-1));
    EXPECT_CALL(mock_channel, get_events_interested()).WillOnce(Return(1));
    EXPECT_CALL(mock_channel, get_name()).WillOnce(Return("test"));

    EXPECT_NE(io_poller->add_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}

TEST_F(IoPollerTest, TestRemoveChannel) {
    MockIoChannel mock_channel;
    EXPECT_CALL(mock_channel, get_fd()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_events_interested()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_name()).Times(2).WillRepeatedly(Return("test"));

    EXPECT_EQ(io_poller->add_channel(mock_channel), 0);
    EXPECT_EQ(io_poller->remove_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}
TEST_F(IoPollerTest, TestRemoveChannelFail) {
    MockIoChannel mock_channel;

    // An invalid file descriptor was used
    EXPECT_CALL(mock_channel, get_fd()).Times(2).WillRepeatedly(Return(-1));
    EXPECT_CALL(mock_channel, get_events_interested()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_name()).Times(2).WillRepeatedly(Return("test"));

    EXPECT_NE(io_poller->add_channel(mock_channel), 0);
    EXPECT_NE(io_poller->remove_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}


TEST_F(IoPollerTest, TestUpdateChannel) {
    MockIoChannel mock_channel;
    EXPECT_CALL(mock_channel, get_fd()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_events_interested()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_name()).Times(2).WillRepeatedly(Return("test"));

    EXPECT_EQ(io_poller->add_channel(mock_channel), 0);
    EXPECT_EQ(io_poller->update_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}

TEST_F(IoPollerTest, TestUpdateChannelFail) {
    MockIoChannel mock_channel;

    // An invalid file descriptor was used
    EXPECT_CALL(mock_channel, get_fd()).Times(2).WillRepeatedly(Return(-1));
    EXPECT_CALL(mock_channel, get_events_interested()).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(mock_channel, get_name()).Times(2).WillRepeatedly(Return("test"));

    EXPECT_NE(io_poller->add_channel(mock_channel), 0);
    EXPECT_NE(io_poller->update_channel(mock_channel), 0);
    // Additional checks can be added here to verify the internal state of io_poller
}

TEST_F(IoPollerTest, TestPoll) {
    MockIoChannel mock_channel;
    EXPECT_CALL(mock_channel, get_fd()).WillOnce(Return(1));
    EXPECT_CALL(mock_channel, get_events_interested()).WillOnce(Return(1));
    EXPECT_CALL(mock_channel, get_name()).WillOnce(Return("test"));

    EXPECT_EQ(io_poller->add_channel(mock_channel), 0);
    IoChannels active_channels;
    EXPECT_EQ(io_poller->poll(1000, active_channels), 0);
}