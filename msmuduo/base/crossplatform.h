#ifndef _crossplatform_h
#define _crossplatform_h


#include<time.h>
#ifdef WIN32
	#include<windows.h>
#else 
	#include<sys/time.h>
#endif

#ifdef WIN32 


//windows÷–∂®“Â£∫
/*
* Structure used in select() call, taken from the BSD file sys/time.h.
*/
/*
struct timeval {
long    tv_sec;
long    tv_usec;
*/
int gettimeofday(struct timeval *tp, void *time_zone_p);
void writePid();

#endif










#endif