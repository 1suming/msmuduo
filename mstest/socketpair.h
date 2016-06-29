
#include<iostream>
using namespace std;
#ifdef WIN32 
	#define socket_t SOCKET
#else 
	#define socket_t int 
#endif


#ifdef WIN32 
#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib")


int socketpair(int family, int type, int protocol, socket_t socks[2])
{
	int tcp1, tcp2;
	struct sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int namelen = sizeof(name);

	tcp1 = tcp2 = -1;

	int tcp = socket(AF_INET, SOCK_STREAM,0);

	int opt = 1;
	setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (tcp == -1)
	{
		cout << "socket 1" << endl;
		cout << strerror(errno) << endl;
		cout << GetLastError() << endl;

		goto clean;

	}
	if (bind(tcp, (sockaddr*)&name, namelen) == -1)
	{
		cout << "bind error" << endl;
		goto clean;
	}
	if (listen(tcp, 5) == -1)
	{
		cout << "listen error" << endl;
		goto clean;
	}
	//刚开始port为0，随机端口，然后进行bind，listen，通过getsockname来获取port，
	if (getsockname(tcp, (sockaddr*)&name, &namelen) == -1){ 
		cout << "getsockname" << endl;
		goto clean;
	}

	tcp1 = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp1 == -1){
		cout << "tcp1 socket" << endl;
		goto clean;
	}
	if (-1 == connect(tcp1, (sockaddr*)&name, namelen))
	{
		cout << "connect error" << endl;

		goto clean;
	}

	tcp2 = accept(tcp, (sockaddr*)&name, &namelen);
	if (tcp2 == -1)
	{
		cout << "accept error" << endl;
		goto clean;
	}

	if (closesocket(tcp) == -1)//关闭listen socket
	{
		cout << "close socket " << endl;
		goto clean;
	}

	socks[0] = tcp1;
	socks[1] = tcp2;
	return 0;

clean:

	if (tcp != -1){
		closesocket(tcp);
	}
	if (tcp2 != -1){
		closesocket(tcp2);
	}
	if (tcp1 != -1){
		closesocket(tcp1);
	}

	return -1;




}



#else 


#endif
