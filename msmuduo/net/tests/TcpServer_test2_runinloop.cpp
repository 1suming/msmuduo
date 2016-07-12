#include"stdafx.h"

#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"

#include<stdio.h>


NS_USING;


EventLoop* g_loop;
int g_flag = 0;


void runInThread()
{
	printf("runInThread(): tid = %d\n",CurrentThread::tid());
}
 

void run3()
{
	printf("run3():   flag = %d\n", g_flag);
	//g_loop->runAfter(3, run4);
	g_flag = 3;
}


void run2()
{
	printf("run2(): flag = %d\n",g_flag);
	g_loop->queueInLoop(run3); //����EventLoop�����͵��õ���ͬһ���̣߳�����wakeup,���ǵ�poll timeout����.

}
int main()
{
	Logger::setLogLevel(Logger::TRACE);
 
	EventLoop loop;
	g_loop = &loop;
	
	loop.runInLoop(run2);

	loop.loop();
	 
}