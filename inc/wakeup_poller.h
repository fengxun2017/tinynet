#ifndef WAKEUP_POLLER_H_
#define WAKEUP_POLLER_H_
#include <memory>
#include <string>
#include "wakeup_poller_interface.h"
#include "io_poller_interface.h"
#include "io_channel_interface.h"

namespace tinynet {

class PollerWakeup final : public PollerWakeupInterface 
{
public:
    PollerWakeup(std::shared_ptr<IoPollerInterface>& _poller, std::string name);
    ~PollerWakeup() override;
    void wakeup() override;

private:
    void handle_recv(void);
    int create_eventfd(void);

    int _event_fd;
    std::unique_ptr<IoChannelInterface> _channel;
};

}
#endif  // WAKEUP_POLLER_H_