#include"stdafx.h"
#include"InetAddress.h"
#include"base/Logging.h"
#include"base/sockettool.h"
#include"base/endiantool.h"

#include <boost/static_assert.hpp>

NS_USING;

//// INADDR_ANY use (type)value casting.
//#pragma GCC diagnostic ignored "-Wold-style-cast"

#ifdef WIN
	typedef uint32_t in_addr_t;
#endif
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;


//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };


BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));


InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
 	memset(&addr_, 0, sizeof addr_);
	addr_.sin_family = AF_INET;
	in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
	addr_.sin_addr.s_addr = endiantool::hostToNetwork32(ip);
	addr_.sin_port = endiantool::hostToNetwork16(port);
}
InetAddress::InetAddress(StringArg ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	sockettool::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIpPort() const
{
	char buf[32];
 	sockettool::toIpPort(buf, sizeof buf,addr_);
	return buf;
}
string InetAddress::toIp() const
{
	char buf[32];
	sockettool::toIp(buf, sizeof buf, addr_);
	return buf;
}





static threadlocal char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
	assert(out != NULL);
	struct hostent hent;
	struct hostent* he = NULL;
	int herrno = 0;
	bzero(&hent, sizeof(hent));

#ifdef WIN 
	he=gethostbyname(hostname.c_str());
 #else 
	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);

#endif

#ifdef WIN 
	if (he!=NULL)
#else
	if (ret == 0 && he != NULL)
#endif
	{
		assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
		out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
	}
	else
	{
		 
		LOG_ERROR << "InetAddress::resolve";
		
		return false;
	}
}