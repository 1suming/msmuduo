
#ifndef _platform_h_
#define _platform_h_

#include <assert.h>

//��ǰ�����ĸ�ƽ̨
#if !defined(PLATFORM)
	#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
		#define WIN
	#else 
		#define LINUX
	#endif
	#define PLATFORM
#endif

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

#ifdef WIN
// ����protobuf����
#pragma warning( disable : 4996 )
#endif

//�̡߳���
#ifdef WIN
	#include<process.h>
	#include<sys/locking.h>
#else
	#include<pthread.h>

	#define nullptr NULL
#endif


//�׽�������
#ifdef WIN 
/*
vs�У�warning C4005: ��FD_SETSIZE��: ���ض���
*/
#ifdef WIN
  #pragma warning(disable:4005)
#endif
	#define FD_SETSIZE 1024
#ifdef WIN
	#pragma warning(default:4005) //��������ΪĬ��,ʹ��enable��Ч
#endif



	#define SHUT_RD SD_RECEIVE
	#define SHUT_WR SD_SEND
	#define SHUT_RDWR SD_BOTH
#else 

#endif

// �ֲ߳̾��洢
#if !defined(_TTHREAD_CPP11_) && !defined(thread_local)
	#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
	#define threadlocal __thread
#else
	#define threadlocal  __declspec(thread)
#endif
#endif

// ��������
#ifdef WIN

/*
win10������ʾ error C2338: sizeof(struct stat) == sizeof(struct _stat64i32)

	#define fstat _fstati64
	#define stat _stati64

*/
#define fstat _fstati64i32 //��ʱ�ĳ�_fstati64i32 _fstati64
#define stat _stati64i32 //��ʱ�ĳ�_stati64  _stati64i32

 
	#define mode_t int
	#define sleep Sleep  //Sleep()����ĵ�λ�����Ժ���Ϊ��λ
	#define bzero(buf, len) memset(buf, 0, len)
#else
	#define sprintf_s snprintf
	#define vsprintf_s vsnprintf
	#define sleep(ms) usleep(ms * 1000)
#endif

//����ͷ�ļ�
#include<string>
#include<list>
#include<vector>
#include<map>
#include<iterator>
#include<algorithm>

#include<stdio.h>
#include <string.h> //strerror

using namespace std;

// ���Ժ��ض���
#ifdef WIN
	#define _FUNC_ __FUNCTION__
#else
	#define _FUNC_ << __FUNCTION__ <<
#endif

#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


template<typename To, typename From>
inline To implicit_cast(From const &f)
{
	return f;
}


#endif