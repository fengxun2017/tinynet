#include <functional>
#include <mutex>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string>
#include "tcp_client.h"
#include "logging.h"
#include "tinynet_util.h"
namespace tinynet
{
TcpClient::TcpClient(EventLoop *event_loop, const std::string &client_name) :
    _name(client_name), 
    _event_loop(event_loop),
    _connector(event_loop, client_name + ":connector")
{
    _connector.set_newconn_cb(std::bind(&TcpClient::handle_new_connection, this, std::placeholders::_1));

    LOG(DEBUG) << "client:" << _name << " has been created" << std::endl;
}

TcpClient::~TcpClient()
{
    {
        std::lock_guard<std::mutex> lock(_conn_mutex);
        if (_conn)
        {
            // FIXME: There are risks
            _conn->disable_conn();
            _conn = nullptr;
        }
    }
    LOG(DEBUG) << "client:" << _name << " has been destructed" << std::endl;
}

void TcpClient::disconnect(void)
{
    std::lock_guard<std::mutex> lock(_conn_mutex);
    if (_conn)
    {
        // FIXME: There are risks
        _conn->disable_conn();
    }
}

void TcpClient::write_data(const void *buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(_conn_mutex);
    if (_conn)
    {
      _conn->write_data(buffer, size);
    }
}

bool TcpClient::connect(std::string server_ip, int server_port)
{
    bool ret = false;

    // FIXME: What happens if the user calls repeatedly?
    ret = _connector.connect(server_ip, server_port);

    return ret;
}

void TcpClient::handle_new_connection(TcpConnPtr conn)
{
    if (nullptr != conn)
    {
        {
            std::lock_guard<std::mutex> lock(_conn_mutex);
            _conn = conn;
            _conn->set_disconnected_cb(std::bind(&TcpClient::handle_disconnected, this, std::placeholders::_1));
            _conn->set_onmessage_cb(std::bind(&TcpClient::handle_message, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
            _conn->set_write_complete_cb(std::bind(&TcpClient::handle_write_complete, this, std::placeholders::_1));
            _conn->enable_read();
        }
        if (_newconn_cb)
        {
            _newconn_cb(_conn);
        }
    }
    else 
    {
        // FIXME: return nullptr?
        /* _connector_socket.get_socket_error return error in TcpConnector::handle_write_complete*/
        handle_disconnected(conn);
    }
}

void TcpClient::handle_disconnected(TcpConnPtr conn)
{
    if (_disconnected_cb)
    {
        _disconnected_cb(conn);
    }
    
}

void TcpClient::handle_message(TcpConnPtr conn, const uint8_t *data, size_t size)
{
    if (_on_message_cb)
    {
        _on_message_cb(conn, data, size);
    }
}

void TcpClient::handle_write_complete(TcpConnPtr conn)
{
    if (_write_complete_cb)
    {
        _write_complete_cb(conn);
    }
}

}