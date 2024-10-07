#ifndef _TINYNET_TCP_CONNECTION_H_
#define _TINYNET_TCP_CONNECTION_H_

#include <functional>
#include <string>
#include "io_socket.h"
#include "io_channel.h"

namespace tinynet
{

class TcpConnection {
public:
    TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port);

    ~TcpConnection();

    void write_data(const void* buffer, size_t length);
    void set_disconnected_cb(std::function<void(TcpConnection &conn)> &disconected_cb) {_disconected_cb = disconected_cb;};
    void set_onmessage_cb(std::function<void(TcpConnection &conn)> &on_message_cb) {_on_message_cb = on_message_cb;};
    void set_write_complete_cb(std::function<void(TcpConnection &conn)> & write_complete_cb) { _write_complete_cb = write_complete_cb;}
    std::string get_client_ip(void);

    int get_client_port(void);

    void close(void);

private:
    void handle_recvdata(uint8_t data, size_t len);

    IoSocket _socket;
    IoChannel _channel;
    std::string _client_ip;
    int _client_port;
    std::string _server_ip;
    int _server_port;
    std::function<void(TcpConnection &conn)> _write_complete_cb = nullptr;
    std::function<void(TcpConnection &conn)> _on_message_cb = nullptr;
    std::function<void(TcpConnection &conn)> _disconected_cb = nullptr;

};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}


#endif // _TINYNET_TCP_CONNECTION_H_