#include"../stdafx.h"

#include"AsyncLogging.h"
#include"LogFile.h"
#include"Timestamp.h"

NS_USING;

AsyncLogging::AsyncLogging(const string& basename,
	size_t rollSize,
	int flushInterval)
	: flushInterval_(flushInterval),
	running_(false),
	basename_(basename),
	rollSize_(rollSize),
	thread_(boost::bind(&AsyncLogging::threadFunc, this), "Logging"),
	latch_(1),
	mutex_(),
	cond_(mutex_),
	currentBuffer_(new Buffer), //create buffer
	nextBuffer_(new Buffer),
	buffers_()
{
	currentBuffer_->bzeroinit();
	nextBuffer_->bzeroinit();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	lock_guard_t lock(mutex_);
	if (currentBuffer_->avail() > len)
	{
		//most common case: buffer is not full,copy data here
		currentBuffer_->append(logline, len);
	}
	else //buffer is full, push it ,and find next spare buffer
	{
		buffers_.push_back(currentBuffer_.release());

		if (nextBuffer_) //is there is one already,use it 
		{
			currentBuffer_ = boost::ptr_container::move(nextBuffer_); //移动，而非复制

		}
		else //allocate a new one
		{
			currentBuffer_.reset(new  Buffer); //Rarely happends 
			/*
			类似std::auto_ptr
			的reset()方法，如果不传递参数（或者传递NULL），则智能指针会释放当前管理的内存。如果传递一个对象，则智能指针会释放当前对象，来管理新传入的对象。
			
			*/
		}


		currentBuffer_->append(logline, len);

		cond_.notify(); //Notify

	}
}

void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countDown();

	LogFile output(basename_, rollSize_, false); //这里的传入的threadsafe为false
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);

	newBuffer1->bzeroinit();
	newBuffer2->bzeroinit();

	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);

	while (running_)
	{

		{
			lock_guard_t lock(mutex_); //用来和生产者互斥
			if (buffers_.empty()) //unsual usage,没有使用while循环
			{
				cond_.wait(flushInterval_ * 1000);//flushInterval_是seconds

			}
			//执行到这里有2个条件：其一是超时，其二是写满了一个或多个buffer
			buffers_.push_back(currentBuffer_.release());//移动，而非复制 ,release类似std::auto_ptr的release,  返回包含指针的地址，并将包含指针设为NUll
			currentBuffer_ = boost::ptr_container::move(newBuffer1);//移动，而非复制
			buffersToWrite.swap(buffers_);//内部指针交换，而非复制 ,交换后，可在临界区外安全地访问buffersToWrite，将其中的日志数据写入文件

			if (!nextBuffer_)//临界区最终干的一件事情是用newBuffer2替换nextBuffer_,这样前端始终有一个备用buffer,nextbuffer_减少前端分配内存的概率
			{
				nextBuffer_ = boost::ptr_container::move(newBuffer2);
			}


		}


		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25)//日志堆积，直接丢掉
		{
			char buf[256];
			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
				Timestamp::now().toFormattedString().c_str(),
				buffersToWrite.size() - 2);
			fputs(buf, stderr);
			output.append(buf, static_cast<int>(strlen(buf)));

			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}


		for (size_t i = 0; i < buffersToWrite.size(); i++)
		{
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?

			output.append(buffersToWrite[i].data(), buffersToWrite[i].length());

		}

		if (buffersToWrite.size() > 2)
		{
			// drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);
		}

		if (!newBuffer1)
		{
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.pop_back();
			newBuffer1->reset(); //将buffer的指针指向头部
		}
		if (!newBuffer2)
		{
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.pop_back();
			newBuffer2->reset();
		}


		buffersToWrite.clear();
		
		output.flush();

	}

	output.flush();
}