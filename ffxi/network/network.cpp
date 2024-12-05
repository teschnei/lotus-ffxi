#include "network.h"

Network::Network() {}
Connection Network::Connect(std::string ip)
{
    asio::ip::tcp::resolver resolver(context);
    auto endpoints = resolver.resolve(ip, "54001");
    asio::ip::tcp::socket sock(context);
    asio::connect(sock, endpoints);
    sock.write_some(asio::buffer(std::string("test")));
    return {};
}
