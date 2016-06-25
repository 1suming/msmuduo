#ifndef WIN
#include<windows.h>
#include<tlhelp32.h>//CreateToolhelp32Snapshot

#include<iostream>
using namespace std;


DWORD WINAPI ThreadFun(LPVOID pM)
{
	printf("子线程的线程ID号为：%d\n子线程输出Hello World\n", GetCurrentThreadId());
	Sleep(10 * 1000);
	return 0;
}


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
int main()
{
	const DWORD pid = GetCurrentProcessId();

	DWORD tid = GetCurrentThreadId();
	cout << "pid:" << pid << endl;
	cout << "tid:" << tid << endl;


	HANDLE handle = CreateThread(NULL, 0, ThreadFun, NULL, 0, NULL);

	cout << "main thread id:" << getMainThreadId() << endl;








}

#endif