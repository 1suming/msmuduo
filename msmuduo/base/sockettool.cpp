#include"stdafx.h"
#include"sockettool.h"
#include"endiantool.h"
#include"base/Logging.h"

NS_BEGIN
 
#ifdef WIN 
/*
关于这个实现可以看libevent:https://github.com/nmathewson/Libevent/blob/master/evutil.c evutil_socketpair
*/
int socketpair(int family, int type, int protocol, socket_t socks[2] )
{
#ifdef WIN 
	executeWSAStartup();
#endif
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
		LOG_ERROR << "socketpair tcp==-1" << getErrno() << getErrorMsg(getErrno());

		goto clean;

	}
	if (::bind(tcp, (struct sockaddr*)&name, namelen) == -1)
	{
		LOG_ERROR << "socketpair bind tcp error" << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}
	if (listen(tcp, 5) == -1)
	{
		LOG_ERROR << "socketpair listen tcp error" << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}
	//刚开始port为0，随机端口，然后进行bind，listen，通过getsockname来获取port，
	if (getsockname(tcp, (sockaddr*)&name, &namelen) == -1){
		LOG_ERROR << "socketpair getsockname " << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}

	tcp1 = socket(AF_INET, SOCK_STREAM, 0);//connector
	if (tcp1 == -1){
		LOG_ERROR << "socketpair tcp1==-1" << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}
	if (-1 == connect(tcp1, (sockaddr*)&name, namelen))
	{
		LOG_ERROR << "socketpair connect " << getErrno() << getErrorMsg(getErrno());

		goto clean;
	}

	tcp2 = accept(tcp, (sockaddr*)&name, &namelen);
	if (tcp2 == -1)
	{
		LOG_ERROR << "socketpair accept " << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}
	/* Now check we are talking to ourself by matching port and host on the
	two sockets.	 */
	if (getsockname(tcp1, (struct sockaddr*)&name, &namelen) == -1)
	{
		LOG_ERROR << "socketpair getsockname tcp1" << getErrno() << getErrorMsg(getErrno());
		goto clean;
	}
	sockettool::close(tcp);//关闭listen socket
	 
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
		sockettool::close(tcp);
	}
	if (tcp2 != -1){
		sockettool::close(tcp2);
	}
	if (tcp1 != -1){
		sockettool::close(tcp1);
	}

	return -1;


}

bool executeWSAStartup()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		 
		LOG_FATAL << "WSAStartup error";
		return false;

	}
	return true;

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


//新建一个命名空间
namespace sockettool
{
	typedef struct sockaddr SA;

	const SA* sockaddr_cast(const struct sockaddr_in* addr)
	{
		return static_cast<const SA*>(implicit_cast<const void*>(addr));
	}

	SA* sockaddr_cast(struct sockaddr_in* addr)
	{
		return static_cast<SA*>(implicit_cast<void*>(addr));
	}



	void close(int sockfd)
	{
#ifdef WIN
		if (::closesocket(sockfd) < 0)
		{
			LOG_ERROR << "sockettool::close";
		}
#else 
		if (::close(sockfd) < 0)
		{
			LOG_ERROR << "sockettool::close";
		}
#endif

	}

	int createNonblockingOrDie()
	{
		int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			LOG_FATAL << "socket::createNonblockingOrDie";
		}

		make_socket_nonblocking(sockfd);

