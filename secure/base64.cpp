#include "base64.h"

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

void to_base64(uint8_t *in, uint32_t in_len, char *out, uint32_t out_buffer_size) {
    uint32_t i = 0, j = 0;
    uint8_t char_array_3[3], char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(in++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                if (j < out_buffer_size - 1) {
                    out[j++] = base64_chars[char_array_4[i]];
                }
            }
            i = 0;
        }
    }

    if (i) {
        for (uint32_t k = i; k < 3; k++) {
            char_array_3[k] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (uint32_t k = 0; (k < i + 1); k++) {
            if (j < out_buffer_size - 1) {
                out[j++] = base64_chars[char_array_4[k]];
            }
        }

        while ((i++ < 3)) {
            if (j < out_buffer_size - 1) {
                out[j++] = '=';
            }
        }
    }

    if (j < out_buffer_size) {
        out[j] = '\0';
    }
}
