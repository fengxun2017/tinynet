#ifndef _TINYNET_WEBSOCKET_CONNECTION_H_
#define _TINYNET_WEBSOCKET_CONNECTION_H_
#include <vector>
#include <memory>
#include "tcp_connection.h"
#include "ws_common.h"

namespace tinynet
{

class WebSocketConnection;
using WsConnPtr = std::shared_ptr<WebSocketConnection>;
class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection> 
{
public:

    WebSocketConnection(TcpConnPtr conn, std::string name);
    ~WebSocketConnection(){};

    std::string get_name(void) {return _name;}
    void write_data(const uint8_t* data, size_t size, WebSocket::OpCode opcode = WebSocket::OPCODE_TEXT, bool fin = true);
    void websocket_disconn(uint16_t statcode, std::string reason);
    friend class WebSocketServer;
private:
    enum WebSocketFrameRecvState
    {
        WAIT_FIN_AND_OPCODE = 0,
        WAIT_MASK_AND_LEN,
        WAIT_EXT_LEN,
        WAIT_MASK_KEY,
        WAIT_PAYLOAD,
        RECV_COMPLETE
    };
    struct WebSocketFrameHeader
    {
        bool fin;
        uint8_t opcode;
        bool mask;
        uint64_t payload_length;
        uint8_t masking_key[4];
    };
    void reset_recv_state(void);
    void process_input(const uint8_t *data, size_t size);
    bool is_recv_complete(void);
    void handle_recv_data(const uint8_t *data, size_t len, std::function<void(WsConnPtr&, const uint8_t *data, size_t size)> user_cb);

    TcpConnPtr _tcp_conn;
    std::vector<uint8_t> _payload_buffer;
    WebSocketFrameRecvState _frame_state;
    uint8_t _ext_payload_len;
    uint8_t _recv_count;
    WebSocketFrameHeader _header;
    bool _packet_recv_complate;
    std::string _name;

};

} // tinynet


#endif // _TINYNET_WEBSOCKET_CONNECTION_H_