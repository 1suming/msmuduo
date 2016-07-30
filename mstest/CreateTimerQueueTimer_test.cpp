#include"globalsock.h"

#include<stdio.h>
#include<iostream>
using namespace std;

static int counter = 0;
HANDLE gDoneEvent;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)

{

	if (lpParam == NULL)

	{

		printf("TimerRoutine lpParam is NULL\n");

	}

	else

	{

		// lpParam points to the argument; in this case it is an int



		printf("Timer routine called. Parameter is %d.\n",

			*(int*)lpParam);

		if (TimerOrWaitFired)

		{

			printf("The wait timed out.\n");

		}

		else

		{

			printf("The wait event was signaled.\n");

		}

	}
	SetEvent(gDoneEvent);
	//SetEvent和ResetEvent。前者把事件对象设为”有信号”状态，而后者正好相反。 


}
int main()
{
	HANDLE hTimer = NULL;

	/*
	HANDLE     CreateEvent(

	LPSECURITY_ATTRIBUTES     lpEventAttributes,     //     SD
	BOOL     bManualReset,                                                 //     reset     type
	BOOL     bInitialState,                                                      //     initial     state
	LPCTSTR     lpName                                                       //     object     name
	);   
	*/

	//use an event object to track the TimerRoutine exection
	gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	if (gDoneEvent == NULL)
	{
		printf("createEvent failed");
		return 1;
	}

	HANDLE hTimerQueue = NULL;

	hTimerQueue = CreateTimerQueue();
	if (NULL == hTimerQueue)
	{
		printf("CreateTimerQueue失败。。");
		return 1;
	}
	/*
	BOOL WINAPI CreateTimerQueueTimer(
	_Out_    PHANDLE             phNewTimer,
	_In_opt_ HANDLE              TimerQueue,
	_In_     WAITORTIMERCALLBACK Callback,
	_In_opt_ PVOID               Parameter,
	_In_     DWORD               DueTime,
	_In_     DWORD               Period,
	_In_     ULONG               Flags
	);

	*/
	int arg = 123;
	 
	printf("starttimer\n");

	//// Set a timer to call the timer routin
	if (!CreateTimerQueueTimer(&hTimer, hTimerQueue, WAITORTIMERCALLBACK(TimerRoutine),
		&arg, 5000, 0, NULL))
	{
		printf("CreateTimerQueueTimer失败。。");
		return 1;
	}

	// TODO: Do other useful work here

	
	/*
	Wait For the timer-queue thread to complete using an event object,
	the thread will signal the event at that time
	*/
	 
	if (WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0)
	{
		printf("WaitForSingleObject failed:%d\n", GetLastError());
	}

	CloseHandle(gDoneEvent);

	//Delete all timers in the timer queue
	printf("closehandle\n");
	if (!DeleteTimerQueue(hTimerQueue))
	{
		printf("deletetimerqueu failed");
	}
	 

}