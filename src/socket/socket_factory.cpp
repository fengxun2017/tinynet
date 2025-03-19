#include "socket_factory.h"
#include "linux_socket.h"
#include <memory>

namespace tinynet
{
std::unique_ptr<SocketInterface> SocketFactory::create_socket(const std::string &name, SocketInterface::Protocol protocol)
{
    // only support linux
    return std::make_unique<LinuxSocket>(name, protocol);
}

}  // namespace tinynet