#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tcp_client.h"
#include "tcp_connector.h"
#include "event_loop.h"
#include "io_socket.h"
#include "io_channel.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

class MockTcpConnector : public tinynet::TcpConnector {
public:
    MockTcpConnector(tinynet::EventLoop *event_loop, std::string name)
    :TcpConnector(event_loop, name) 
    {
        
    }
    MOCK_METHOD(bool, connect, (const std::string &, int, std::unique_ptr<tinynet::IoSocket>), (override));
};

class MockEventLoop : public tinynet::EventLoop {
public:
    MOCK_METHOD(void, run_in_loop, (const tinynet::EventLoop::RunInLoopCallBack &, std::string), (override));
    MOCK_METHOD(bool, is_in_loop_thread, (), (override));
};

class MockTcpConnection : public tinynet::TcpConnection {
public:
    MOCK_METHOD(void, disable_conn, (), (override));
    MOCK_METHOD(void, write_data, (const void *, size_t), (override));
    MOCK_METHOD(void, enable_read, (), (override));
    MOCK_METHOD(void, set_disconnected_cb, (tinynet::TcpConnection::TcpConnDisconnectedCb));
    MOCK_METHOD(void, set_onmessage_cb, (tinynet::TcpConnection::TcpConnOnMessageCb));
    MOCK_METHOD(void, set_write_complete_cb, (tinynet::TcpConnection::TcpConnWriteCompleteCb));
};

class TcpClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_event_loop = std::make_unique<MockEventLoop>();
        mock_tcp_connector = std::make_unique<MockTcpConnector>(mock_event_loop.get(), "test_connector");
        tcp_client = std::make_unique<tinynet::TcpClient>(mock_event_loop.get(), "test_client", std::move(mock_tcp_connector));
    }

    std::unique_ptr<MockEventLoop> mock_event_loop;
    std::unique_ptr<MockTcpConnector> mock_tcp_connector;
    std::unique_ptr<tinynet::TcpClient> tcp_client;
};

TEST_F(TcpClientTest, ConnectTest) {

}
