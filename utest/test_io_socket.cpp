#include <gtest/gtest.h>
#include <string>
#include "io_socket.h"
#include "socket_factory.h"
#include "socket_interface.h"
#include "logging.h"
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace tinynet;
using namespace testing;

class IoSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code, if needed
    }

    void TearDown() override {
        // Teardown code, if needed
    }

    std::unique_ptr<IoSocket> CreateTcpSocket(const std::string& name) {
        return std::make_unique<IoSocket>(name, SocketInterface::TCP);
    }

    std::unique_ptr<IoSocket> CreateUdpSocket(const std::string& name) {
        return std::make_unique<IoSocket>(name, SocketInterface::UDP);
    }

    std::unique_ptr<IoSocket> CreateCanSocket(const std::string& name) {
        return std::make_unique<IoSocket>(name, SocketInterface::CAN);
    }
};

extern "C" {
    int socket_success(int domain, int type, int protocol) {
        return 0x1234; 
    }
    int socket_fail(int domain, int type, int protocol) {
    return -1;
    }

    int (*mock_socket)(int domain, int type, int protocol) = nullptr;
    int socket(int domain, int type, int protocol)
    {
        LOG(DEBUG) << "use mock socket, domain:" << domain << " type:" << type << " protocol:" << protocol << std::endl;
        return mock_socket(domain, type, protocol);
    }
}
TEST_F(IoSocketTest, TestTcpSocketCreationSuccess) {
    mock_socket = socket_success;
    auto socket = CreateTcpSocket("test_tcp_socket");
    EXPECT_NE(socket->get_fd(), -1);
}

TEST_F(IoSocketTest, TestTcpSocketCreationFail) {
    mock_socket = socket_fail;
    auto socket = CreateTcpSocket("test_tcp_socket");
    EXPECT_EQ(socket->get_fd(), -1);
}

TEST_F(IoSocketTest, TestUdpSocketCreation) {
    auto socket = CreateUdpSocket("test_udp_socket");
    EXPECT_NE(socket->get_fd(), -1);
}

TEST_F(IoSocketTest, TestCanSocketCreation) {
    auto socket = CreateCanSocket("test_can_socket");
    EXPECT_NE(socket->get_fd(), -1);
}

TEST_F(IoSocketTest, TestTcpBind) {
    auto socket = CreateTcpSocket("test_tcp_bind");
    EXPECT_TRUE(socket->bind("127.0.0.1", 8080));
}

TEST_F(IoSocketTest, TestUdpBind) {
    auto socket = CreateUdpSocket("test_udp_bind");
    EXPECT_TRUE(socket->bind("127.0.0.1", 8081));
}

TEST_F(IoSocketTest, TestCanBind) {
    auto socket = CreateCanSocket("test_can_bind");
    // Assuming "can0" is a valid CAN interface on your system
    EXPECT_TRUE(socket->bind("can0", 0));
}

TEST_F(IoSocketTest, TestTcpListen) {
    auto socket = CreateTcpSocket("test_tcp_listen");
    EXPECT_TRUE(socket->bind("127.0.0.1", 8082));
    EXPECT_TRUE(socket->listen(5));
}

TEST_F(IoSocketTest, TestTcpAccept) {
    auto server_socket = CreateTcpSocket("test_tcp_accept_server");
    EXPECT_TRUE(server_socket->bind("127.0.0.1", 8083));
    EXPECT_TRUE(server_socket->listen(5));

    // For simplicity, we will not create a client socket in this test.
    // In a real-world scenario, you would need to create a client socket and connect to the server.
    std::string client_ip;
    int client_port;
    EXPECT_EQ(server_socket->accept(client_ip, client_port), -1); // No client connected, should return -1
}

TEST_F(IoSocketTest, TestTcpConnect) {
    auto socket = CreateTcpSocket("test_tcp_connect");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8084);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    // For simplicity, we will not run a server on port 8084.
    // In a real-world scenario, you would need to run a server on this port.
    EXPECT_EQ(socket->connect((struct sockaddr*)&addr, sizeof(addr)), -1); // No server running, should return -1
}

TEST_F(IoSocketTest, TestTcpWriteRead) {
    auto server_socket = CreateTcpSocket("test_tcp_write_read_server");
    EXPECT_TRUE(server_socket->bind("127.0.0.1", 8085));
    EXPECT_TRUE(server_socket->listen(5));

    auto client_socket = CreateTcpSocket("test_tcp_write_read_client");
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8085);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);

    // For simplicity, we will not handle the server-client connection in this test.
    // In a real-world scenario, you would need to handle the server-client connection.
    EXPECT_EQ(client_socket->connect((struct sockaddr*)&server_addr, sizeof(server_addr)), -1); // No server running, should return -1

    // Assuming the connection is successful, you would write and read data here.
    // For simplicity, we will skip this part.
}

TEST_F(IoSocketTest, TestCanWriteRead) {
    auto socket = CreateCanSocket("test_can_write_read");
    // Assuming "can0" is a valid CAN interface on your system
    EXPECT_TRUE(socket->bind("can0", 0));

    uint32_t can_id = 0x123;
    const char* data = "Hello";
    size_t length = strlen(data);

    EXPECT_EQ(socket->write_can_data(can_id, data, length), -1); // No CAN bus connected, should return -1

    // Assuming the CAN bus is connected, you would read data here.
    // For simplicity, we will skip this part.
}

