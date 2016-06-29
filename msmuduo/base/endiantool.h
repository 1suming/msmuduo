
#ifndef _endiantool_h_
#define _endiantool_h_

NS_BEGIN
namespace endiantool
{
	// ��64λ�������������ֽ�˳��ת���������ֽ�˳��
	uint64_t hostToNetwork64(uint64_t host64);

	// ��32λ�������������ֽ�˳��ת���������ֽ�˳��
	uint32 hostToNetwork32(uint32 host32);

	// ��16λ�������������ֽ�˳��ת���������ֽ�˳��
	uint16 hostToNetwork16(uint16 host16);

	// ��64λ�������������ֽ�˳��ת���������ֽ�˳��
	uint64_t networkToHost64(uint64_t net64);

	// ��32λ�������������ֽ�˳��ת���������ֽ�˳��
	uint32 networkToHost32(uint32 net32);

	// ��16λ�������������ֽ�˳��ת���������ֽ�˳��
	uint16 networkToHost16(uint16 net16);


	/*----------------------------*/
	uint64_t hostToNetwork(uint64_t host64);

	// ��32λ�������������ֽ�˳��ת���������ֽ�˳��
	uint32 hostToNetwork(uint32 host32);

	// ��16λ�������������ֽ�˳��ת���������ֽ�˳��
	uint16 hostToNetwork(uint16 host16);

	// ��64λ�������������ֽ�˳��ת���������ֽ�˳��
	uint64_t networkToHost(uint64_t net64);

	// ��32λ�������������ֽ�˳��ת���������ֽ�˳��
	uint32 networkToHost(uint32 net32);

	// ��16λ�������������ֽ�˳��ת���������ֽ�˳��
	uint16 networkToHost(uint16 net16);

	#define is_bigendian() ((*(char*) &endian) == 0)
	#define is_littlendbian() ((*(char*) &endian) == 1)

}

NS_END
#endif //_endiantool_h_