		return sockfd;

	}
	int  connect(int sockfd, const struct sockaddr_in& addr)
	{
		return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
	}
	void listenOrDie(int sockfd)
	{
		int ret = ::listen(sockfd, SOMAXCONN);
		if (ret < 0)
		{
			LOG_FATAL << " sockettool::listenorDie";
		}
	}
	void bindOrDie(int sockfd, const struct sockaddr_in& addr)
	{
		int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
		if (ret < 0)
		{
			LOG_FATAL << "sockets:bindOrDie";
		}
	}

	void shutdownWrite(int sockfd)
	{
 
		////windows下SD_SEND,SD_RECEIVE和SD_BOTH ，SHUT_WR已经重定义
		if (::shutdown(sockfd, SHUT_WR) < 0)
		{
			LOG_ERROR << "shutdownWrite";
		}
	}


	int accept(int sockfd, struct sockaddr_in* addr)
	{
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

		int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
		// setNonBlockAndCloseOnExec(connfd);
		//!!!Important:使socket变成非阻塞
		make_socket_nonblocking(connfd);
		// int connfd = ::accept4(sockfd, sockaddr_cast(addr),
		//                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
		if (connfd < 0)
		{
			int savedErrno = getErrno();
			LOG_ERROR << "Socket:accept";
			switch (savedErrno)
			{
			case EAGAIN:
			case ECONNABORTED:
			case EINTR:
			case EPROTO: // ???
			case EPERM:
			case EMFILE: // per-process lmit of open file desctiptor ???
				// expected errors
				errno = savedErrno;
				break;
			case EBADF:
			case EFAULT:
			case EINVAL:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case ENOTSOCK:
			case EOPNOTSUPP:
				// unexpected errors
				LOG_FATAL << "unexpected error of ::accept " << savedErrno;
				break;
			default:
				LOG_FATAL << "unknown error of ::accept " << savedErrno;
				break;
			}
		}
		return connfd;
		 
	}


	ssize_t read(int sockfd, void *buf, size_t count)
	{
#ifdef LINUX 
		return ::read(sockfd, buf, count);
#else 
		return ::recv(sockfd, (char*)buf, count, 0);
#endif
	}

	//ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
	//{
	//  return ::readv(sockfd, iov, iovcnt);
	//}

	ssize_t  write(int sockfd, const void *buf, size_t count)
	{
#ifdef LINUX 
		return ::write(sockfd, buf, count);
#else 
		return ::send(sockfd, (char*)buf, count, 0);
#endif
	}

	/*----------------------------------*/

	void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)//把ip和port写入addr
	{
		addr->sin_family = AF_INET;
		addr->sin_port = endiantool::hostToNetwork16(port);
		if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
		{
			LOG_ERROR << " sockets::fromIpPort";
		}

	}
	void toIpPort(char* buf, size_t size,const struct sockaddr_in& addr)
	{
		assert(size >= INET_ADDRSTRLEN);
		::inet_ntop(AF_INET, (void*)&addr.sin_addr, buf, static_cast<socklen_t>(size));
		size_t end = ::strlen(buf);
		uint16_t port = endiantool::networkToHost16(addr.sin_port);
		assert(size > end);
		snprintf(buf + end, size - end, ":%u", port);//把port输入到buf
	}
	void toIp(char* buf, size_t size,	const struct sockaddr_in& addr)
	{
		assert(size >= INET_ADDRSTRLEN);
		::inet_ntop(AF_INET, (void*)&addr.sin_addr, buf, static_cast<socklen_t>(size));


	}
	int  getSocketError(int sockfd)
	{
		int optval;
		socklen_t optlen = static_cast<socklen_t>(sizeof optval);
		//windows下要强制转成char*
		if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) < 0)
		{
			return errno;
		}
		else
		{
			return optval;
		}
	}

	struct sockaddr_in getLocalAddr(int sockfd)
	{
		struct sockaddr_in localaddr;
		bzero(&localaddr, sizeof(localaddr));
		socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);

		if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
		{
			LOG_ERROR << "getLocalAddr";
		}
		return localaddr;

	}
	struct sockaddr_in getPeerAddr(int sockfd)
	{
		struct sockaddr_in peeraddr;
		bzero(&peeraddr, sizeof peeraddr);
		socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
		if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
		{
			LOG_ERROR << "sockets::getPeerAddr";
		}
		return peeraddr;

	}
	bool isSelfConnect(int sockfd)
	{
		struct sockaddr_in localaddr = getLocalAddr(sockfd);
		struct sockaddr_in peeraddr = getPeerAddr(sockfd);
		return localaddr.sin_port == peeraddr.sin_port &&
			localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;

	}
}
NS_END