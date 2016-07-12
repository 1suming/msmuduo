#include"stdafx.h"

#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"
#include"net/TcpServer.h"

#include<stdio.h>
/*
windows下面测试会偶尔出现 WSAPOLL没有事件，也就是返回的numEvent为0，过一会儿又有了。

*/

NS_USING;

string message;

void onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		printf("onConnection(): tid=%d new connection [%s] from %s\n",
			CurrentThread::tid(),
			conn->name().c_str(),
			conn->peerAddress().toIpPort().c_str());


		conn->send(message); //一连接就发送
	}
	else
	{
		printf("onConnection(): tid=%d connection [%s] is down\n",
			CurrentThread::tid(),
			conn->name().c_str());
	}
}


void onWriteComplete(const TcpConnectionPtr& conn)
{
	conn->send(message); //这个调用又会导致onWriteComplete触发.
}
void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{


	printf("onMessage(): tid=%d received %d bytes from connection [%s] at %s\n",
		CurrentThread::tid(),
		buf->readableBytes(),
		conn->name().c_str(),
		receiveTime.toFormattedString().c_str());

	string str(buf->peek(), buf->readableBytes());
	printf("str:%s\n", str.c_str());

	buf->retrieveAll();

}
int main()
{

	string line;
	for (int i = 33; i < 127; i++)
	{
		line.push_back(char(i));
	}
	line += line; 
	for (size_t i = 0; i < 127 - 33; ++i)
	{
		message += line.substr(i, 72) + '\n';
	}

	cout << message << endl << endl;

	Logger::setLogLevel(Logger::TRACE);

	InetAddress listenAddr(9981);
	EventLoop loop;


	TcpServer server(&loop, listenAddr, "tcpservertest");

	server.setConnectionCallback(onConnection);

	server.setMessageCallback(onMessage);

	server.setWriteCompleteCallback(onWriteComplete);


	server.start();

	loop.loop();



}