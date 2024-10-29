#include "SHA1.h"

namespace secure
{

const size_t SHA1::SHA1_OUT_SIZE = 20;

SHA1::SHA1()
{
    reset();
}

bool SHA1::compute_sha1(const uint8_t* in, size_t in_len, uint8_t* out, size_t out_size)
{
    if (out_size < SHA1::SHA1_OUT_SIZE)
    {
        return false;
    }

    SHA1 sha1;
    sha1.update(in, in_len);
    sha1.final(out);

    return true;
}

void SHA1::update(const uint8_t* data, uint64_t len)
{
    for (uint64_t i = 0; i < len; ++i) {
        data_[datalen_++] = data[i];
        if (datalen_ == 64) {
            transform();
            bitlen_ += 512;
            datalen_ = 0;
        }
    }
}

void SHA1::final(uint8_t* hash)
{
    uint64_t i = datalen_;

    // Padding
    if (datalen_ < 56) {
        data_[i++] = 0x80;
        while (i < 56) {
            data_[i++] = 0x00;
        }
    } else {
        data_[i++] = 0x80;
        while (i < 64) {
            data_[i++] = 0x00;
        }
        transform();
        memset(data_, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform
    bitlen_ += datalen_ * 8;
    data_[63] = bitlen_;
    data_[62] = bitlen_ >> 8;
    data_[61] = bitlen_ >> 16;
    data_[60] = bitlen_ >> 24;
    data_[59] = bitlen_ >> 32;
    data_[58] = bitlen_ >> 40;
    data_[57] = bitlen_ >> 48;
    data_[56] = bitlen_ >> 56;
    transform();

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
            hash[i + (j * 4)] = (state_[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}


void SHA1::reset()
{
    state_[0] = 0x67452301;
    state_[1] = 0xEFCDAB89;
    state_[2] = 0x98BADCFE;
    state_[3] = 0x10325476;
    state_[4] = 0xC3D2E1F0;
    datalen_ = 0;
    bitlen_ = 0;
}

void SHA1::transform()
{
    uint32_t a, b, c, d, e, f, k, temp;
    uint32_t m[80];
    for (uint32_t i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = (data_[j] << 24) | (data_[j + 1] << 16) | (data_[j + 2] << 8) | (data_[j + 3]);
    }
    for (uint32_t i = 16; i < 80; ++i) {
        m[i] = leftRotate(m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16], 1);
    }
    a = state_[0];
    b = state_[1];
    c = state_[2];
    d = state_[3];
    e = state_[4];
    for (uint32_t i = 0; i < 80; ++i) {
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        temp = leftRotate(a, 5) + f + e + k + m[i];
        e = d;
        d = c;
        c = leftRotate(b, 30);
        b = a;
        a = temp;
    }
    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
}

uint32_t SHA1::leftRotate(uint32_t x, uint32_t c)
{
    return (x << c) | (x >> (32 - c));
}

}