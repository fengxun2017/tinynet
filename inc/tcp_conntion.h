#ifndef _TINYNET_TCP_CONNTION_H_
#define _TINYNET_TCP_CONNTION_H_

#include <string>

namespace tinynet
{

class TcpConnection {
public:
    TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port);

    ~TcpConnection();

    void write_data(const void* buffer, size_t length);


    std::string get_client_ip(void);

    int get_client_port(void);

private:
    void handle_recvdata(uint8_t data, size_t len);

    int _sockfd;
    std::string _client_ip;
    int _client_port;
    std::string _server_ip;
    int _server_port;

};

}


#endif // _TINYNET_TCP_CONNTION_H_