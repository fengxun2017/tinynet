#include <cassert>
#include <cstddef>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <errno.h>
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
      _read_data_buffer(4096),
      _state(TCP_CONNECTED),
      _write_data_buffer(4096),
      _event_loop(event_loop)
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

void TcpConnection::write_data(const void *buffer, size_t size)
{
    if (nullptr != _event_loop)
    {
        if (_event_loop->is_in_loop_thread())
        {
            LOG(DEBUG) << "write_data direct in eventloop thread: " << std::this_thread::get_id() << std::endl;
            write_data_in_loop(buffer, size);
        }
        else
        {
            uint8_t *pbuffer = (uint8_t *)buffer;
            auto bind_func = std::bind(static_cast<void (TcpConnection::*)(std::vector<uint8_t>&)>(&TcpConnection::write_data_in_loop),
                    this,
                    std::vector<uint8_t>(pbuffer, pbuffer + size));
            _event_loop->run_in_loop(
                bind_func,
                "write_data_in_loop(vector<uint8_t>)");
        }
    }
    else
    {
        LOG(ERROR) << "_event_loop is null in TcpConnection::write_data_in_loop" << std::endl;
    }
}
void TcpConnection::write_data_in_loop(std::vector<uint8_t> &data_buffer)
{

    if (nullptr != _event_loop)
    {
        if (!_event_loop->is_in_loop_thread())
        {
            LOG(ERROR) << "write_data_in_loop not run in event_loop" << std::endl;
        }
        else
        {
            uint8_t *pdata = data_buffer.data();
            size_t size = data_buffer.size();
            write_data_in_loop(pdata, size);
        }
    }
    else
    {
        LOG(ERROR) << "_event_loop is null in TcpConnection::write_data_in_loop" << std::endl;
    }
}

void TcpConnection::write_data_in_loop(const void* buffer, size_t length)
{
    ssize_t bytes_written = 0;
    size_t left_size = length;
    bool has_error = false;

    assert(check_fd(_sockfd));
    assert(_event_loop != nullptr);
    assert(_event_loop->is_in_loop_thread());

    if (!_channel.is_writing())
    {
        bytes_written = ::write(_sockfd, buffer, left_size);
        if (bytes_written >= 0)
        {
            LOG(DEBUG) << _name << " send data in event_loop, size = " << bytes_written << std::endl;
            left_size -= bytes_written;
            if (0 == left_size && _write_complete_cb)
            {
                LOG(DEBUG) << "_write_complete_cb will run in eventloop" << std::endl;
                _event_loop->run_in_loop(std::bind(_write_complete_cb, shared_from_this()), "_write_complete_cb");
            }
        }
        else
        {
            LOG(WARNING) << "::write failed! err info: " << error_to_str(errno) << std::endl;
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                has_error = true;
            }
        }
    }

    if (left_size > 0 && (!has_error))
    {
        LOG(DEBUG) << "in write_data_in_loop, left_size = " << left_size << std::endl;

        const uint8_t *pdata = static_cast<const uint8_t*>(buffer);
        _write_data_buffer.append(pdata + bytes_written, left_size);
        if (!_channel.is_writing())
        {
            _channel.enable_write();
        }
    }
}

void TcpConnection::handle_onmessage(void)
{
    ssize_t bytes_read = read(_sockfd, _read_data_buffer.data(), _read_data_buffer.size());
    LOG(DEBUG) <<_name << " recv data. " << "len=" << bytes_read << std::endl;
    if (bytes_read > 0) 
    {
        if (nullptr != _on_message_cb)
        {
            _on_message_cb(shared_from_this(), _read_data_buffer.data(), (size_t)bytes_read);
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
    assert(_event_loop->is_in_loop_thread());

    if (nullptr != _write_complete_cb)
    {
        _write_complete_cb(shared_from_this());
    }
    size_t left_size = _write_data_buffer.current_size();
    if(left_size > 0)
    {
        uint8_t *pbuffer = _write_data_buffer.get_read_pointer();
        size_t bytes_written = ::write(_sockfd, pbuffer, left_size);

        if (bytes_written >= 0)
        {
            _write_data_buffer.confirm_consume(bytes_written);
            LOG(DEBUG) << "in handle_write_complete :consum size=" << bytes_written << std::endl;
        }
        else
        {
            LOG(WARNING) << "::write failed in handle_write_complete! err info: " << error_to_str(errno) << std::endl;
        }
    }
    else
    {
        _channel.disable_write();
        LOG(DEBUG) << "There is no more data to send, close the write completion notification event" << std::endl;
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
