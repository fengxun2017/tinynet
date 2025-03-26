#include <gtest/gtest.h>
#include <string>
#include "io_socket.h"
#include "socket_factory.h"
#include "socket_interface.h"
#include "logging.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace tinynet;
using namespace testing;

extern "C" {

    /***********mock  api: socket */
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

    /****************mock api: bind */
    int bind_success(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return 0; 
    }
    int bind_fail(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return -1;
    }
    int (*mock_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = nullptr;
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return mock_bind(sockfd, addr, addrlen);
    }

    /**************mock api: inet_pton *************/
   int inet_pton_success(int af, const char *src, void *dst) {
        return 1; 
    }
    int inet_pton_fail(int af, const char *src, void *dst) {
        return 0;
    }
    int (*mock_inet_pton)(int af, const char *src, void *dst) = nullptr;
    int inet_pton(int af, const char *src, void *dst)
    {
        return  mock_inet_pton(af, src, dst);
    }

    /**************mock api: listen *************/
    int listen_success(int sockfd, int backlog) {
        return 0; 
    }
    int listen_fail(int sockfd, int backlog) {
        return -1;
    }
    int (*mock_listen)(int sockfd, int backlog) = nullptr;
    int listen(int sockfd, int backlog)
    {
        return mock_listen(sockfd, backlog);
    }

    /************mock api: accept4*****************/
    int accept4_success(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
        return 0x1234; 
    }
    int accept4_fail(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
        return -1;
    }
    int (*mock_accept4)(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) = nullptr;
    int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags)
    {
        return mock_accept4(sockfd, addr, addrlen, flags);
    }

    /**************mock api: connect *************/
    int connect_success(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return 0; 
    }
    int connect_fail(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return -1;
    }
    int (*mock_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = nullptr;
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return mock_connect(sockfd, addr, addrlen);
    }
}

class IoSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code, if needed
        mock_socket = socket_success;
        mock_bind = bind_success;
        mock_inet_pton = inet_pton_success;
        mock_listen = listen_success;
        mock_accept4 = accept4_success;
        mock_connect = connect_success;
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

TEST_F(IoSocketTest, TestTcpSocketCreationSuccess) {
    mock_socket = socket_success;
    auto socket = CreateTcpSocket("test_tcp_socket");
    EXPECT_NE(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::TCP);
}

TEST_F(IoSocketTest, TestTcpSocketCreationFail) {
    mock_socket = socket_fail;
    auto socket = CreateTcpSocket("test_tcp_socket");
    EXPECT_EQ(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::TCP);
}

TEST_F(IoSocketTest, TestUdpSocketCreationSuccess) {
    mock_socket = socket_success;
    auto socket = CreateUdpSocket("test_udp_socket");
    EXPECT_NE(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::UDP);
}

TEST_F(IoSocketTest, TestUdpSocketCreationFail) {
    mock_socket = socket_fail;
    auto socket = CreateUdpSocket("test_udp_socket");
    EXPECT_EQ(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::UDP);
}


TEST_F(IoSocketTest, TestCanSocketCreationSuccess) {
    mock_socket = socket_success;
    auto socket = CreateCanSocket("test_can_socket");
    EXPECT_NE(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::CAN);
}

TEST_F(IoSocketTest, TestCanSocketCreationSuccessFail) {
    mock_socket = socket_fail;
    auto socket = CreateCanSocket("test_can_socket");
    EXPECT_EQ(socket->get_fd(), -1);
    EXPECT_EQ(socket->get_protocol(), tinynet::SocketInterface::CAN);
}

TEST_F(IoSocketTest, TestTcpBindSuccess) {
    
    auto socket = CreateTcpSocket("test_tcp_bind");
    EXPECT_TRUE(socket->bind("127.0.0.1", 8080));
}

TEST_F(IoSocketTest, TestTcpBindFail1) {
    mock_inet_pton = inet_pton_fail;
    auto socket = CreateTcpSocket("test_tcp_bind");
    EXPECT_FALSE(socket->bind("127.0.0.1", 8080));
}
TEST_F(IoSocketTest, TestTcpBindFail2) {
    mock_bind = bind_fail;
    auto socket = CreateTcpSocket("test_tcp_bind");
    EXPECT_FALSE(socket->bind("127.0.0.1", 8080));
}

TEST_F(IoSocketTest, TestUdpBind) {
    auto socket = CreateUdpSocket("test_udp_bind");
    EXPECT_TRUE(socket->bind("127.0.0.1", 8081));
}

TEST_F(IoSocketTest, TestCanBindSuccess) {
    mock_bind = bind_success;
    auto socket = CreateCanSocket("test_can_bind");
    // Assuming "can0" is a valid CAN interface on your system
    EXPECT_TRUE(socket->bind("can0", 0));
}
TEST_F(IoSocketTest, TestCanBindFail) {
    mock_bind = bind_fail;
    auto socket = CreateCanSocket("test_can_bind");
    // Assuming "can0" is a valid CAN interface on your system
    EXPECT_FALSE(socket->bind("can0", 0));
}

TEST_F(IoSocketTest, TestTcpListenSuccess) {
    auto socket = CreateTcpSocket("test_tcp_listen");
    EXPECT_TRUE(socket->listen(5));
}
TEST_F(IoSocketTest, TestTcpListenFail) {
    mock_listen = listen_fail;
    auto socket = CreateTcpSocket("test_tcp_listen");
    EXPECT_FALSE(socket->listen(5));
}

TEST_F(IoSocketTest, TestUdpListen) {
    auto socket = CreateUdpSocket("test_udp_listen");
    EXPECT_FALSE(socket->listen(5));
}

TEST_F(IoSocketTest, TestTcpAcceptSuccess) {
    auto server_socket = CreateTcpSocket("test_tcp_accept_server");
    std::string client_ip;
    int client_port;
    
    EXPECT_NE(server_socket->accept(client_ip, client_port), -1); 
}
TEST_F(IoSocketTest, TestTcpAcceptFail) {
    auto server_socket = CreateTcpSocket("test_tcp_accept_server");
    std::string client_ip;
    int client_port;
    mock_accept4 = accept4_fail;
    EXPECT_EQ(server_socket->accept(client_ip, client_port), -1); 
}

TEST_F(IoSocketTest, TestUdpAccept) {
    auto server_socket = CreateUdpSocket("test_udp_accept_server");
    std::string client_ip;
    int client_port;

    EXPECT_EQ(server_socket->accept(client_ip, client_port), -1); 
}
TEST_F(IoSocketTest, TestTcpConnectSuccess) {
    auto socket = CreateTcpSocket("test_tcp_connect");
    struct sockaddr_in addr;

    mock_connect = connect_success;
    EXPECT_NE(socket->connect((struct sockaddr*)&addr, sizeof(addr)), -1); // No server running, should return -1
}
TEST_F(IoSocketTest, TestTcpConnectFail) {
    auto socket = CreateTcpSocket("test_tcp_connect");
    struct sockaddr_in addr;

    mock_connect = connect_fail;
    EXPECT_EQ(socket->connect((struct sockaddr*)&addr, sizeof(addr)), -1); // No server running, should return -1
}

