#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "wakeup_poller.h"
#include "io_poller_interface.h"
#include "logging.h"
#include <memory>
#include <functional>

using namespace tinynet;
using namespace testing;

class MockIoPoller : public IoPollerInterface {
public:
    MOCK_METHOD(int, poll, (int timeout_ms, IoChannels &active_channels), (override));
    MOCK_METHOD(int, add_channel, (IoChannelInterface &channel), (override));
    MOCK_METHOD(int, remove_channel, (IoChannelInterface &channel), (override));
    MOCK_METHOD(int, update_channel, (IoChannelInterface &channel), (override));
};

class TestWakeupPoller : public ::testing::Test {
protected:
    void SetUp() override {
        mock_poller = std::make_shared<MockIoPoller>();
        _callback_value = -1;

        EXPECT_CALL(*mock_poller, add_channel(_)).Times(1);
        wakeup_poller = std::make_unique<PollerWakeup>(mock_poller, "test_wakeup", [this](int ret) { _callback_value = ret; });
    }

    void TearDown() override {
        wakeup_poller.reset();
        mock_poller.reset();
    }

    std::shared_ptr<MockIoPoller> mock_poller;
    std::unique_ptr<PollerWakeup> wakeup_poller;
    int _callback_value;
};

TEST_F(TestWakeupPoller, TestWakeup) {
    wakeup_poller->wakeup();
    wakeup_poller->handle_recv();
    EXPECT_EQ(_callback_value, 8);
}
