#include <vector>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include "ws_connection.h"
#include "logging.h"

namespace tinynet
{

void WebSocketConnection::handle_recv_data(uint8_t *data, size_t len)
{
    /* Messages from the client must be masked, so your server must expect this to be 1. */
}


void WebSocketConnection::write_data(const uint8_t* data, size_t size, WebSocket::Opcode opcode = WebSocket::OPCODE_TEXT, bool fin = true)
{
    std::vector<uint8_t> frame;
    /* The frame header information can be up to 14 bytes */
    frame.reserve(14);

    if ((opcode < WebSocket::OPCODE_CONT) || (opcode > WebSocket::OPCODE_PONG))
    {
        LOG(ERROR) << "opcode not support" << std::endl;
        return ;
    }
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

