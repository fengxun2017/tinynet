#ifndef _TINYNET_CHANNEL_INTERFACE_H_
#define _TINYNET_CHANNEL_INTERFACE_H_

#include <functional>
#include <string>

namespace tinynet
{

class IoChannelInterface
{
public:
    using ChannelEventCallback = std::function<void(void)> ;
    virtual ~IoChannelInterface() = default;

    virtual void set_read_callback(ChannelEventCallback read_cb) = 0;
    virtual void set_write_callback(ChannelEventCallback write_cb) = 0;
    virtual void set_close_callback(ChannelEventCallback close_cb) = 0;
    virtual void set_error_callback(ChannelEventCallback error_cb) = 0;

    virtual void handle_event(void) = 0;

    virtual void enable_read(void) = 0;
    virtual void enable_write(void) = 0;
    virtual void disable_read(void) = 0;
    virtual void disable_write(void) = 0;
    virtual void disable_all(void) = 0;
    virtual bool is_writing(void) = 0;

    virtual int get_fd(void) = 0;
    virtual void set_events_received(int events) = 0;
    virtual std::string get_name(void) = 0;

    virtual uint32_t get_events_interested(void) = 0;
    virtual uint32_t get_events_received(void) = 0;
};

using IoChannels =  std::vector<IoChannelInterface*>;
}  // namespace tinynet

#endif  // _TINYNET_IO_CHANNEL_INTERFACE_H_