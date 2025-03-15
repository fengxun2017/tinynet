#include "poller_factory.h"
#include "linux_poller.h"

namespace tinynet
{

std::unique_ptr<PollerInterface> PollerFactory::create_poller()
{
    // 根据需要创建具体的 Poller 实例，这里假设只支持 Linux
    return std::make_unique<LinuxPoller>();
}

}  // namespace tinynet