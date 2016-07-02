#include"../stdafx.h"
#include"sockettool.h"

#include"base/Logging.h"

NS_BEGIN
 
#ifdef WIN 
/*
关于这个实现可以看libevent:https://github.com/nmathewson/Libevent/blob/master/evutil.c evutil_socketpair
*/
int socketpair(int family, int type, int protocol, socket_t socks[2] )
{
	int tcp1, tcp2;
	struct sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int namelen = sizeof(name);

	tcp1 = tcp2 = -1;

	int tcp = socket(AF_INET, SOCK_STREAM, 0); //listener

	int opt = 1;
	setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (tcp == -1)
	{
		//cout << "socket 1" << endl;
		//cout << strerror(errno) << endl;
		//cout << GetLastError() << endl;

		goto clean;

	}
	if (::bind(tcp, (struct sockaddr*)&name, namelen) == -1)
	{
		//cout << "bind error" << endl;
		goto clean;
	}
	if (listen(tcp, 5) == -1)
	{
		//cout << "listen error" << endl;
		goto clean;
	}
	//刚开始port为0，随机端口，然后进行bind，listen，通过getsockname来获取port，
	if (getsockname(tcp, (sockaddr*)&name, &namelen) == -1){
		//cout << "getsockname" << endl;
		goto clean;
	}

	tcp1 = socket(AF_INET, SOCK_STREAM, 0);//connector
	if (tcp1 == -1){
		//cout << "tcp1 socket" << endl;
		goto clean;
	}
	if (-1 == connect(tcp1, (sockaddr*)&name, namelen))
	{
		//cout << "connect error" << endl;

		goto clean;
	}

	tcp2 = accept(tcp, (sockaddr*)&name, &namelen);
	if (tcp2 == -1)
	{
		//cout << "accept error" << endl;
		goto clean;
	}
	/* Now check we are talking to ourself by matching port and host on the
	two sockets.	 */
	if (getsockname(tcp1, (struct sockaddr*)&name, &namelen) == -1)
	{
		goto clean;
	}
	if (closesocket(tcp) == -1)//关闭listen socket
	{
		//cout << "close socket " << endl;
		goto clean;
	}
	/*
	if (nonblocking)//libevent没有设置nonblocking
	{
		set_socket_nonblocking(tcp1);
		set_socket_nonblocking(tcp2);
	}
	*/
	socks[0] = tcp1;//connector
	socks[1] = tcp2;//acceptor
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


 
#endif //end win


int getErrno()
{
	int err = 0;
#ifdef WIN
	err=WSAGetLastError();
#else
	err = errno;
#endif
	return err;
}

const char* getErrorMsg(int err)
{
#ifdef WIN 
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	return (const char*)lpMsgBuf;
#else 
	return strerror(err);
#endif

}

int make_socket_nonblocking(int fd)
{
#ifdef WIN 

	unsigned long nonblocking=1;
	if(ioctlsocket(fd,FIONBIO,&nonblocking)==SOCKET_ERROR)
	{
		LOG_ERROR << "ioctlsocket error,fd:" << fd;
		return -1;
	}

#else 
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0)
	{
		return -1;
	}
	if (!(flags & O_NONBLOCK))
	{
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			return -1;
		}
	}

#endif

	return 0;
}

int make_listen_socket_reuseable(int fd)
{
#ifdef LINUX
	int one = 1;
	/* REUSEADDR on Unix means, "don't hang on to this address after the
	* listener is closed."  On Windows, though, it means "don't keep other
	* processes from binding to this address while we're using it. */
	return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&one,
		(ev_socklen_t)sizeof(one));
#else
	return 0;
#endif
}


NS_END