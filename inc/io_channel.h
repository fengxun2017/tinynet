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
    typedef std::function<void(void)> EventCallback;

    IoChannel(int fd, std::shared_ptr<IoPoller> &poller, std::string name);
    ~IoChannel();

    void register_reab_callback(EventCallback read_cb) {_read_cb = read_cb;};
    void register_write_callback(EventCallback write_cb) {_write_cb = write_cb;};
    void register_close_callback(EventCallback close_cb) {_read_cb = close_cb;};
    void register_error_callback(EventCallback error_cb) {_error_cb = error_cb;};

    void handle_event(void);

    void enable_read(void);
    void enable_write(void);
    void disable_read(void);
    void disable_write(void);

    int get_fd(void) {return _fd;}
    void set_events_received(int events) {_events_received = events;}

private:
    const int  _fd;
    uint32_t        _events_interested;
    uint32_t        _events_received;
    std::string     _name;
    ChannelState   _state;
    
    std::shared_ptr<IoPoller> _poller = nullptr;
    EventCallback _read_cb = nullptr;
    EventCallback _write_cb = nullptr;
    EventCallback _close_cb = nullptr;
    EventCallback _error_cb = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_CHANNEL_H_