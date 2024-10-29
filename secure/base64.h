#ifndef _TINYNET_BASE64_H_
#define _TINYNET_BASE64_H_


#include <cstdint>
#include <cstring>

namespace secure
{

void to_base64(uint8_t *in, uint32_t in_len, char *out, uint32_t out_buffer_size);

}

#endif // _TINYNET_BASE64_H_