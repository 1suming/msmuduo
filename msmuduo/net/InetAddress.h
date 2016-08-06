#ifndef _InetAddress_h
#define _InetAddress_h


#include<boost/noncopyable.hpp>
#include"msmuduo/base/StringPiece.h"
#include"msmuduo/base/Copyable.h"

NS_BEGIN
/*
Wrapper of sockaddr_in 

//This is an POD interface class
*/
class InetAddress : public copyable
{
 
public:
	/// Constructs an endpoint with given port number.
	/// Mostly used in TcpServer listening.
	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

	/// Constructs an endpoint with given ip and port.
	/// @c ip should be "1.2.3.4"
	InetAddress(StringArg ip, uint16_t port);

	/// Constructs an endpoint with given struct @c sockaddr_in
	/// Mostly used when accepting new connections
	InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{ }

	string toIp() const;
	string toIpPort() const;

	// default copy/assignment are Okay

	const struct sockaddr_in& getSockAddrInet() const { return addr_; }
	void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

	uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
	uint16_t portNetEndian() const { return addr_.sin_port; }

	// resolve hostname to IP address, not changing port or sin_family
	// return true on success.
	// thread safe
	static bool resolve(StringArg hostname, InetAddress* result);
	// static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

	 
	struct sockaddr_in addr_;

};
NS_END

#endif