// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//


#ifndef _stdafx_h
#define _stdafx_h



#define NS_BEGIN namespace ms {
#define  NS_END   }
#define  NS_USING using namespace ms


#include"type.h"
#include"platform.h"
#include<assert.h>
#ifdef WIN
	#define snprintf _snprintf
//提示“timespec”:“struct”类型重定义 ，添加宏即可
/*
struct timespec {
	// long long in windows is the same as long in unix for 64bit  
	long long tv_sec;
	long long tv_nsec;

};
*/
#endif
#endif
