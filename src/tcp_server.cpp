#include <string>
#include <utility>
#include <sstream>
#include "logging.h"
#include "tcp_server.h"

namespace tinynet
{

void TcpServer::set_worker_thread_num(uint8_t num)
{
    _work_thread_num = num;
}

TcpServer::TcpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name, uint8_t work_thread_num)
    : _name(name),
      _ip(ip),
      _port(port),
      _acceptor(event_loop, ip, port, _name+"_acceptor"),
      _event_loop_pool(event_loop, _name+"_event_loop"),
      _work_thread_num(0)
{
    _event_loop = event_loop;
    _acceptor.set_newconn_cb(std::bind(&TcpServer::handle_new_connection, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    LOG(DEBUG) << "Service object: [" << ip <<":" << port <<"] is created" << std::endl;
}

TcpServer::~TcpServer(void)
{
    stop();
    LOG(DEBUG) << _name << " has been destructed." << std::endl;
}

bool TcpServer::start(void)
{
    bool ret = false;
    _event_loop_pool.create_pool(_work_thread_num);

    ret = _acceptor.start();
    LOG(INFO) << _name << " start!" << std::endl;

    return ret;
}

void TcpServer::stop(void)
{
    // Stop accepting new connections
    LOG(INFO) << "Stop " << _name << std::endl;
    _acceptor.close();

    for (auto& item : _connections)
    {
        LOG(INFO) << "TcpServer:" << _name << " disconnect from " << (item.second)->get_name() << std::endl;
        // (item.second)->close();
        (item.second)->disable_conn();
    }
    _connections.clear();
}

void TcpServer::handle_new_connection(int sockfd, const std::string& client_ip, int client_port)
{

#ifdef TINYNET_DEBUG
        auto item = _connections.find(sockfd);
        if (item != _connections.end())
        {
            LOG(ERROR) << "The same file descriptor already exists! something wrong" << std::endl; 
        }
        else
#endif
        {
            LOG(INFO) << "server:[" << _ip << ":" << _port << "] receives a connection request from client:["
                <<  client_ip << ":" << client_port << "]" << std::endl;
        
            std::ostringstream oss;
            oss << "[" << _ip << ":" << _port << "<->"
                    <<  client_ip << ":" << client_port << "]";
            std::string conn_name = std::move(oss.str());

            EventLoop *event_loop = _event_loop_pool.get_next_loop();
            TcpConnPtr new_conn = std::make_shared<TcpConnection>(sockfd, client_ip, client_port, _ip, _port, event_loop, conn_name);

            _connections.emplace(std::make_pair(new_conn->get_fd(), new_conn));
            new_conn->set_disconnected_cb(std::bind(&TcpServer::handle_disconnected, this, std::placeholders::_1));
            new_conn->set_onmessage_cb(std::bind(&TcpServer::handle_message, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
            new_conn->set_write_complete_cb(std::bind(&TcpServer::handle_write_complete, this, std::placeholders::_1));
            new_conn->enable_read();

            if (_newconn_cb) 
            {
                _newconn_cb(new_conn);
            }
        }

}


void TcpServer::handle_disconnected(TcpConnPtr conn)
{
    LOG(INFO) << "The connection between "<< _name << " and [" << conn->get_client_ip() << ":" << conn->get_client_port() << "] is disconnected" << std::endl;
    auto item = _connections.find(conn->get_fd());
    if (item != _connections.end())
    {
        if (_disconnected_cb) {
            _disconnected_cb(conn);
        }
        // NOTE: The release should be at the end
        (void)_connections.erase(item);
    }
    else
    {
        LOG(ERROR) << "error in TcpServer::handle_disconnected, "
                << "the connection was not found from the connection collection " << std::endl;
    }
}

void TcpServer::handle_message(TcpConnPtr conn, const uint8_t *data, size_t size)
{
    LOG(DEBUG) <<_name << " receives data\n";
#ifdef TINYNET_DEBUG
    auto item = _connections.find(conn->get_fd());
    if (item == _connections.end())
    {
        LOG(ERROR) << "error in TcpServer::handle_message, "
                << "the connection was not found from the connection collection " << std::endl;

    }
    else
#endif
    {
        if (_on_message_cb)
        {
            _on_message_cb(conn, data, size);
        }
    }
}

void TcpServer::handle_write_complete(TcpConnPtr conn)
{
    LOG(INFO) << "the write operation to connect " << conn->get_client_ip() << ":" << conn->get_client_port() << " is complete\n";
#ifdef TINYNET_DEBUG
    auto item = _connections.find(conn->get_fd());
    if (item == _connections.end())
    {
        LOG(ERROR) << "error in TcpServer::handle_write_complete, "
                << "the connection was not found from the connection collection " << std::endl;

    }
    else
#endif
    {
        if (_write_complete_cb)
        {
            _write_complete_cb(conn);
        }
    }
}

}  // namespace tinynet

