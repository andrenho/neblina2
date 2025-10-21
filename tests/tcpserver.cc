#include "doctest.h"

#include "echo/echo.hh"
#include "server/tcpserver.hh"

TEST_SUITE("TCP Server")
{
    TEST_CASE("Simple echo")
    {
        logging_verbose = true;
        TCPServer(23456, false, std::make_unique<EchoProtocol>(), 8).run();
    }
}