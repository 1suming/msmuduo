#include"stdafx.h"
 
 
#include"msmuduo/net/TcpClient.h"
#include"msmuduo/base/Logging.h"
#include"msmuduo/net/EventLoop.h"

#include<stdio.h>

NS_USING;


int globalCnt = 0;
void connectCallback(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		LOG_INFO << "-----------connectionCallback connected";
	}
	else
	{
		LOG_INFO << "-----------connectionCallback disconnected";
	}

}

void messageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	LOG_DEBUG << "---mesageCallback";
	string str = buf->retrieveAllAsString();
	globalCnt += str.size();
	printf("receive:%s\n", str.c_str());
	printf("\n------------------globalcnt:%d\n", globalCnt);
	 
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