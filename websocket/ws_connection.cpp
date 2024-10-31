#include <vector>
#include <cstdint>
#include <cstring>
#include "ws_connection.h"
#include "logging.h"

namespace tinynet
{

void WebSocketConnection::write_data(const uint8_t* data, size_t size, WebSocket::Opcode opcode = WebSocket::OPCODE_TEXT, bool fin = true)
{
    std::vector<uint8_t> frame;
    uint8_t head_buffer[14];
    uint8_t head_len = 0;

    if ((opcode < WebSocket::OPCODE_CONT) || (opcode > WebSocket::OPCODE_PONG))
    {
        LOG(ERROR) << "opcode not support" << std::endl;
        return ;
    }

    head_buffer[head_len++] = fin ? (0x80 | opcode) : opcode;
    if (size <= 125)
    {
        head_buffer[head_len++] = size;
    }
    else if(size <= 65535)
    {
        head_buffer[head_len++] = 126;
        head_buffer[head_len++] = (size>>8)&0xff;
        head_buffer[head_len++] = size&0xff;

        uint16_t len = htons(size);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 2);
    }
    else
    {
        uint64_t len = size;
        head_buffer[head_len++] = 127;
        head_buffer[head_len++] = (len>>56)&0xff;
        head_buffer[head_len++] = (len>>48)&0xff;
        head_buffer[head_len++] = (len>>40)&0xff;
        head_buffer[head_len++] = (len>>32)&0xff;
        head_buffer[head_len++] = (len>>24)&0xff;
        head_buffer[head_len++] = (len>>16)&0xff;
        head_buffer[head_len++] = (len>>8)&0xff;
        head_buffer[head_len++] = (len)&0xff;

        frame.push_back(127);
        frame.insert(frame.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 8);
    }

    frame.insert(frame.end(), data, data + size);

    _tcp_conn->write_data(frame.data(), frame.size());
}


} // tinynet

