
// ƽ̨��ص�ͷ�ļ�����
#if defined(WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define __UNDEF_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef __UNDEF_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef __UNDEF_LEAN_AND_MEAN
#endif

#include<winsock2.h> //����socket
#include<WS2tcpip.h> // WS2TCPIP.H - WinSock2 Extension for TCP/IP protocols ���磺socklen_t

#endif