#ifndef _sockettool_h
#define _sockettool_h

NS_BEGIN

//Ϊ�˼���linux���÷�������sockettool�����ռ䶨�壬����������
int socketpair(int family, int type, int protocol, socket_t socks[2]); 
namespace sockettool
{

#ifdef WIN
	

#endif

}


NS_END
#endif