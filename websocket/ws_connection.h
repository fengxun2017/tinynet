#ifndef _TINYNET_WEBSOCKET_CONNECTION_H_
#define _TINYNET_WEBSOCKET_CONNECTION_H_
#include <vector>
#include "tcp_connection.h"
#include "ws_common.h"

namespace tinynet
{

class WebSocketConnection {

public:


    ~WebSocketConnection(){};

    void write_data(WebSocket::OpCode opcode = WebSocket::OPCODE_TEXT, const uint8_t* data, size_t size, bool fin = true);
    bool handle_recv_data(uint8_t *data, size_t len);
    void websocket_disconn(uint16_t statcode, std::string reason);
private:
    enum WebSocketFrameRecvState
    {
        WAIT_FIN_AND_OPCODE = 0,
        WAIT_MASK_AND_LEN,
        WAIT_EXT_LEN,
        WAIT_MASK_KEY,
        WAIT_PAYLOAD,
        RECV_COMPLETE
    }
    struct WebSocketFrameHeader
    {
        bool fin;
        uint8_t opcode;
        bool mask;
        uint64_t payload_length;
        uint8_t masking_key[4];
    };
    void reset_recv_state(void);
    void process_input(uint8_t *data, size_t len);
    bool is_recv_complete(void);

    TcpConnPtr _tcp_conn;
    std::vector<uint8_t> _payload_buffer;
    WebSocketFrameRecvState _frame_state;
    uint8_t _ext_payload_len;
    uint8_t _recv_count;
    WebSocketFrameHeader _header;
    bool _packet_recv_complate;


};

} // tinynet


#endif // _TINYNET_WEBSOCKET_CONNECTION_H_