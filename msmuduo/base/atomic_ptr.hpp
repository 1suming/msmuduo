/*

Reference zeromq https://github.com/zeromq/zeromq3-x/blob/master/src/atomic_ptr.hpp




*/
#ifndef _atomic_ptr_hpp
#define  _atomic_ptr_hpp

#include"msmuduo/stdafx.h"

#define USE_LOCK
#ifdef USE_LOCK 
#include"msmuduo/base/lock.h"
	#define ZMQ_ATOMIC_PTR_MUTEX
#elif defined WIN
	#define  ZMQ_ATOMIC_PTR_WINDOWS

#elif defined LINUX
	#define ZMQ_ATOMIC_PTR_X86
#endif



NS_BEGIN

//This class encapsulates serveral atomic operations on pointers;

template<typename T> 
class atomic_ptr_t
{
public:

	//Initialize atomic pointer
	inline atomic_ptr_t()
	{
		ptr = NULL;
	}
	//destroy atomic pointer
	inline ~atomic_ptr_t()
	{

	}

	//  Set value of atomic pointer in a non-threadsafe way
	//  Use this function only when you are sure that at most one
	//  thread is accessing the pointer at the moment.
	inline void set(T *ptr_)
	{
		this->ptr = ptr_;
	}


	//Perform atomic 'exchange pointers' operation.Pointer is set
	//  to the 'val' value. Old value is returned.
	inline T* xchg(T* val_)
	{
#if defined ZMQ_ATOMIC_PTR_WINDOWS
		return (T*) InterlockedExchangePointer ((PVOID*) &ptr, val_);
#elif defined ZMQ_ATOMIC_PTR_MUTEX
		sync_.lock();
		T* old = (T*)ptr;
		ptr = val_;
		sync_.unlock();
		return old;

#endif
	}


	//  Perform atomic 'compare and swap' operation on the pointer.
	//  The pointer is compared to 'cmp' argument and if they are
	//  equal, its value is set to 'val'. Old value of the pointer
	//  is returned.
	inline T* cas(T* cmp_, T* val)
	{
#ifdef ZMQ_ATOMIC_PTR_WINDOWS 
		return (T*) InterlockedCompareExchangePointer (
			(volatile PVOID*) &ptr, val_, cmp_);
#elif defined ZMQ_ATOMIC_PTR_MUTEX
		sync_.lock();
		T* old = (T*)ptr;
		if (ptr == cmp_)
			ptr = val_;
		sync_.unlock();
		return old;
#else 
	#error atomic_ptr is not implemented for this platform
#endif

	}

private:
	volatile T* ptr;

#ifdef ZMQ_ATOMIC_PTR_MUTEX
	mutex_t sync_;

#endif

	atomic_ptr_t(const atomic_ptr_t&);
	const atomic_ptr_t &operator = (const atomic_ptr_t&);

};


NS_END
#endif