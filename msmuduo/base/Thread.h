#ifndef _Thread_h
#define _Thread_h


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/atomic.hpp>

#include<string>
using namespace std;

#ifdef WIN
	typedef HANDLE THANDLE;
#else 
	typedef pthread_t THANDLE;
#endif


namespace ms
{
 
namespace CurrentThread
{
	inline int tid();
}
class Thread : boost::noncopyable
{
public:
	typedef boost::function<void()> ThreadFunc;

#ifdef WIN
	static unsigned int __stdcall run_helper(void *arg)
#else 
	static void* run_helper(void* arg)
#endif
	{
		
		Thread *t = (Thread*)arg;
		t->run();
		return NULL;

	}
	void run()
	{
		tid_=CurrentThread::tid();//可能在调用时tid_还是0.
		func_();
	}
	 

	bool started() const { return started_; }
	const string& name() const { return name_; }
	static int numCreated() { return numCreated_; }

	int tid(){return tid_;}

	explicit Thread(const ThreadFunc& func, const string name = string())
		:started_(false),
		joined_(false),
		threadId_(0),
		tid_(0),
		func_(func),
		name_(name)

	{
		setDefaultName();
	}
	void start()
	{
		setDefaultName();
	
		assert(!started_);
		started_ = true;

#ifdef WIN
		unsigned int thrdaddr;
		 
		threadId_ = (HANDLE)_beginthreadex(NULL, 0,run_helper, this, 0, &thrdaddr);
		//printf("%d\n", thrdaddr);
		//printf("%dyes\n",threadId_);
		if (threadId_ == 0)
		{
			//printf("failed to create thread!!!");
			return;
		}
			 
		tid_ = thrdaddr;
#else
		if(0==::pthread_create(&threadId_,NULL,run_helper,this))
		{

		}
		else
		{
			//printf("failed to create thread!!!");
			return;
		}

#endif

	}
	int join()
	{
		assert(started_);
		assert(!joined_);
		joined_ = true;
#ifdef WIN
		WaitForSingleObject(threadId_, INFINITE);
		CloseHandle(threadId_);

#else
		::pthread_join(threadId_,NULL);
#endif

		return 0;
	}
	~Thread()
	{
		if (started_ && !joined_)
		{

		}
	}

private:
	void setDefaultName()
	{
		numCreated_++;
		int num = numCreated_;
		if (name_.empty())
		{
			char buf[32];
			snprintf(buf, sizeof buf, "Thread%d", num);
			name_ = buf;
		}
	}




private:
	bool started_;
	bool joined_;

	THANDLE threadId_;
	//boost::shared_ptr<int> tid_;
	int tid_;

	string     name_;

	ThreadFunc func_;

	static boost::atomic_int numCreated_;

};

namespace CurrentThread
{
	//internal 
	extern threadlocal  int t_cachedTid;
	void cacheTid();

	inline int tid()
	{
		if (t_cachedTid == 0)
		{
			cacheTid();
		}
		return t_cachedTid;

	}
	bool isMainThread();
}


 
}
#endif