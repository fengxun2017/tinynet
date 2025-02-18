#ifndef _TINYNET_RING_BUFFER_H_
#define _TINYNET_RING_BUFFER_H_

#include <vector>
#include <cstdio>   // std::memcpy
#include <algorithm> // std::min
#include <cstring>
namespace tinynet {

// Non-thread-safe
template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : _buffer(capacity), _write_index(0), _read_index(0), _size(0) {}

    void append(const T* data, size_t n)
    {
        if (n > available_space())
        {
            resize(_buffer.size() + n - available_space());
        }

        size_t first_part = std::min(n, _buffer.size() - _write_index);
        std::memcpy(_buffer.data() + _write_index, data, first_part * sizeof(T));

        size_t remaining = n - first_part;
        if (remaining > 0)
        {
            std::memcpy(_buffer.data(), data + first_part, remaining * sizeof(T));
        }

        _write_index = (_write_index + n) % _buffer.size();
        _size += n;
    }

    T* get_read_pointer()
    {
        return &(_buffer[_read_index]);
    }

    size_t current_size() const
    {
        return _size;
    }

    void confirm_consume(size_t m)
    {
        if (m > _size)
        {
            m = _size;
        }
        _read_index = (_read_index + m) % _buffer.size();
        _size -= m;
    }

private:
    size_t available_space() const
    {
        return _buffer.size() - _size;
    }

    void resize(size_t new_capacity)
    {
        if (new_capacity <= _buffer.size())
        {
            return;
        }

        std::vector<T> new_buffer(new_capacity);
        if (_size > 0)
        {
            if (_read_index < _write_index)
            {
                std::memcpy(new_buffer.data(), _buffer.data() + _read_index, _size * sizeof(T));
            }
            else
            {
                size_t first_part = _buffer.size() - _read_index;
                std::memcpy(new_buffer.data(), _buffer.data() + _read_index, first_part * sizeof(T));
                std::memcpy(new_buffer.data() + first_part, _buffer.data(), _write_index * sizeof(T));
            }
        }

        _buffer = std::move(new_buffer);
        _read_index = 0;
        _write_index = _size;
    }

    std::vector<T> _buffer;
    size_t _write_index;
    size_t _read_index;
    size_t _size;           // The number of bytes currently cached
};

} // namespace tinynet

#endif // _TINYNET_RING_BUFFER_H_
