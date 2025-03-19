#include "can.h"

namespace tinynet
{

void Can::write_data(uint32_t can_id, const void* buffer, size_t length)
{
    _socket.write_can_data(can_id, buffer, length);
}

bool Can::bind(std::string canif)
{
    return _socket.bind(canif, 0);
}



} // namespace tinynet