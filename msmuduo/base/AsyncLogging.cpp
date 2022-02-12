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
	//boost 1.78���벻ͨ�� currentBuffer_( new Buffer  ), //create buffer
	//boost 1.78���벻ͨ�� nextBuffer_(new Buffer),
	buffers_()
{
	 
	currentBuffer_.ptr() = new Buffer ;//boost 1.78�����ַ�ʽ�ñ���ͨ��
	nextBuffer_.ptr() = new Buffer;

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
			currentBuffer_ = boost::ptr_container::move(nextBuffer_); //�ƶ������Ǹ���

		}
		else //allocate a new one
		{
			//boost 1.78���벻ͨ�� currentBuffer_.reset(new  Buffer); //Rarely happends  
			currentBuffer_.reset();
			currentBuffer_.ptr() = (new  Buffer);
			/*
			����std::auto_ptr
			��reset()��������������ݲ��������ߴ���NULL����������ָ����ͷŵ�ǰ������ڴ档�������һ������������ָ����ͷŵ�ǰ�����������´���Ķ���
			
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

	LogFile output(basename_, rollSize_, false); //Important ����Ĵ����threadsafeΪfalse��Ϊʲô����Ϊ�첽����ͱ�֤���̰߳�ȫ
	//boost 1.78���벻ͨ��BufferPtr newBuffer1(new Buffer);
	//boost 1.78���벻ͨ�� BufferPtr newBuffer2(new Buffer);
	BufferPtr newBuffer1;
	BufferPtr newBuffer2;

	newBuffer1.ptr() = new Buffer;
	newBuffer2.ptr() = new Buffer;

	newBuffer1->bzeroinit();
	newBuffer2->bzeroinit();

	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);

	while (running_)
	{

		{
			lock_guard_t lock(mutex_); //�����������߻���
			if (buffers_.empty()) //unsual usage,û��ʹ��whileѭ��
			{
				cond_.wait(flushInterval_ * 1000);//flushInterval_��seconds

			}
			//ִ�е�������2����������һ�ǳ�ʱ�������д����һ������buffer
			buffers_.push_back(currentBuffer_.release());//�ƶ������Ǹ��� ,release����std::auto_ptr��release,  ���ذ���ָ��ĵ�ַ����������ָ����ΪNUll
			currentBuffer_ = boost::ptr_container::move(newBuffer1);//�ƶ������Ǹ���
			buffersToWrite.swap(buffers_);//�ڲ�ָ�뽻�������Ǹ��� ,�����󣬿����ٽ����ⰲȫ�ط���buffersToWrite�������е���־����д���ļ�

			if (!nextBuffer_)//�ٽ������ոɵ�һ����������newBuffer2�滻nextBuffer_,����ǰ��ʼ����һ������buffer,nextbuffer_����ǰ�˷����ڴ�ĸ���
			{
				nextBuffer_ = boost::ptr_container::move(newBuffer2);
			}


		}


		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25)//��־�ѻ���ֱ�Ӷ���
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
			newBuffer1->reset(); //��buffer��ָ��ָ��ͷ��
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