#include "can.h"

namespace tinynet
{

void Can::write_data(uint32_t can_id, const void* buffer, size_t length)
{
    _sockfd.write_can_data(can_id, buffer, length);
}



} // namespace tinynet