#include <vector>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <functional>
#include "ws_connection.h"
#include "logging.h"

namespace tinynet
{


WebSocketConnection::WebSocketConnection(TcpConnPtr conn, std::string name, bool used_for_server)
    :_tcp_conn(conn),
    _frame_state(WAIT_FIN_AND_OPCODE),
    _name(name),
    _used_for_server(used_for_server)
{
    _payload_buffer.reserve(100);
}

void WebSocketConnection::reset_recv_state(void)
{
    _frame_state = WAIT_FIN_AND_OPCODE;
    _payload_buffer.clear();
    _recv_count = 0;
}

bool WebSocketConnection::is_recv_complete(void)
{
    bool ret = false;
    if (RECV_COMPLETE == _frame_state)
    {
        ret = true;
    }

    return ret;
}

void WebSocketConnection::handle_recv_data(const uint8_t *data, size_t len, std::function<void(WsConnPtr &, const uint8_t *data, size_t size)> user_cb)
{
    if (NULL != data)
    {
        for (int i = 0; i < len; i++)
        {
            process_input(data[i]);
            if (is_recv_complete())
            {
                LOG(INNER_DEBUG) << "recv complete, process websocket payload" << std::endl;
                /* Messages from the client must be masked, so your server must expect this to be 1. */
                if ((!_header.mask) && (_used_for_server))
                {
                    LOG(ERROR) << "Messages from the client must be masked." << std::endl;
                    websocket_disconn(WebSocket::POLICY_VIOLATION, "There are no mask markers");
                    reset_recv_state();
                }
                else
                {
                    if (WebSocket::OPCODE_PING == _header.opcode)
                    {
                        LOG(INNER_DEBUG) << "recv PING, resp PONG" << std::endl;
                        write_data( (uint8_t *)_payload_buffer.data(), _payload_buffer.size(), WebSocket::OPCODE_PONG);
                    }
                    else if (WebSocket::OPCODE_PONG == _header.opcode)
                    {
                        LOG(INNER_DEBUG) << "recv OPCODE_PONG" << std::endl;
                        // ignore
                    }
                    else if(WebSocket::OPCODE_CONT == _header.opcode)
                    {
                        LOG(ERROR) << "There shouldn't be this opcode here" << std::endl;
                    }
                    else if(WebSocket::OPCODE_CLOSE == _header.opcode)
                    {
                        LOG(INFO) << "The peer initiates a disconnection, close code:" <<(_payload_buffer.data()[0]<<8)+_payload_buffer.data()[1]
                                << " close reason: " << std::string(reinterpret_cast<char *>(&_payload_buffer.data()[2]), _payload_buffer.size()-2) << std::endl;
                    }
                    else
                    {
                        auto ws_conn = shared_from_this();
                        user_cb(ws_conn, static_cast<uint8_t *>(_payload_buffer.data()), _payload_buffer.size());

                        // process done, reset state
                        reset_recv_state();
                    }
                }
            }
        }
    }
    else
    {
        LOG(ERROR) << "data is NULL in WebSocketConnection::process_input." << std::endl;
    }
}

void WebSocketConnection::websocket_disconn(uint16_t stat_code, std::string reason)
{
    std::vector<uint8_t> payload;
    
    payload.push_back((stat_code>>8)&0xff);
    payload.push_back(stat_code&0xff);
    payload.insert(payload.end(), reason.begin(), reason.end());
    write_data((uint8_t *)payload.data(), payload.size(), WebSocket::OPCODE_CLOSE);
    _tcp_conn->disable_conn();
}


void WebSocketConnection::process_input(uint8_t data)
{
    uint8_t len;
    uint8_t opcode;
    switch (_frame_state)
    {
        case WAIT_FIN_AND_OPCODE:
            // LOG(INNER_DEBUG) << "websocket:WAIT_FIN_AND_OPCODE " << static_cast<int>(data) << std::endl;
            _header.fin = data & 0x80;
            opcode = data & 0x0F;
            if (WebSocket::OPCODE_CONT != opcode)
            {
                _header.opcode = opcode;
            }
            _frame_state = WAIT_MASK_AND_LEN;
        break;

        case WAIT_MASK_AND_LEN:
            // LOG(INNER_DEBUG) << "websocket:WAIT_MASK_AND_LEN" << static_cast<int>(data) << std::endl;
            _header.mask = data&0x80;
            len = data&0x7F;

            _recv_count = 0;
            if (len <= 125)
            {
                _header.payload_length = len;
                if (_header.mask)
                {
                    _frame_state = WAIT_MASK_KEY;
                }
                else
                {
                    _frame_state = WAIT_PAYLOAD;
                }
                // LOG(INNER_DEBUG) << "websocket payload length = " << _header.payload_length << std::endl;
            }
            else
            {
                _frame_state = WAIT_EXT_LEN;
                _header.payload_length = 0;
                if (len == 126)
                {
                    _ext_payload_len = 2;

                }
                else if (len == 127)
                {
                    _ext_payload_len = 8;
                }
            }
        break;

        case WAIT_EXT_LEN:
            // LOG(INNER_DEBUG) << "websocket:WAIT_EXT_LEN" << static_cast<int>(data) << std::endl;
            _recv_count++;
            _header.payload_length = (_header.payload_length << 8) + data;
            if (_recv_count >= _ext_payload_len)
            {
                if (_header.mask)
                {
                    _frame_state = WAIT_MASK_KEY;
                }
                else
                {
                    _frame_state = WAIT_PAYLOAD;
                }
                _recv_count = 0;
            }
        break;

        case WAIT_MASK_KEY:
            // LOG(INNER_DEBUG) << "websocket:WAIT_MASK_KEY" << static_cast<int>(data) << std::endl;
            _header.masking_key[_recv_count++] = data;
            if (_recv_count >= 4)
            {
                _frame_state = WAIT_PAYLOAD;
                _recv_count = 0;
            }
        break;

        case WAIT_PAYLOAD:
            if (_header.mask)
            {
                _payload_buffer.push_back(data ^ _header.masking_key[_recv_count & 3]);
            }
            else
            {
                _payload_buffer.push_back(data);
            }
            // LOG(INNER_DEBUG) << "websocket:WAIT_PAYLOAD: " << _payload_buffer[_recv_count] << std::endl;

            _recv_count++;
            if (_recv_count >= _header.payload_length)
            {
                // FIXME: should check fin and opcode
                if (_header.fin)
                {
                    _frame_state = RECV_COMPLETE;
                }
                else
                {
                    // only reset state, to receive follow-up packages
                    LOG(INNER_DEBUG) << "only reset state, to receive follow-up packages" << std::endl;
                    _frame_state = WAIT_FIN_AND_OPCODE;
                }
            }
        break;

        case RECV_COMPLETE:
            LOG(ERROR) << "The data has been fully received" << std::endl;
        break;

        default:
            LOG(ERROR) << "websocket frame recv state error" << std::endl;
            reset_recv_state();
        break;
    }

}


void WebSocketConnection::write_data(const uint8_t* data, size_t size, WebSocket::OpCode opcode, bool fin)
{
    std::vector<uint8_t> frame;

    // With all-zero, you can do without XOR operations
    uint8_t masking_key[4] =  {0,0,0,0};
    /* The frame header information can be up to 14 bytes */
    frame.reserve(14);

    if ((opcode < WebSocket::OPCODE_CONT) || (opcode > WebSocket::OPCODE_PONG))
    {
        LOG(ERROR) << "opcode not support" << std::endl;
        return ;
    }
    /* Fragmentation is only available on opcodes 0x0 to 0x2 */
    if (opcode > WebSocket::OPCODE_BINARY)
    {
        fin = true;
    }

    uint8_t first_byte = fin ? (0x80 | opcode) : opcode;
    frame.push_back(first_byte);

    uint8_t mask_flag = _used_for_server ? 0x00 : 0x80;
    if (size <= 125)
    {
        frame.push_back(size|mask_flag);
    }
    else if(size <= 65535)
    {
        frame.push_back(126|mask_flag);
        uint16_t len = htobe16(size);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 2);
    }
    else
    {
        frame.push_back(127|mask_flag);
        uint64_t len = htobe64(size);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 8);
    }

    if (mask_flag)
    {
        frame.insert(frame.end(), &masking_key[0], &masking_key[4]);
    }

    _tcp_conn->write_data(frame.data(), frame.size());
    _tcp_conn->write_data(data, size);
}


} // tinynet

