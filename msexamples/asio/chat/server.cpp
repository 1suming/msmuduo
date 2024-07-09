#include"codec.h"


#include"msmuduo/base/Logging.h"
#include"msmuduo/net/Buffer.h"
#include"msmuduo/net/TcpConnection.h"
#include"msmuduo/net/EventLoop.h"

#include<boost/bind.hpp>
#include<boost/function.hpp>

#include"msmuduo/net/TcpServer.h"
#include<string>
#include<set>
////#include<boost/shared_ptr.hpp>
NS_USING;

class ChatServer :noncopyable {
public:
	ChatServer(EventLoop* loop, const InetAddress& listenAddr):
		server_(loop,listenAddr,"ChatServer"),
		codec_(boost::bind(&ChatServer::onStringMessage,this,_1,_2,_3))
	{
		server_.setConnectionCallback(
			boost::bind(&ChatServer::onConnection, this, _1));
		server_.setMessageCallback(
			boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->peerAddress().toIpPort() << " -> "
			<< conn->localAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			connections_.insert(conn);
		}
		else
		{
			connections_.erase(conn);
		}

	}
	void onStringMessage(const TcpConnectionPtr&,
		const std::string& message,
		Timestamp)
	{
		for (ConnectionList::iterator it = connections_.begin(); it != connections_.end(); it++)
		{
			codec_.send(get_pointer(*it), message);
		}
	}
private:
	typedef std::set<TcpConnectionPtr> ConnectionList;
	TcpServer server_;
	LengthHeaderCodec codec_;
	ConnectionList connections_;
};

int main(int argc, char* argv[])
{
	
	uint16_t port = 8888;

	if (argc >1 ) {
		//printf("Usage: %s port\n", argv[0]);
		//return -1;
		port  = static_cast<uint16_t>(atoi(argv[1]));
	}
	LOG_INFO << "pid=" << getpid() << "port :" << port;
	EventLoop loop;
	InetAddress serverAddr(port);
	ChatServer server(&loop, serverAddr);
	server.start();
	loop.loop();



}