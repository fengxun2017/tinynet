#include <vector>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include "ws_connection.h"
#include "logging.h"

namespace tinynet
{


WebSocketConnection::WebSocketConnection(TcpConnPtr conn)
    :_tcp_conn(conn),
    _frame_state(WAIT_FIN_AND_OPCODE)
{
    _payload_buffer.reserve(100);
}

void WebSocketConnection::reset_recv_state(void)
{
    _frame_state = WAIT_FIN_AND_OPCODE;
    _payload_buffer.clear();
    _recv_count = 0;
}

bool is_recv_complete(void)
{
    bool ret = false;
    if (RECV_COMPLETE == _frame_state)
    {
        ret = true;
    }

    return ret;
}

void WebSocketConnection::handle_recv_data(uint8_t *data, size_t len)
{
    process_input(data, len);
    if (is_recv_complete())
    {
        /* Messages from the client must be masked, so your server must expect this to be 1. */
        if (!_header.mask)
        {
            LOG(ERROR) << "Messages from the client must be masked." << std::endl;
            websocket_disconn(WebSocket::POLICY_VIOLATION, "There are no mask markers");
            reset_recv_state();
        }
        else
        {
            if (WebSocket::OPCODE_PING == _header.opcode)
            {
                write_data(WebSocket::OPCODE_PONG, (uint8_t *)_payload_buffer.data(), _payload_buffer.size());
            }
            else if (WebSocket::OPCODE_PONG == _header.opcode)
            {
                // ignore
            }
            else
            {
                
            }
        }
    }
}

void WebSocketConnection::websocket_disconn(uint16_t stat_code, std::string reason)
{
    std::vector<uint8_t> payload;
    
    payload.push_back((statecode>>8)&0xff);
    payload.push_back(statecode&0xff);
    payload.insert(payload.end(), reason.begin(), reason.end());
    write_data(WebSocket::OPCODE_CLOSE, (uint8_t *)payload.data(), payload.size());
    _tcp_conn->disable_conn();
}


void WebSocketConnection::process_input(uint8_t *data, size_t len)
{
    size_t i = 0;
    bool ret = true;

    if (NULL != data)
    {
        while (i++ < len)
        {
            switch (_frame_state)
            {
                case WAIT_FIN_AND_OPCODE:
                    LOG(INNER_DEBUG) << "websocket:WAIT_FIN_AND_OPCODE" << std::endl
                    _header.fin = data[i] & 0x80;
                    _header.opcode = data[i] & 0x0F;
                    _frame_state = WAIT_MASK_AND_LEN;
                break;

                case WAIT_MASK_AND_LEN:
                    LOG(INNER_DEBUG) << "websocket:WAIT_MASK_AND_LEN" << std::endl
                    _header.mask = data[i]&0x80;
                    uint8_t len = data[i]&0x7F;

                    _recv_count = 0;
                    if (len <= 125)
                    {
                        _header.payload_length = len;
                        _frame_state = WAIT_MASK_KEY;
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
                    LOG(INNER_DEBUG) << "websocket:WAIT_EXT_LEN" << std::endl
                    _recv_count++;
                    _header.payload_length = (_header.payload_length << 8) + data[i];
                    if (_recv_count >= _ext_payload_len)
                    {
                        _frame_state = WAIT_MASK_KEY;
                        _recv_count = 0;
                    }
                break;

                case WAIT_MASK_KEY:
                    LOG(INNER_DEBUG) << "websocket:WAIT_MASK_KEY" << std::endl
                    _header.masking_key[_recv_count++] = data[i];
                    if (_recv_count >= 4)
                    {
                        _frame_state = WAIT_PAYLOAD;
                        _recv_count = 0;
                    }
                break;

                case WAIT_PAYLOAD:
                    LOG(INNER_DEBUG) << "websocket:WAIT_PAYLOAD" << std::endl
                    _payload_buffer[_recv_count] = data[i] ^ _header.masking_key[_recv_count & 3];
                    _recv_count++;
                    if (_recv_count >= _header.payload_length)
                    {
                        if (_header.fin)
                        {
                            _frame_state = RECV_COMPLETE;
                        }
                        else
                        {
                            // only reset state, to receive follow-up packages
                            _frame_state = WAIT_FIN_AND_OPCODE;
                        }
                    }
                break;

                case RECV_COMPLETE:
                    LOG(ERROR) << "The data has been fully received"
                break;

                default:
                    LOG(ERROR) << "websocket frame recv state error" << std::endl;
                    reset_recv_state();
                break;
            }
        }
    }
    else
    {
        LOG(ERROR) << "data is NULL in WebSocketConnection::handle_recv_data." << std::endl;
    }

    return 
}


void WebSocketConnection::write_data(WebSocket::OpCode opcode = WebSocket::OPCODE_TEXT, const uint8_t* data, size_t size, bool fin = true)
{
    std::vector<uint8_t> frame;
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

    if (size <= 125)
    {
        frame.push_back(size);
    }
    else if(size <= 65535)
    {
        frame.push_back(126);
        uint16_t len = htobe16(size);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 2);
    }
    else
    {
        frame.push_back(127);
        uint64_t len = htobe64(size);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 8);
    }

    _tcp_conn->write_data(frame.data(), frame.size());
    _tcp_conn->write_data(data, size);
}


} // tinynet
