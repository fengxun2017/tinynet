#include "logging.h"
#include "tcp_server.h"
#include "event_loop.h"

int main(void)
{
    tinynet::EventLoop loop();
    tinynet::TcpServer tcp_server(&loop, 127.0.0.1, 14000, "test_server");

    return 0;
}