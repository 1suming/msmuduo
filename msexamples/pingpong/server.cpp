#include "msmuduo/net/TcpServer.h"
#include "msmuduo/base/Atomic.h"
#include "msmuduo/base/Logging.h"
#include "msmuduo/base/Thread.h"
#include "msmuduo/net/EventLoop.h"
#include "msmuduo/net/InetAddress.h"

#include <utility>

#include <stdio.h>

#ifdef LINUX 
	#include <unistd.h>
#endif 
NS_USING;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->setTcpNoDelay(true);
    }
}
void onMessage(const TcpConnectionPtr& conn,
    Buffer* buf, Timestamp)
{
    conn->send(buf);

}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else
    {
        LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
        Logger::setLogLevel(Logger::WARN);

        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TcpServer server(&loop, listenAddr, "PingPong");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1)
        {
            server.setThreadNum(threadCount);
        }

        server.start();

        loop.loop();
    }
}