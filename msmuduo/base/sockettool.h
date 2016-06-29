#ifndef _sockettool_h
#define _sockettool_h

NS_BEGIN

//为了兼容linux的用法，不在sockettool命名空间定义，而放在外面
int socketpair(int family, int type, int protocol, socket_t socks[2]); 
namespace sockettool
{

#ifdef WIN
	

#endif

}


NS_END
#endif