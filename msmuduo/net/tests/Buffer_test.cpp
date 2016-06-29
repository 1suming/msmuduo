#include"../../stdafx.h"
#include"../Buffer.h"
 
#include"../../base/sockettool.h"
#include<iostream>

NS_USING;
#define BUFF_SIZE 50

int main()
{
#ifdef WIN32 
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		cout << "WSAStartup failed";
		exit(0);
	}
 
#endif


	int fds[2];
	int ret = socketpair(AF_INET, SOCK_STREAM, 0, fds);
	if (ret == -1)
	{
		cout << "socket pair error:" << strerror(errno) <<","<<GetLastError()<< endl;
		exit(-1);
	}
	char buf[BUFF_SIZE] = "This is a test string";
	char springBuf[50] = { 0 };
 
	int w, r;
	//test in a single process
	for (int i = 1; i <= 60; i++){
		string str = "This is a test string";

		
		sprintf(springBuf, "%d", i);
		string lastStr = str + springBuf;

		if ((w = send(fds[0],lastStr.c_str(), lastStr.length(),0 ) ) == -1)
		{
			cout << "write socket error:" << strerror(errno);
			exit(-1);
		}
	}
	Buffer buffer;
	int savedErrno;
	buffer.readFd(fds[1], &savedErrno);

	cout << "buffer:" << buffer.retrieveAllAsString() << endl;
	cout << endl << endl << endl;
	buffer.readFd(fds[1], &savedErrno);
	cout << "buffer:" << buffer.retrieveAllAsString() << endl;





}