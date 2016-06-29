
#ifndef _endiantool_h_
#define _endiantool_h_

NS_BEGIN
namespace endiantool
{
	// 将64位的整数从主机字节顺序转换成网络字节顺序
	uint64_t hostToNetwork64(uint64_t host64);

	// 将32位的整数从主机字节顺序转换成网络字节顺序
	uint32 hostToNetwork32(uint32 host32);

	// 将16位的整数从主机字节顺序转换成网络字节顺序
	uint16 hostToNetwork16(uint16 host16);

	// 将64位的整数从网络字节顺序转换成主机字节顺序
	uint64_t networkToHost64(uint64_t net64);

	// 将32位的整数从网络字节顺序转换成主机字节顺序
	uint32 networkToHost32(uint32 net32);

	// 将16位的整数从网络字节顺序转换成主机字节顺序
	uint16 networkToHost16(uint16 net16);


	/*----------------------------*/
	uint64_t hostToNetwork(uint64_t host64);

	// 将32位的整数从主机字节顺序转换成网络字节顺序
	uint32 hostToNetwork(uint32 host32);

	// 将16位的整数从主机字节顺序转换成网络字节顺序
	uint16 hostToNetwork(uint16 host16);

	// 将64位的整数从网络字节顺序转换成主机字节顺序
	uint64_t networkToHost(uint64_t net64);

	// 将32位的整数从网络字节顺序转换成主机字节顺序
	uint32 networkToHost(uint32 net32);

	// 将16位的整数从网络字节顺序转换成主机字节顺序
	uint16 networkToHost(uint16 net16);

	#define is_bigendian() ((*(char*) &endian) == 0)
	#define is_littlendbian() ((*(char*) &endian) == 1)

}

NS_END
#endif //_endiantool_h_