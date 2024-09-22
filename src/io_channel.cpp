#include <functional>
#include <sys/epoll.h>
#include <sstream>
#include <memory>
#include "io_poller.h"
#include "io_channel.h"
#include "logging.h"

namespace tinynet
{
IoChannel::IoChannel(int fd, std::shared_ptr<IoPoller> &poller, std::string name)
    : _fd(fd), _poller(poller), _name(name)
{
    _state = NOTIN_POLER;
}

IoChannel::~IoChannel(){};

void IoChannel::enable_read(void)
{
    _events_interested |= (EPOLLIN | EPOLLPRI);
}

void IoChannel::disable_read(void)
{
    _events_interested &= ~(EPOLLIN | EPOLLPRI);
}

void IoChannel::enable_write(void)
{
    _events_interested |= EPOLLOUT;
}
void IoChannel::disable_write(void)
{
    _events_interested &= ~EPOLLOUT;
}

static std::string event_to_string(uint32_t event_mask)
{
  std::ostringstream oss;
  if (event_mask & EPOLLIN) oss << "EPOLLIN ";
  if (event_mask & EPOLLOUT) oss << "EPOLLOUT ";
  if (event_mask & EPOLLRDHUP) oss << "EPOLLRDHUP ";
  if (event_mask & EPOLLPRI) oss << "EPOLLPRI ";
  if (event_mask & EPOLLERR) oss << "EPOLLERR ";
  if (event_mask & EPOLLHUP) oss << "EPOLLHUP ";

  return oss.str();
}
void IoChannel::handle_event(void)
{

    LOG(DEBUG) << "fd:" << _fd << " event:" << event_to_string(_events_received);
    if ((_events_received & (EPOLLIN | EPOLLPRI)) && _read_cb)
    {
        _read_cb();
    }

    if ((_events_received & EPOLLOUT) && _write_cb)
    {
        _write_cb();
    }
}
}  // namespace tinynet
