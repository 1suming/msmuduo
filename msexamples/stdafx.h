// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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
//��ʾ��timespec��:��struct�������ض��� ����Ӻ꼴��
/*
struct timespec {
	// long long in windows is the same as long in unix for 64bit  
	long long tv_sec;
	long long tv_nsec;

};
*/
#endif
#endif
