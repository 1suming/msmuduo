#include"stdafx.h"
#include"tunnel.h"

#include<malloc.h>

#include<map>

#ifdef LINUX
#include <sys/resource.h>
#endif


NS_USING;


EventLoop* g_eventloop;
InetAddress* g_serverAddr;
map<string, TunnelPtr> g_tunnels;

#ifdef LINUX
void memstat()
{
	malloc_stats();

}
#endif


void onServerConnection(const TcpConnectionPtr& conn)
{
	LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
	if (conn->connected())
	{
		conn->setTcpNoDelay(true);
		TunnelPtr tunnel(new Tunnel(g_eventloop, *g_serverAddr, conn));

		tunnel->setup();
		tunnel->connect();

		g_tunnels[conn->name()] = tunnel;


	}
	else
	{
		assert(g_tunnels.find(conn->name()) != g_tunnels.end());

		g_tunnels[conn->name()]->disconnect();
		g_tunnels.erase(conn->name());

	}
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
	LOG_DEBUG << buf->readableBytes();

	if (!conn->getContext().empty())//判断boost::any是否为空
	{
		//conn存储了clientConn，中间人-》server的conn
		const TcpConnectionPtr& clientConn
			= boost::any_cast<const TcpConnectionPtr&>(conn->getContext());

		clientConn->send(buf);

	}

}

int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::TRACE);
	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s <host_ip> <port> <listen_port>\n", argv[0]);
		return -1;
	}
 
		LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
#ifdef LINUX
		{
			// set max virtual memory to 256MB.
			size_t kOneMB = 1024 * 1024;
			rlimit rl = { 256 * kOneMB, 256 * kOneMB };
			setrlimit(RLIMIT_AS, &rl);
		}
#endif

		const char* ip = argv[1];
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress serverAddr(ip, port);
		g_serverAddr = &serverAddr;

		uint16_t acceptPort = static_cast<uint16_t>(atoi(argv[3]));
		InetAddress listenAddr(acceptPort);


		EventLoop loop;
		g_eventloop = &loop;
#ifdef LINUX
		loop.runEvery(3, memstat);
#endif
		

		TcpServer server(&loop, listenAddr, "tcpReplay");

		server.setConnectionCallback(onServerConnection);
		server.setMessageCallback(onServerMessage);

		server.start();
		loop.loop();
}