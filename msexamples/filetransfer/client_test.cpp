#include"stdafx.h"
 
 
#include"msmuduo/net/TcpClient.h"
#include"msmuduo/base/Logging.h"
#include"msmuduo/net/EventLoop.h"

#include<stdio.h>

NS_USING;



void connectCallback(const TcpConnectionPtr& conn)
{
	LOG_INFO << "-----------connectionCallback";
 


}

void messageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	LOG_DEBUG << "---mesageCallback";
	string str = buf->retrieveAllAsString();
	printf("receive:%s\n", str.c_str());
	 
}


int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::TRACE);
	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 9981); // no such server
	TcpClient client(&loop, serverAddr, "TcpClient");
	client.connect();

	client.setConnectionCallback(connectCallback);
	client.setMessageCallback(messageCallback);
	loop.loop();
}