#ifndef _TINYNET_WEBSOCKET_CONNECTION_H_
#define _TINYNET_WEBSOCKET_CONNECTION_H_

#include "tcp_connection.h"
#include "ws_common.h"

namespace tinynet
{

class WebSocketConnection {
public:
    WebSocketConnection(TcpConnPtr conn):_tcp_conn(conn) {_data_buffer.reserve(14);}
    ~WebSocketConnection(){};

    void write_data(const uint8_t* data, size_t size, WebSocket::Opcode opcode = WebSocket::OPCODE_TEXT, bool fin = true);
private:
    void handle_recv_data(uint8_t *data, size_t len);
    TcpConnPtr _tcp_conn;
    std::vector<uint8_t> _data_buffer;
};

} // tinynet


#endif // _TINYNET_WEBSOCKET_CONNECTION_H_