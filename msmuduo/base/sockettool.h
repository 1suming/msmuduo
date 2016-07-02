#ifndef _sockettool_h
#define _sockettool_h

NS_BEGIN

#ifdef WIN
//为了兼容linux的用法，不在sockettool命名空间定义，而放在外面
int socketpair(int family, int type, int protocol, socket_t socks[2] ); 
 
#endif


int getErrno();
const char* getErrorMsg(int err);// 打印出错误号对应的错误文本

//设置fd为non blocking
int make_socket_nonblocking(int fd);

int make_listen_socket_reuseable(int fd);


namespace sockettool
{

#ifdef WIN
	

#endif


	

}


NS_END
#endif