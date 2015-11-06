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

#ifdef WIN
	#define snprintf _snprintf
#endif
#endif
