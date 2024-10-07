#ifndef _TINYNET_IO_CHANNEL_H_
#define _TINYNET_IO_CHANNEL_H_

#include "io_poller.h"
#include <functional>
#include <memory>
#include <string>

namespace tinynet
{

class IoChannel
{

public:
    enum ChannelState { IN_POLLER, NOTIN_POLER };
    using EventCallback = std::function<void(void)> ;

    IoChannel(int fd, std::shared_ptr<IoPoller> &poller, std::string name);
    ~IoChannel();

    void set_reab_callback(EventCallback read_cb) {_read_cb = read_cb;}
    void set_write_callback(EventCallback write_cb) {_write_cb = write_cb;}
    void set_close_callback(EventCallback close_cb) {_read_cb = close_cb;}
    void set_error_callback(EventCallback error_cb) {_error_cb = error_cb;}

    void handle_event(void);

    void enable_read(void);
    void enable_write(void);
    void disable_read(void);
    void disable_write(void);
    void disable_all(void);
    int get_fd(void) {return _fd;}
    void set_events_received(int events) {_events_received = events;}
    std::string get_name(void) {return _name;}

private:
    void update_poll_cfg(void);

    const int  _fd;
    std::string _name;
    uint32_t        _events_interested = 0;
    uint32_t        _events_received = 0;
    ChannelState   _state;
    
    std::shared_ptr<IoPoller> _poller = nullptr;
    EventCallback _read_cb = nullptr;
    EventCallback _write_cb = nullptr;
    EventCallback _close_cb = nullptr;
    EventCallback _error_cb = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_IO_CHANNEL_H_