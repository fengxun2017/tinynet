#ifndef _TINYNET_POLLER_FACTORY_H_
#define _TINYNET_POLLER_FACTORY_H_

#include <memory>
#include "poller_interface.h"

namespace tinynet
{

class PollerFactory
{
public:
    static std::unique_ptr<PollerInterface> create_poller();
};

}  // namespace tinynet

#endif  // _TINYNET_POLLER_FACTORY_H_