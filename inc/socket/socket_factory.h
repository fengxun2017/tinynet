#ifndef _TINYNET_SOCKET_FACTORY_H_
#define _TINYNET_SOCKET_FACTORY_H_

#include "socket_interface.h"
#include <memory>

namespace tinynet
{

class SocketFactory {
public:
    static std::unique_ptr<SocketInterface> create_socket(const std::string &name, SocketInterface::Protocol protocol);
};

}  // namespace tinynet

#endif  // _TINYNET_SOCKET_FACTORY_H_