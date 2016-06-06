#include"../stdafx.h"

#include"AsyncLogging.h"

NS_USING;

AsyncLogging::AsyncLogging(string& basename,
	size_t rollSize,
	int flushInterval):
	flushInterval_(flushInterval),
	running_(false),
	basename_(basename),
	rollSize_(rollSize),
	thread_(boost::bind(&AsyncLogging::threadFunc,this), "Logging"),
	latch_(1),
	mutex_(),
	cond_(mutex_),
	currentBuffer_(new Buffer)£¬
	nextBuffer_(new Buffer),
	buffers_()
{
		currentBuffer_->bzero();
		nextBuffer_->bzero();
		buffers_.reserve(16);
 

}

void AsyncLogging::append(const char* logline, int len)
{
	ms::lock_guard_t lock(mutex_);
	if (currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		buffers_.push_back(currentBuffer_->release());

	}
}