#include <string>
#include "tcp_server.h"
#include "tcp_connection.h"

namespace tinynet
{

TcpServer::TcpServer(const std::string& ip, int port)
    : _acceptor(ip, port) 
{
    _acceptor.set_newconn_cb(std::bind(&TcpServer::handle_new_connection, this));
}

TcpServer::~TcpServer(void)
{
    stop();
}

bool TcpServer::start(void)
{
    return _acceptor.start();
}

void TcpServer::stop(void)
{
    _acceptor.stop();
    for (auto& conn : _connections)
    {
        conn->close();
    }
    _connections.clear();
}

void TcpServer::handle_new_connection(TcpConnectionPtr& new_conn)
{
    if(nullptr != new_conn)
    {
        _connections.push_back(new_conn);
        if (_disconnected_cb) {
            new_conn->set_disconnected_cb(std::bind(&TcpServer::handle_disconnected, this));
        }
        if (_on_message_cb) {
            new_conn->set_onmessage_cb(std::bind(&TcpServer::handle_message, this));
        }
        if (_write_complete_cb) {
            new_conn->set_write_complete_cb(std::bind(&TcpServer::handle_write_complete, this));
        }

        if (_newconn_cb) {
            _newconn_cb(*new_conn);
        }
    }
    else
    {
        LOG(ERROR) << "new_conn is NULL in TcpServer::handle_new_connection" << std::endl;
    }
}

void handle_disconnected(TcpConnection& conn)
{
    auto it = std::find_if(_connections.begin(), _connections.end(),
                            &conn { return c.get() == &conn; });
    if (it != _connections.end())
    {
        _connections.erase(it);
    }
    if (_disconnected_cb) {
        _disconnected_cb(conn);
    }
}

void handle_message(TcpConnection& conn)
{
    if (_on_message_cb) {
        _on_message_cb(conn);
    }
}

void handle_write_complete(TcpConnection& conn)
{
    if (_write_complete_cb) {
        _write_complete_cb(conn);
    }
}

}  // namespace tinynet

