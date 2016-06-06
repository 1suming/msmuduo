#ifndef  _AsyncLogging_h
#define  _AsyncLogging_h

#include<boost/bind.hpp>
#include<boost/noncopyable.hpp>
#include<boost/ptr_container/ptr_vector.hpp>
#include<boost/scoped_ptr.hpp>


#include"Thread.h"
#include"CountDownLatch.h"
#include"lock.h"
#include"LogStream.h"




NS_BEGIN
class AsyncLogging : boost::noncopyable
{

public:
	AsyncLogging(const string& basename,
		size_t rollSize,
		int flushInterval = 3);

	~AsyncLogging()
	{
		if (running_)
			stop();
	}
	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.start();
		latch_.wait();

	}
	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:
	//declare but not define,prevent compiler-synthesized function  synthesize: 合成;综合
	AsyncLogging(const AsyncLogging&);  // ptr_container
	void operator=(const AsyncLogging&);  // ptr_container

	void threadFunc();

	typedef ms::detail::FixedBuffer<ms::detail::kLargeBuffer> Buffer;
	typedef boost::ptr_vector<Buffer> BufferVector;
	typedef BufferVector::auto_type BufferPtr;  //auto_type是数据的智能指针,类似于std::auto_ptr.http://www.boost.org/doc/libs/1_61_0/libs/ptr_container/doc/ptr_vector.html
	 

	const int flushInterval_;
	bool running_;
	string basename_;
	size_t rollSize_;

	ms::Thread thread_;
	ms::CountDownLatch latch_;

	ms::mutex_t mutex_;
	ms::condition_var_t cond_;

	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;


	BufferVector buffers_;



};


NS_END

#endif