#include <string>
#include <unistd.h>
#include "tcp_connection.h"
#include "logging.h"

namespace tinynet
{

TcpConnection::TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port,
                EventLoop *event_loop, std::string &name);
    : _name(name),
      _sockfd(sockfd),
      _client_ip(client_ip),
      _client_port(client_port),
      _server_ip(server_ip),
      _server_port(server_port),
      _channel(sockfd, event_loop->get_poller(), _name + "_channel")

{
    _channel.set_reab_callback(std::bind(&TcpConnection::handle_onmessage, this));
    _channel.set_write_callback(std::bind(&TcpConnection::handle_onmessage, this));
    _channel.set_close_callback(std::bind(&TcpConnection::handle_onmessage, this));
}

TcpConnection::~TcpConnection() 
{
    close(_sockfd);
}

void TcpConnection::write_data(const void* buffer, size_t length)
{

}


std::string TcpConnection::get_client_ip(void) 
{
    return _client_ip;
}

int TcpConnection::get_client_port(void) 
{
    return _client_port;
}

void TcpConnection::handle_onmessage(void)
{
    ssize_t bytes_read = read(_sockfd, _data_buffer, sizeof(_data_buffer));
    if (bytes_read > 0) 
    {
        LOG(DEBUG) << "recv data from " << get_client_ip() 
            <<":" << get_client_port() << ". len=" << bytes_read << std::endl;
        
        if (nullptr != _on_message_cb)
        {
            _on_message_cb(shared_from_this(), _data_buffer, bytes_read);
        }
    }
    else if (bytes_read == 0)
    {
        handle_disconnected();
    } 
    else
    {
        LOG(ERROR) << "read failed in TcpConnection::handle_onmessage, err info:"
            << error_to_str(errno);
    }
}

void TcpConnection::handle_disconnected(void)
{
    if (nullptr != _disconected_cb)
    {
        _disconected_cb(shard_from_this());
    }
}

void TcpConnection::handle_write_complete(void)
{
    if (nullptr != _write_complete_cb)
    {
        _write_complete_cb(shard_from_this());
    }
}

} // tinynet
