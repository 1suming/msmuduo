#include"msmuduo/stdafx.h"

#include"net/EventLoop.h"
#include"base/Thread.h"

#include <boost/bind.hpp>

#include<stdio.h>


NS_USING;

int cnt = 0;
EventLoop* g_loop;

void printTid()
{
	printf("tid=%d\n", CurrentThread::tid());
	printf("now %s\n", Timestamp::now().toString().c_str());
}


void print(const char* msg)
{
	printf("msg %s %s\n", Timestamp::now().toString().c_str(), msg);

	 
}

 
int main()
{
 
	    Logger::setLogLevel(Logger::TRACE);
		EventLoop loop;
		g_loop = &loop;

		print("main");

		loop.runEvery(1, boost::bind(print, "once1"));
		loop.runEvery(2, boost::bind(print, "once2"));
		loop.loop();
  
}
