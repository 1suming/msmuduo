#include "echo.h"

#include "msmuduo/base/Logging.h"
#include "msmuduo/net/EventLoop.h"

NS_USING;

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress listenAddr(2007);
    int maxConnections = 2;
    if (argc > 1)
    {
        maxConnections = atoi(argv[1]);
    }
    LOG_INFO << "maxConnections = " << maxConnections;
    EchoServer server(&loop, listenAddr, maxConnections);
    server.start();
    loop.loop();
}