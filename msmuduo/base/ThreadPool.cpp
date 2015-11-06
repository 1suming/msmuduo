#include"../stdafx.h"

#include"ThreadPool.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace ms;

void ThreadPool::start(int numThreads)
{
	assert(threads_.empty());
	running_ = true;
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; i++)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i + 1);
		threads_.push_back(new  Thread(
			boost::bind(&ThreadPool::runInThread, this), name_ + id));
		threads_[i].start();
	}

	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}

}
void ThreadPool::stop()
{
	{
		lock_guard_t lock(mutex_);
		running_ = false;
		notEmpty_.notifyAll();
	}
	/*
	for (boost::ptr_vector<Thread>::iterator it = threads_.begin();
		it != threads_.end(); it++)
	{
		printf("join!\n");
		boost::bind(&Thread::join, it);
	}
	*/
	 
	for_each(threads_.begin(),
		threads_.end(),
		boost::bind(&Thread::join, _1));
}
void ThreadPool::runInThread()
{
	try{
		if (threadInitCallback_)
		{
			threadInitCallback_();
		}
		while (running_)
		{
			Task task(take());
			if (task)
				task();
		}
	}catch (std::exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
		fprintf(stderr, "reaseon:%s\n", ex.what());
		abort();
	}

}
ThreadPool::Task ThreadPool::take()
{
	lock_guard_t lockguard(mutex_);
	//always use a while-loop,due to spurious wake up 
	while (queue_.empty() && running_)
	{
		notEmpty_.wait();
	}
	Task task;
	if (!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();
		
		if (maxQueueSize_ > 0)
		{
			notFull_.notify();//通知notfull_
		}

	}
	return task;

}

void ThreadPool::run(const Task& task)
{
	if (threads_.empty())
	{
		task();
	}
	else
	{
		lock_guard_t lockguard(mutex_);
		while (isFull())
		{
			notFull_.wait(); //等待不满(not full)
		}
		assert(!isFull());

		queue_.push_back(task);
		notEmpty_.notify();
	}
}