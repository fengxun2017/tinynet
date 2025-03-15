#ifndef _TINYNET_IO_CHANNEL_H_
#define _TINYNET_IO_CHANNEL_H_

#include <functional>
#include <memory>
#include <string>
#include "io_channel_interface.h"
#include "io_poller_interface.h"

namespace tinynet
{

class IoChannel final : public IoChannelInterface
{
public:
    enum ChannelState { IN_POLLER, NOTIN_POLER };

    IoChannel(int fd, std::shared_ptr<IoPollerInterface> poller, std::string name);
    ~IoChannel() override;

    void set_read_callback(ChannelEventCallback read_cb) override {_read_cb = read_cb;}
    void set_write_callback(ChannelEventCallback write_cb) override {_write_cb = write_cb;}
    void set_close_callback(ChannelEventCallback close_cb) override {_close_cb = close_cb;}
    void set_error_callback(ChannelEventCallback error_cb) override {_error_cb = error_cb;}

    void handle_event(void) override;

    int enable_read(void) override;
    int enable_write(void) override;
    int disable_read(void) override;
    int disable_write(void) override;
    int disable_all(void) override;
    bool is_writing(void) override;

    int get_fd(void) override {return _fd;}
    void set_events_received(int events) override {_events_received = events;}
    std::string get_name(void) override {return _name;}

    uint32_t get_events_interested(void) override {return _events_interested;}
    uint32_t get_events_received(void) override {return _events_received;}

private:
    int update_poll_cfg(void);

    const int  _fd;
    std::string _name;
    uint32_t        _events_interested = 0;
    uint32_t        _events_received = 0;
    ChannelState   _state;

    std::shared_ptr<IoPollerInterface> _poller = nullptr;
    ChannelEventCallback _read_cb = nullptr;
    ChannelEventCallback _write_cb = nullptr;
    ChannelEventCallback _close_cb = nullptr;
    ChannelEventCallback _error_cb = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_IO_CHANNEL_H_