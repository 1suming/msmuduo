#ifndef WIN
#include<windows.h>
#include<tlhelp32.h>//CreateToolhelp32Snapshot

#include<iostream>
using namespace std;


DWORD WINAPI ThreadFun(LPVOID pM)
{
	printf("���̵߳��߳�ID��Ϊ��%d\n���߳����Hello World\n", GetCurrentThreadId());
	Sleep(10 * 1000);
	return 0;
}


DWORD getMainThreadId()
{
	DWORD m_threadId = 0;
	FILETIME timeRunE = { 0 };
	const DWORD idProcess = GetCurrentProcessId();
	HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, idProcess);//��ȡ���վ��
	if (hThreadSnap == INVALID_HANDLE_VALUE)//�����ȡʧ�ܷ���
		return 0;
	THREADENTRY32 pe32 = { sizeof(pe32) };//���սṹ�������С
	if (::Thread32First(hThreadSnap, &pe32))
	{
		do
		{
			if (pe32.th32OwnerProcessID == idProcess)//�������ID��������Ѱ�ҵĽ���ID�򷵻����߳�ID
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
	}//����ѭ���ж�β
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