#ifndef _sockettool_h
#define _sockettool_h

NS_BEGIN

#ifdef WIN
//Ϊ�˼���linux���÷�������sockettool�����ռ䶨�壬����������
int socketpair(int family, int type, int protocol, socket_t socks[2] ); 
 
#endif


int getErrno();
const char* getErrorMsg(int err);// ��ӡ������Ŷ�Ӧ�Ĵ����ı�

//����fdΪnon blocking
int make_socket_nonblocking(int fd);

int make_listen_socket_reuseable(int fd);


namespace sockettool
{

#ifdef WIN
	

#endif


	

}


NS_END
#endif