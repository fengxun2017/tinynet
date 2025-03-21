#ifndef _TINYNET_TCP_CONNECTION_H_
#define _TINYNET_TCP_CONNECTION_H_

#include <cstdint>
#include <functional>
#include <string>
#include <memory>
#include <any>
#include <vector>
#include <mutex>
#include "io_socket.h"
#include "io_channel.h"
#include "event_loop.h"
#include "ring_buffer.h"
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
                EventLoop *event_loop, std::string name);

    ~TcpConnection();

    void write_data(const void* buffer, size_t length);
    void set_disconnected_cb(std::function<void(TcpConnPtr)> disconected_cb) {_disconected_cb = disconected_cb;}
    void set_onmessage_cb(std::function<void(TcpConnPtr, const uint8_t *, size_t )> on_message_cb) {_on_message_cb = on_message_cb;}
    void set_write_complete_cb(std::function<void(TcpConnPtr)> write_complete_cb) { _write_complete_cb = write_complete_cb;}
    int get_fd(void) {return _channel.get_fd();}

    std::string get_client_ip(void) { return _client_ip;}
    int get_client_port(void) {return _client_port;}

    std::string get_name(void) {return _name;}

    void enable_read(void) {_channel.enable_read();}
    void enable_write(void) {_channel.enable_write();}
    void disable_conn(void);

    void set_context(const std::any &context) { _context = context; }
    std::any &get_context()  { return _context; }

    void set_context2(const std::any &context) { _context2 = context; }
    std::any &get_context2()  { return _context2; }

private:
    void write_data_in_loop(std::vector<uint8_t> &data_buffer);
    void write_data_in_loop(const void* buffer, size_t length);

    void handle_onmessage(void);
    void handle_disconnected(void);
    void handle_write_complete(void);
    void close(void);

    std::string _name;
    int _sockfd;
    std::string _client_ip;
    int _client_port;
    std::string _server_ip;
    int _server_port;
    IoChannel _channel;
    std::vector<uint8_t> _read_data_buffer;
    TcpConnState _state;
    std::any _context;
    std::any _context2;
    std::function<void(TcpConnPtr)> _write_complete_cb = nullptr;
    std::function<void(TcpConnPtr, const uint8_t *, size_t)> _on_message_cb = nullptr;
    std::function<void(TcpConnPtr)> _disconected_cb = nullptr;
    RingBuffer<uint8_t> _write_data_buffer;
    EventLoop *_event_loop;
};

}


#endif // _TINYNET_TCP_CONNECTION_H_