#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "tcp_connection.h"
#include "logging.h"
#include "event_loop.h"
#include "tinynet_util.h"
namespace tinynet
{

TcpConnection::TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port,
                EventLoop *event_loop, std::string name)
    : _name(name),
      _sockfd(sockfd),
      _client_ip(client_ip),
      _client_port(client_port),
      _server_ip(server_ip),
      _server_port(server_port),
      _channel(sockfd, event_loop->get_poller(), _name + ":channel"),
      _data_buffer(4096),
      _state(TCP_CONNECTED)
{
    _channel.set_reab_callback(std::bind(&TcpConnection::handle_onmessage, this));
    _channel.set_write_callback(std::bind(&TcpConnection::handle_write_complete, this));
    _channel.set_close_callback(std::bind(&TcpConnection::handle_disconnected, this));

    LOG(DEBUG) << "TcpConnection created: " << _name << std::endl;
}

TcpConnection::~TcpConnection() 
{
    if (check_fd(_sockfd))
    {
        close();
    }
    LOG(DEBUG) << "connection: " <<_name << " has been destructed." << std::endl;
}

void TcpConnection::close() 
{
    if (check_fd(_sockfd))
    {
        LOG(INFO) << "close " << _name << " socket fd" << std::endl;
        ::close(_sockfd);
        _sockfd = -1;
    }
}

void TcpConnection::write_data(const void* buffer, size_t length)
{
    ssize_t bytes_write = 0;
    size_t left_size = length;

    if (check_fd(_sockfd))
    {
        /* TODO: Implement asynchronous write using cache.*/
        while (left_size)
        {
            bytes_write = ::write(_sockfd, buffer, left_size);
            LOG(DEBUG) << _name << " send data size = " << bytes_write << std::endl;

            if (0 > bytes_write)
            {
                error_to_str(errno);
                break;
            }
            else
            {
                left_size -= bytes_write;
            }
        }
    }
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
    ssize_t bytes_read = read(_sockfd, _data_buffer.data(), _data_buffer.size());
    LOG(DEBUG) <<_name << " recv data. " << "len=" << bytes_read << std::endl;
    if (bytes_read > 0) 
    {
        
        if (nullptr != _on_message_cb)
        {
            _on_message_cb(shared_from_this(), _data_buffer.data(), (size_t)bytes_read);
        }
    }
    else if (bytes_read == 0)
    {
        handle_disconnected();
    } 
    else
    {
        LOG(ERROR) << "read failed in TcpConnection::handle_onmessage, err info:"
            << error_to_str(errno) << std::endl;
    }
}

void TcpConnection::handle_disconnected(void)
{
    disable_conn();
}

void TcpConnection::handle_write_complete(void)
{
    if (nullptr != _write_complete_cb)
    {
        _write_complete_cb(shared_from_this());
    }
}

void TcpConnection::disable_conn(void)
{
    _channel.disable_all();
    close();

    // FIXME: Perhaps callbacks shouldn't be called in this context
    if (nullptr != _disconected_cb)
    {
        _disconected_cb(shared_from_this());
    }
}
} // tinynet
