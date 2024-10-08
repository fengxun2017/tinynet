#include <string>
#include <utility>
#include "logging.h"
#include "tcp_server.h"
#include "tcp_connection.h"

namespace tinynet
{

typedef enum 
{
    EVENT_ON_MESSAGE = 0x01,
    EVENT_WRITE_COMPLETE,
    EVENT_DISCONNECTED
}EventType;
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

void TcpServer::handle_new_connection(std::shared_ptr<TcpConnection>& new_conn)
{
    if(nullptr != new_conn)
    {
        auto item = _connections.find(conn.get_fd());
        if (item != _connections.end())
        {
            LOG(ERROR) << "The same file descriptor already exists" << std::endl; 
        }
        else
        {
            LOG(INFO) << "The connection from " << new_conn->get_client_ip() << ":" << new_conn->get_client_port() << " is established" << std::endl;
            _connections.emplace(std::make_pair(new_conn.get_fd(), new_conn));
            if (_disconnected_cb)
            {
                new_conn->set_disconnected_cb(std::bind(&TcpServer::handle_disconnected, this));
            }
            if (_on_message_cb)
            {
                new_conn->set_onmessage_cb(std::bind(&TcpServer::handle_message, this));
            }
            if (_write_complete_cb)
            {
                new_conn->set_write_complete_cb(std::bind(&TcpServer::handle_write_complete, this));
            }

            if (_newconn_cb) {
                _newconn_cb(*new_conn);
            }
        }
    }
    else
    {
        LOG(ERROR) << "new_conn is NULL in TcpServer::handle_new_connection" << std::endl;
    }
}


void TcpServer::handle_disconnected(TcpConnection& conn)
{
    LOG(INFO) << "the connection with " << conn.get_client_ip() << ":" << conn.get_client_port() << "is disconnected\n";

    auto item = _connections.find(conn.get_fd());
    if (item != _connections.end())
    {
        (void)_connections.erase(item);

        if (_disconnected_cb) {
            _disconnected_cb(conn);
        }
    }
    else
    {
        LOG(ERROR) << "error in TcpServer::handle_disconnected, "
                << "the connection was not found from the connection collection " << std::endl;
    }
}

void TcpServer::handle_message(TcpConnection& conn)
{
    LOG(INFO) << conn.get_client_ip() << ":" << conn.get_client_port() << "receives data\n";
    auto item = _connections.find(conn.get_fd());
    if (item != _connections.end())
    {
        if (_on_message_cb) {
            _on_message_cb(conn);
        }
    }
    else
    {
        LOG(ERROR) << "error in TcpServer::handle_message, "
                << "the connection was not found from the connection collection " << std::endl;
    }
}

void TcpServer::handle_write_complete(TcpConnection& conn)
{
    LOG(INFO) << "the write operation to connect " << conn.get_client_ip() << ":" << conn.get_client_port() << "is complete\n";
    auto item = _connections.find(conn.get_fd());
    if (item != _connections.end())
    {
        if (_write_complete_cb) {
            _write_complete_cb(conn);
        }
    }
    else
    {
        LOG(ERROR) << "error in TcpServer::handle_write_complete, "
                << "the connection was not found from the connection collection " << std::endl;
    }
}

}  // namespace tinynet

