#include"msmuduo/stdafx.h"

#include"net/Buffer.h"
#include"base/sockettool.h"

#include<iostream>

NS_USING;
#define BUFF_SIZE 50

int main()
{
#ifdef WIN
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		cout << "WSAStartup failed";
		exit(0);
	}

 
#endif


	int fds[2];
	int ret = socketpair(AF_INET, SOCK_STREAM, 0, fds);

	/*
		make_socket_nonblocking(fds[0]);
		make_socket_nonblocking(fds[1]);
	*/
	if (ret == -1)
	{
		//���û��WSAStartup�᷵�أ�socket pair error:0 No error,10093
		cout << "socket pair error:" << errno<<ends<<strerror(errno) <<","<<GetLastError()<< endl;
		//���û��WSAStartup�᷵�أ�10093 Either the application has not called WSAStartup, or WSAStartup failed.
		cout << getErrno() << ends << getErrorMsg(getErrno()) << endl;
		exit(-1);
	}
	char buf[BUFF_SIZE] = "This is a test string";
	char springBuf[50] = { 0 };
 
	int w, r;
	//test in a single process
	for (int i = 1; i <= 60; i++){
		string str = "This is a test string"; //"b";// 

		
		sprintf(springBuf, "%d", i);
		string lastStr = str + springBuf;

		if ((w = send(fds[0],lastStr.c_str(), lastStr.length(),0 ) ) == -1) //
		{
			cout << "write socket error:" << getErrno() << getErrorMsg(getErrno());
			exit(-1);
		}
	}
	Buffer buffer;
	int savedErrno;
	int readCnt = 0;
	readCnt=buffer.readFd(fds[1], &savedErrno);

	cout << "readCnt:"<<readCnt<<",buffer:" << buffer.retrieveAllAsString() << endl;
	cout << endl << endl << endl;

	/*
	recv()��recvfrom()��WSARecv()��WSARecvfrom()�������������׽���Ϊ�������øú����������ݡ������ʱ�׽��ֻ�������û�����ݿɶ���������߳������ݵ���ǰһֱ˯�ߡ�

   */
	readCnt=buffer.readFd(fds[1], &savedErrno);
	cout << "readCnt:"<<readCnt <<",buffer:" << buffer.retrieveAllAsString() << endl;





}