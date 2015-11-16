#ifndef _CountDownLatch_h
#define _CountDownLatch_h

#include"lock.h"

#include <boost/noncopyable.hpp>

namespace ms
{
class CountDownLatch: boost::noncopyable 
{
public:
	explicit CountDownLatch(int count):
		mutex_(),
		condition_(mutex_),
		count_(count)
		
	{
		
	}
	void wait()
	{
		lock_guard_t guard(mutex_);
		while(count_>0)
		{
			condition_.wait();
		}
	}
	void countDown()
	{
		lock_guard_t guard(mutex_);
		--count_;
		if(count_==0)
		{
			condition_.notifyAll();
		}
	}
	int getCount() const 
	{
		lock_guard_t guard(mutex_);
		return count_;
	}
	
private:
	mutable mutex_t mutex_;
	condition_var_t condition_;
	int count_;
	
	
};

}
#endif