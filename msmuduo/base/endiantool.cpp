#include"../stdafx.h"
#include"endiantool.h"

NS_BEGIN //最开始找不到hostToNetwork的定义就是因为少了NS_BEGIN
#ifdef WIN


// 如果是win平台，则实现linux下的hton64、ntoh64等函数
namespace endiantool
{
	const int endian = 1;

	// host long 64 to network
	uint64_t hton64(uint64_t host)
	{
		if (is_bigendian()) {
			return host;
		}

		uint64_t ret = 0;
		uint32 high, low;

		low = host & 0xFFFFFFFF;
		high = (host >> 32) & 0xFFFFFFFF;
		low = htonl(low);
		high = htonl(high);

		ret = low;
		ret <<= 32;
		ret |= high;
		return ret;
	}

	//network to host long 64
	uint64_t  ntoh64(uint64_t net)
	{
		if (is_littlendbian()) {
			return net;
		}

		uint64_t ret = 0;
		uint32 high, low;

		low = net & 0xFFFFFFFF;
		high = (net >> 32) & 0xFFFFFFFF;
		low = ntohl(low);
		high = ntohl(high);

		ret = low;
		ret <<= 32;
		ret |= high;

		return ret;
	}
}
#endif

namespace endiantool
{
	// 将64位的整数从主机字节顺序转换成网络字节顺序
	uint64_t hostToNetwork64(uint64_t host64)
	{
#ifdef WIN
		return hton64(host64);
#else
		return htobe64(host64); // means host to big endian64
#endif
	}

	// 将32位的整数从主机字节顺序转换成网络字节顺序
	uint32 hostToNetwork32(uint32 host32)
	{
#ifdef WIN
		return htonl(host32);
#else
		return htobe32(host32);
#endif

	}

	// 将16位的整数从主机字节顺序转换成网络字节顺序
	uint16 hostToNetwork16(uint16 host16)
	{
#ifdef WIN
		return htons(host16); // means host to network short
#else
		return htobe16(host16);
#endif
	}

	// 将64位的整数从网络字节顺序转换成主机字节顺序
	uint64_t networkToHost64(uint64_t net64)
	{
#ifdef WIN
		return ntoh64(net64);
#else
		return be64toh(net64);
#endif
	}

	// 将32位的整数从网络字节顺序转换成主机字节顺序
	uint32 networkToHost32(uint32 net32)
	{
#ifdef WIN
		return ntohl(net32);
#else
		return be32toh(net32);
#endif
	}

	// 将16位的整数从网络字节顺序转换成主机字节顺序
	uint16 networkToHost16(uint16 net16)
	{
#ifdef WIN
		return ntohs(net16);
#else
		return be16toh(net16);
#endif
	}
	//add
	uint64_t hostToNetwork(uint64_t host64) { return hostToNetwork64(host64); }

	// 将32位的整数从主机字节顺序转换成网络字节顺序
	uint32 hostToNetwork(uint32 host32) { return  hostToNetwork32(host32); }

	// 将16位的整数从主机字节顺序转换成网络字节顺序
	uint16 hostToNetwork(uint16 host16) { return hostToNetwork16(host16); }

	// 将64位的整数从网络字节顺序转换成主机字节顺序
	uint64_t networkToHost(uint64_t net64) { return networkToHost64(net64); }

	// 将32位的整数从网络字节顺序转换成主机字节顺序
	uint32 networkToHost(uint32 net32)  { return networkToHost32(net32); }

	// 将16位的整数从网络字节顺序转换成主机字节顺序
	uint16 networkToHost(uint16 net16)  { return networkToHost16(net16); }


}
NS_END
