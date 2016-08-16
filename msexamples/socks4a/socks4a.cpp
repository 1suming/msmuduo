#include"msmuduo/net/EventLoop.h"
#include"msmuduo/net/TcpServer.h"

#include<stdio.h>
#include<string>


using namespace ms;
using namespace std;


EventLoop* g_eventLoop;
InetAddress* g_serverAddr;
map<string, TunnelPtr> g_tunnels;



void onServerConnection(const TcpConnectionPtr& conn)
{
	LOG_DEBUG << conn->name() << (conn->connected() ? " UP" : " DOWN");

	if (conn->connected())
	{
		conn->setTcpNoDelay(true);

		TunnelPtr tunnel(new Tunnel(g_eventLoop, g_serverAddr, conn));

		tunnel->setup();
		tunnel->connect();
		g_tunnels[conn->name()] = tunnel;

	}
	else
	{
		std::map<string, TunnelPtr>::iterator it = g_tunnels.find(conn->name());
	}
}
int main(int argc, char* argv[])
{
	uint16_t port = 9981;
	if (argc > 1 )
	{
		port = static_cast<uint16_t>(atoi(argv[1]));
	}

	LOG_INFO << "pid=" << getpid() << ",tid=" << CurrentThread::tid();
	InetAddress listenAddr(port);

	EventLoop loop;
	g_eventLoop = &loop;

	TcpServer server(&loop, listenAddr, "socks4");

	server.setConnectionCallback(onServerConnection);
	server.setMessageCallback(onServerMessage);

	server.start();

	loop.loop();

	 
}





