#ifndef _TINYNET_CAN_H_
#define _TINYNET_CAN_H_
#include "io_socket.h"
#include <string>

namespace tinynet
{

class Can
{
public:
    Can(std::string name) : _socket(name, IoSocket::CAN) {}
    ~Can() {};
    void write_data(uint32_t can_id, const void* buffer, size_t length);
    bool bind(std::string canif);

private:
    IoSocket _socket;
};

} // namespace tinynet

#endif // _TINYNET_CAN_H_