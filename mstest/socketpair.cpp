#include"socketpair.h"
#include<iostream>
using namespace std;

#define BUFF_SIZE 50
int main()
{
#ifdef WIN32 
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		cout<<"WSAStartup failed";
		exit(0);
	}


#else 
#endif
	socket_t socks[2];
	int ret=socketpair(AF_INET, SOCK_STREAM, 0, socks);
	if (ret == -1)
	{
		cout << "socket pair error" << endl;
		return -1;
	}

	char *string = "This is a test string";
	char buf[BUFF_SIZE] = "This is a test string";
	char buf2[BUFF_SIZE];

	int w, r;
	//test in a single process
	if ((w = send(socks[0], buf, strlen(buf), 0)) == -1)
	{
		cout << "write socket error:" << strerror(errno);
		return -1;
	}
	//read 
	if ((r = recv(socks[1], buf2, BUFF_SIZE, 0)) == -1)
	{
		cout << "recv error" << strerror(errno);
		return -1;
	}
	buf2[r] = '\0';
	cout <<"buf2 is :"<< buf2 << endl;

	 
	char buf3[BUFF_SIZE] = "another string";
	if ((w = send(socks[1], buf3, strlen(buf3), 0)) == -1)
	{
		cout << "write socket error:" << strerror(errno);
		return -1;
	}
	//read 
	if ((r = recv(socks[0], buf2, BUFF_SIZE, 0)) == -1)
	{
		cout << "recv error" << strerror(errno);
		return -1;
	}
	buf2[r] = '\0';
	cout << "buf2 is :" << buf2 << endl;



	return 0;

}