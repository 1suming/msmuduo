#ifndef _EventLoopThreadPool_h
#define _EventLoopThreadPool_h

#include<boost/function.hpp>
#include<boost/noncopyable.hpp>
#include<boost/ptr_container/ptr_vector.hpp>

NS_BEGIN


class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
public:
	typedef boost::function<void(EventLoop*)> ThreadInitCallback;

	//构造函数需要一个EventLoop,
	EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start(const ThreadInitCallback& cb = ThreadInitCallback());

	//valid after calling start()
	//round-robin
	EventLoop* getNextLoop();

	/// with the same hash code, it will always return the same EventLoop
	EventLoop* getLoopForHash(size_t hashCode);

	std::vector<EventLoop*> getAllLoops();

	bool started() const { return started_; }
	const string& name() const { return name_; }



private:
	EventLoop* baseLoop_;
	string name_;
	bool started_;
	int numThreads_;
	int next_;

	boost::ptr_vector<EventLoopThread> threads_;
	std::vector<EventLoop*> loops_;


};



NS_END
#endif