#ifndef _ypipe_hpp
#define _ypipe_hpp
#include"msmuduo/stdafx.h"

#include<stdlib.h>
#include<stddef.h>

/*
zeromq �����ypipe .ypipe�Ƕ�yqueue���ٰ�װ

//  Lock-free queue implementation.
//  Only a single thread can read from the pipe at any specific moment.
//  Only a single thread can write to the pipe at any specific moment.
//  T is the type of the object in the queue.
//  N is granularity of the pipe, i.e. how many items are needed to
//  perform next memory allocation.

*/

#endif