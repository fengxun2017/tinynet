#ifndef _TINYNET_WS_COMMON_H_
#define _TINYNET_WS_COMMON_H_

namespace tinynet
{

class WebSocket
{
public:
enum Opcode
{
    OPCODE_CONT = 0x0,
    OPCODE_TEXT = 0x1,
    OPCODE_BINARY = 0x2,
    OPCODE_CLOSE = 0x08,
    OPCODE_PING = 0x9,
    OPCODE_PONG = 0xA
};

};


}


#endif // _TINYNET_WS_COMMON_H_