#ifndef _ThreadPool_h
#define _ThreadPool_h

#include"Thread.h"
#include"lock.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include<boost/ptr_container/ptr_vector.hpp>

/*
threadpool跟我们经常见到的taskqueue是同义的，只不过这里用了boost::bind表示task.
*/

#include<deque>

namespace ms
{
	class ThreadPool : boost::noncopyable
	{
		typedef boost::function<void()> Task;

	public:
		explicit ThreadPool(const string& nameArg=string("ThreadPool") ):
			mutex_(),
			notEmpty_(mutex_),
			notFull_(mutex_),
			name_(nameArg),
			maxQueueSize_(0),
			running_(false)
		{
		}
		~ThreadPool()
		{
			if (running_)
				stop();

		}
		size_t queueSize() const
		{
			lock_guard_t lock(mutex_);
			return queue_.size();
		}
		// Could lbock if maxQueue>0
		void run(const Task& f);


		// Must be called before start().
		void setMaxQueueSize(int maxSize){ maxQueueSize_ = maxSize; }
		void setThreadInitCallback(const Task& cb){ threadInitCallback_ = cb; }

		void start(int numThreads);
		void stop();


private:

	bool isFull() const { return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;}//  mutex_.assertLocked(); 
	void runInThread();
	Task take();

	
private:
	mutable mutex_t mutex_;
	condition_var_t notEmpty_;
	condition_var_t notFull_;


	Task threadInitCallback_;

	boost::ptr_vector<ms::Thread> threads_;

	std::deque<Task> queue_;
	size_t maxQueueSize_;
	bool running_;

	string name_;

};

}

#endif
