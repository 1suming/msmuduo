#include"../stdafx.h"

#include"Thread.h"
 
#ifndef WIN
#include<sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/unistd.h>
#endif

namespace ms
{
namespace CurrentThread
{
	threadlocal  int  t_cachedTid = 0;


}
namespace detail
{
	int gettid()
	{
#ifdef WIN
		return GetCurrentThreadId();
#else
		return static_cast<int>(::syscall(SYS_gettid));
#endif

	}
}

boost::atomic_int Thread::numCreated_;


void CurrentThread::cacheTid()
{
	if (t_cachedTid == 0)
	{
		t_cachedTid = detail::gettid();

	}
}
/* 
bool CurrentThread::isMainThread()
{
#ifdef WIN
	return tid() ==  windows下GetCurrentProcessId()和主线程的id不是同一个
#else
	return tid() == ::getpid();
#endif
}
*/


}