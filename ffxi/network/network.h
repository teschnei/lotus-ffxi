#pragma once

#include "asio/io_context.hpp"
#include <asio.hpp>
#include <string>

class Connection
{
};

class Network
{
public:
    Network();
    Connection Connect(std::string ip);

private:
    asio::io_context context;
};
