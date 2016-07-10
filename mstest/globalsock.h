#ifndef _globalsock_h
#define _globalsock_h


// 平台相关的头文件包含
#if defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define __UNDEF_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef __UNDEF_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef __UNDEF_LEAN_AND_MEAN
#endif

#include<winsock2.h> //常用socket
#include<WS2tcpip.h> // WS2TCPIP.H - WinSock2 Extension for TCP/IP protocols 例如：socklen_t
#else
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>

#ifndef INT8_MAX
#define INT8_MAX	0x7f
#define INT16_MAX	0x7fff
#define INT32_MAX	0x7fffffff
#define INT64_MAX	0x7fffffffffffffff
#define UINT8_MAX	0xff
#define UINT16_MAX	0xffff
#define UINT32_MAX	0xffffffff
#define UINT64_MAX	0xffffffffffffffffU
#endif
#endif

#include<iostream>
using namespace std;


int make_socket_nonblocking(int fd);

int getErrno();
const char* getErrorMsg(int err);// 打印出错误号对应的错误文本


int  createServerSocket(int port);


#endif