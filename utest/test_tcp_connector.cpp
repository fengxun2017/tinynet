#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <utility>
#include "io_poller_interface.h"
#include "tcp_connector.h"
#include "event_loop.h"
#include "io_socket.h"
#include "logging.h"

using namespace tinynet;
using namespace testing;

class MockEventLoop : public EventLoop {
public:
    MockEventLoop() {}
    ~MockEventLoop() {}

    // Mock implementation of get_poller
    IoPollerInterface* get_poller() {
        return nullptr; // Return a mock poller if needed
    }
};

class MockIoSocket : public IoSocket {
public:
    MockIoSocket(const std::string &name, Protocol protocol, int fd=-1)
    :IoSocket(name, protocol, fd) 
    {
        
    }
    MOCK_METHOD(int, connect, (struct sockaddr* addr, socklen_t addrlen));
    MOCK_METHOD(int, get_socket_error, ());
    MOCK_METHOD(int, get_fd, ());
};

class TcpConnectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        event_loop = std::make_unique<MockEventLoop>(); 
        mock_socket = std::make_unique<MockIoSocket>("test", SocketInterface::Protocol::TCP);
        tcp_connector = std::make_unique<TcpConnector>(event_loop.get(), "test_connector");
    }

    void TearDown() override {
        tcp_connector.reset();
        event_loop.reset();
    }

    std::unique_ptr<MockEventLoop> event_loop;
    std::unique_ptr<TcpConnector> tcp_connector;
    std::unique_ptr<MockIoSocket> mock_socket; // 原始指针检测调用情况
};

TEST_F(TcpConnectorTest, ConnectSuccess) {
    std::unique_ptr<MockIoSocket> mock_socket = std::make_unique<MockIoSocket>("test", SocketInterface::Protocol::TCP);
    auto socket = mock_socket.get();
    EXPECT_CALL(*socket, connect(_, _)).WillOnce(Return(0));

    EXPECT_TRUE(tcp_connector->connect("127.0.0.1", 8080, std::move(mock_socket)));
}

TEST_F(TcpConnectorTest, ConnectFailure) {
    // 模拟连接失败

    std::unique_ptr<MockIoSocket> mock_socket = std::make_unique<MockIoSocket>("test", SocketInterface::Protocol::TCP);
    auto socket = mock_socket.get();

    EXPECT_CALL(*socket, connect(_, _)).WillOnce(Return(-1));
    EXPECT_FALSE(tcp_connector->connect("127.0.0.1", 8080, std::move(mock_socket)));
}
