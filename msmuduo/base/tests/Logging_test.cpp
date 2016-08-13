#include"../../stdafx.h"
 
#include"../Logging.h"
#include"../Thread.h"
#include"../ThreadPool.h"
#include<iostream>

NS_USING;


void logInThread()
{
	LOG_INFO << "logInThread text";
	sleep(1);
	 
}


int main()
{
	/*
	ThreadPool pool("pool");
	pool.start(5);

	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);
	pool.run(logInThread);

	sleep(10);
	*/
	LOG_DEBUG << "debug msg";
	LOG_INFO << "info msg";
	LOG_OK << "ok msg";
	LOG_WARN << "warn msg";
	LOG_ERROR << "error msg" << "error is ";

	LOG_INFO << "info msg";




}