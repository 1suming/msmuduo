#include"../stdafx.h"

#include"Thread.h"
#include<boost/static_assert.hpp>
 
#ifdef WIN
	#include <tlhelp32.h>
#endif

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
	threadlocal char t_tidString[32];
	threadlocal int t_tidStringLength = 6;
	threadlocal const char* t_threadName = "unknown";
	 



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

AtomicInt32 Thread::numCreated_;


void CurrentThread::cacheTid()
{
	if (t_cachedTid == 0)
	{
		t_cachedTid = detail::gettid();
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);


	}
}

#ifdef WIN 

DWORD getMainThreadId()
{
	DWORD m_threadId = 0;
	FILETIME timeRunE = { 0 };
	const DWORD idProcess = GetCurrentProcessId();
	HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, idProcess);//获取快照句柄
	if (hThreadSnap == INVALID_HANDLE_VALUE)//如果获取失败返回
		return 0;
	THREADENTRY32 pe32 = { sizeof(pe32) };//快照结构并给予大小
	if (::Thread32First(hThreadSnap, &pe32))
	{
		do
		{
			if (pe32.th32OwnerProcessID == idProcess)//如果进程ID等于你想寻找的进程ID则返回主线程ID
			{
				HANDLE handle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, pe32.th32ThreadID);

				FILETIME createtime, time2, time3, time4;
				::GetThreadTimes(handle, &createtime, &time2, &time3, &time4);
				SYSTEMTIME realtime;
				::FileTimeToSystemTime(&createtime, &realtime);
				if (createtime.dwHighDateTime < timeRunE.dwHighDateTime
					|| (timeRunE.dwHighDateTime == 0 && timeRunE.dwLowDateTime == 0))
				{
					m_threadId = pe32.th32ThreadID;
					timeRunE = createtime;
				}
				else
				if (createtime.dwHighDateTime == timeRunE.dwHighDateTime &&
					createtime.dwLowDateTime < timeRunE.dwLowDateTime)
				{
					m_threadId = pe32.th32ThreadID;
					timeRunE = createtime;
				}
			}
		} while (::Thread32Next(hThreadSnap, &pe32));
	}//否则循环判断尾
	::CloseHandle(hThreadSnap);
	return m_threadId;
}

#endif

bool CurrentThread::isMainThread()
{
#ifdef WIN
	return tid() == getMainThreadId();  //windows下GetCurrentProcessId()和主线程的id不是同一个
#else
	return tid() == ::getpid();
#endif
}


}