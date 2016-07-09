#ifndef _Socket_h
#define _Socket_h

#include<boost/noncopyable.hpp>

NS_BEGIN
class InetAddress;

/*
Socket�ķ�װ

Wrapper of socket file descriptor
It close the sockfd when desctructor

//it's thread safe ,all operation are delegated to OS
��װ����Ҫ������������ʱ�Զ�close sockfd
*/

class Socket : boost::noncopyable
{
public:
	explicit Socket(int sockfd) :
		sockfd_(sockfd)
	{

	}
	~Socket();
	int fd() const { return sockfd_; }
	// return true if success.
	bool getTcpInfo(struct tcp_info*) const;
	bool getTcpInfoString(char* buf, int len) const;

	/// abort if address in use
	void bindAddress(const InetAddress& localaddr);
	/// abort if address in use
	void listen();

	/// On success, returns a non-negative integer that is
	/// a descriptor for the accepted socket, which has been
	/// set to non-blocking and close-on-exec. *peeraddr is assigned.
	/// On error, -1 is returned, and *peeraddr is untouched.
	//���ص�connfd�Ѿ�����Ϊ����������
	int accept(InetAddress* peeraddr);

	void shutdownWrite();

	///
	/// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
	///
	void setTcpNoDelay(bool on);

	///
	/// Enable/disable SO_REUSEADDR
	///
	void setReuseAddr(bool on);

	///
	/// Enable/disable SO_REUSEPORT
	///
	void setReusePort(bool on);

	///
	/// Enable/disable SO_KEEPALIVE
	///
	void setKeepAlive(bool on);



private:

	const int sockfd_; //! Ϊconst

};

NS_END
#endif