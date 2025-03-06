#include <unistd.h>
#include "wakeup_poller.h"
#include <sys/eventfd.h>
#include "tinynet_util.h"
#include "logging.h"
#include "io_channel.h"

namespace tinynet {
PollerWakeup::~PollerWakeup()
{
    close(_event_fd);
}

int PollerWakeup::create_eventfd(void)
{
    int event_fd = eventfd(0, 0);
    if (event_fd == -1) 
    { 
        LOG(ERROR) << "EventLoop::create_eventfd failed, error info:"
            << error_to_str(errno)
            << std::endl;

        std::abort();
    }
    return event_fd;
}

PollerWakeup::PollerWakeup(std::shared_ptr<IoPollerInterface>& _poller, std::string name)
{
    _event_fd = create_eventfd(),
    _channel = std::make_unique<IoChannel>(_event_fd, _poller, name);

    _channel->set_read_callback(std::bind(&PollerWakeup::handle_recv, this));

}
void PollerWakeup::wakeup()
{
    uint64_t value = 1;

    LOG(DEBUG) << "wakeup eventloop" << std::endl;
    ssize_t result = ::write(_event_fd, &value, sizeof(value));
    if (result == -1) {
        LOG(ERROR) << "PollerWakeup::wakeup failed, error info:"
            << error_to_str(errno)
            << std::endl;
    }
}

void PollerWakeup::handle_recv(void)
{
    uint64_t value;
    ssize_t result = ::read(_event_fd, &value, sizeof(value));
    if (result == -1) {
        LOG(ERROR) << "PollerWakeup::handle_recv failed, error info:" 
            << error_to_str(errno)
            << std::endl;
    }
}

}