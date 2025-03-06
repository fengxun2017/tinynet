#ifndef WAKEUP_POLLER_INTERFACE_H_
#define WAKEUP_POLLER_INTERFACE_H_

namespace tinynet {

class PollerWakeupInterface {
public:
    virtual ~PollerWakeupInterface() = default;
    virtual void wakeup() = 0;
};

}
#endif  // WAKEUP_POLLER_INTERFACE_H_