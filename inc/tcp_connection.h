#ifndef _TINYNET_TCP_CONNECTION_H_
#define _TINYNET_TCP_CONNECTION_H_

#include <functional>
#include <string>
#include <memory>
#include "io_socket.h"
#include "io_channel.h"
#include "event_loop.h"

namespace tinynet
{

typedef enum
{
    TCP_CONNECTED = 1,
    TCP_DISCONNECTED,
}TcpConnState;

class TcpConnection;
using TcpConnPtr = std::shared_ptr<TcpConnection>;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port,
                EventLoop *event_loop, std::string &name);

    ~TcpConnection();

    void write_data(const void* buffer, size_t length);
    void set_disconnected_cb(std::function<void(TcpConnPtr)> disconected_cb) {_disconected_cb = disconected_cb;}
    void set_onmessage_cb(std::function<void(TcpConnPtr, const uint8_t *, size_t )> on_message_cb) {_on_message_cb = on_message_cb;}
    void set_write_complete_cb(std::function<void(TcpConnPtr)> write_complete_cb) { _write_complete_cb = write_complete_cb;}
    int get_fd(void) {return _channel.get_fd();}

    std::string get_client_ip(void);

    int get_client_port(void);

    void close(void);
    std::string get_name(void) {return _name;}
    
private:
    void handle_onmessage(void);
    void handle_disconnected(void);
    void handle_write_complete(void);

    std::string _name;
    int _sockfd;
    std::string _client_ip;
    int _client_port;
    std::string _server_ip;
    int _server_port;
    IoChannel _channel;
    std::vector<uint8_t> _data_buffer;
    TcpConnState _state;
    std::function<void(TcpConnPtr)> _write_complete_cb = nullptr;
    std::function<void(TcpConnPtr, const uint8_t *, size_t)> _on_message_cb = nullptr;
    std::function<void(TcpConnPtr)> _disconected_cb = nullptr;
};

}


#endif // _TINYNET_TCP_CONNECTION_H_