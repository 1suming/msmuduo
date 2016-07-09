#include"stdafx.h"

#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"

#include<stdio.h>


NS_USING;

void newConnection(int sockfd, const InetAddress& peerAddr)
{
	printf("newConnection(): accepted a new connection from %s\n",
		peerAddr.toIpPort().c_str());

	char buf[] = "How are you";
	sockettool::write(sockfd, buf, sizeof buf);

	sockettool::close(sockfd);

}

int main()
{
  
	 InetAddress listenAddr(9981);
	 EventLoop loop;

	 Acceptor acceptor(&loop, listenAddr);
	 
	 acceptor.setNewConnectionCallback(newConnection);

	 acceptor.listen();

	 loop.loop();

	 

}