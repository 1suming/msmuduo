
#ifndef _platform_h_
#define _platform_h_

#include <assert.h>

//当前处于哪个平台
#if !defined(PLATFORM)
	#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
		#define WIN
	#else 
		#define LINUX
	#endif
	#define PLATFORM
#endif

// 平台相关的头文件包含
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
// 屏蔽protobuf警告
#pragma warning( disable : 4996 )
#endif

//线程、锁
#ifdef WIN
	#include<process.h>
	#include<sys/locking.h>
#else
	#include<pthread.h>

	#define nullptr NULL
#endif

// 线程局部存储
#if !defined(_TTHREAD_CPP11_) && !defined(thread_local)
	#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
	#define threadlocal __thread
#else
	#define threadlocal  __declspec(thread)
#endif
#endif

// 函数调用
#ifdef WIN
	#define fstat _fstati64
	#define stat _stati64
	#define mode_t int
	#define sleep Sleep  //Sleep()里面的单位，是以毫秒为单位
	#define bzero(buf, len) memset(buf, 0, len)
#else
	#define sprintf_s snprintf
	#define vsprintf_s vsnprintf
	#define sleep(ms) usleep(ms * 1000)
#endif

//公用头文件
#include<string>
#include<list>
#include<vector>
#include<map>
#include<iterator>
#include<algorithm>


using namespace std;

// 调试宏重定义
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



#endif