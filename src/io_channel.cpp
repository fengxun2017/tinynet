#include <functional>
#include <sys/epoll.h>
#include <sstream>
#include <memory>
#include "io_poller.h"
#include "io_channel.h"
#include "logging.h"

namespace tinynet
{

void IoChannel::update_poll_cfg(void)
{
    if (nullptr == _poller)
    {
        LOG(ERROR) << _name << ":poller is null, in IoChannel::update_poll_cfg" << std::endl;
        return ;
    }

    if (0 != _events_interested)
    {
        if (NOTIN_POLER == _state)
        {
            _poller->add_channel(*this);
            _state = IN_POLLER;
        }
        else
        {
            _poller->update_channel(*this);
        }
    }
    else
    {
        _poller->remove_channel(*this);
        _state = NOTIN_POLER;
    }
}

IoChannel::IoChannel(int fd, std::shared_ptr<IoPoller> &poller, std::string name)
    : _fd(fd), _poller(poller), _name(name)
{
    LOG(DEBUG) << "IoCchannel created: " << _name << std::endl;
    _state = NOTIN_POLER;
}

IoChannel::~IoChannel()
{
    LOG(DEBUG) << "IoChannel:" << _name << " destructor." << std::endl;
}

void IoChannel::disable_all(void)
{
    LOG(INFO) << _name << ":disable_all" << std::endl;
    _events_interested = 0;
    update_poll_cfg();
}

void IoChannel::enable_read(void)
{
    LOG(INFO) << _name << ":enable read" << std::endl;
    _events_interested |= (EPOLLIN | EPOLLPRI);
    update_poll_cfg();
}

void IoChannel::disable_read(void)
{
    LOG(INFO) << _name << ":disable read" << std::endl;
    _events_interested &= ~(EPOLLIN | EPOLLPRI);
    update_poll_cfg();
}

void IoChannel::enable_write(void)
{
    LOG(INFO) << _name << ":enable write" << std::endl;
    _events_interested |= EPOLLOUT;
    update_poll_cfg();
}
void IoChannel::disable_write(void)
{
    LOG(INFO) << _name << ":disable write" << std::endl;
    _events_interested &= ~EPOLLOUT;
    update_poll_cfg();
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

    LOG(DEBUG) << _name << ": recv event:" << event_to_string(_events_received) << std::endl;
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
