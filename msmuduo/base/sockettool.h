#ifndef _sockettool_h
#define _sockettool_h

NS_BEGIN

#ifdef WIN
//Ϊ�˼���linux���÷�������sockettool�����ռ䶨�壬����������
int socketpair(int family, int type, int protocol, socket_t socks[2] ); 
 
bool executeWSAStartup();
#endif


int getErrno();
const char* getErrorMsg(int err);// ��ӡ������Ŷ�Ӧ�Ĵ����ı�

//����fdΪnon blocking
int make_socket_nonblocking(int fd);

int make_listen_socket_reuseable(int fd);


namespace sockettool
{

	const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

	const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
	const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);
	///
	/// Creates a non-blocking socket file descriptor,
	/// abort if any error.
	int createNonblockingOrDie();

	int  connect(int sockfd, const struct sockaddr_in& addr);
	void bindOrDie(int sockfd, const struct sockaddr_in& addr);
	void listenOrDie(int sockfd);
	int  accept(int sockfd, struct sockaddr_in* addr);
	ssize_t read(int sockfd, void *buf, size_t count);
	ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
	ssize_t write(int sockfd, const void *buf, size_t count);
	void close(int sockfd);
	void shutdownWrite(int sockfd);

	void toIpPort(char* buf, size_t size,
		const struct sockaddr_in& addr);
	void toIp(char* buf, size_t size,
		const struct sockaddr_in& addr);
	void fromIpPort(const char* ip, uint16_t port,
	struct sockaddr_in* addr);



	int getSocketError(int sockfd);

	struct sockaddr_in getLocalAddr(int sockfd);
	struct sockaddr_in getPeerAddr(int sockfd);
	bool isSelfConnect(int sockfd);


	

 
	bool isSelfConnect(int sockfd);



	

}


NS_END
#endif