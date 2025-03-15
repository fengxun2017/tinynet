#ifndef WAKEUP_POLLER_H_
#define WAKEUP_POLLER_H_
#include <memory>
#include <string>
#include <type_traits>
#include <functional>
#include "wakeup_poller_interface.h"
#include "io_poller_interface.h"
#include "io_channel_interface.h"

namespace tinynet {

class PollerWakeup final : public PollerWakeupInterface 
{
public:
    using WakeupCallback = std::function<void(int)>;
    PollerWakeup(std::shared_ptr<IoPollerInterface> _poller, std::string name, WakeupCallback cb=nullptr);
    ~PollerWakeup() override;
    void wakeup() override;
    void handle_recv(void);

private:
    int create_eventfd(void);

    int _event_fd;
    std::unique_ptr<IoChannelInterface> _channel;
    WakeupCallback _callback;
};

}
#endif  // WAKEUP_POLLER_H_