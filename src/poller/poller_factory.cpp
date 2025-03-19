#include "poller_factory.h"
#include "linux_poller.h"

namespace tinynet
{

std::unique_ptr<PollerInterface> PollerFactory::create_poller()
{
    // only support linux
    return std::make_unique<LinuxPoller>();
}

}  // namespace tinynet