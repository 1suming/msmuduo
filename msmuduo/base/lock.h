
#ifndef _lock_h_
#define _lock_h_

#include"msmuduo/stdafx.h" //把这个放在.h中

/*
 fastmutex不能和条件变量结合使用
 
*/
#include"msmuduo/base/fast_mutex.h"



// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }



// 锁：windows下为自旋锁和互斥锁混合，linux下则为互斥锁
namespace ms
{
 	class CAPABILITY("mutext") mutex_t
	{
	public:
		mutex_t();
		~mutex_t();
		bool lock();
		bool unlock();
		bool try_lock();

#ifdef WIN
	public:
		CRITICAL_SECTION& get_mutex(){return m_lock;}
	private:
		CRITICAL_SECTION m_lock;
#else
	public:
		pthread_mutex_t& get_mutex(){ return m_mutex; }
	private:
		pthread_mutex_t m_mutex;
#endif

	};
 
class SCOPED_CAPABILITY lock_guard_t
{
 

public:
	lock_guard_t(mutex_t& mutex) :
		m_mutex(mutex)
		{
			m_mutex.lock();
		}
		~lock_guard_t()
		{
			m_mutex.unlock();
		}

private:
 	mutex_t& m_mutex; //一定要是引用,如果不是引用，就是每次新创建了一个mutex_t.
 

};
//#define lock_guard_t(x) printf("\nMissing guard object name!!!\n")
#define MutexLockGuard(x) error "Missing guard object name"

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define lock_guard_t(x) error "Missing guard object name"


class lock_guard_fastmutex_t
{
public:
	lock_guard_fastmutex_t(fast_mutex& mutex) :
	m_mutex(mutex)
	{
		m_mutex.lock();
	}
	~lock_guard_fastmutex_t()
	{
		m_mutex.unlock();
	}

private:
	fast_mutex& m_mutex;

};
#define lock_guard_fastmutex_t(x) printf("\nMissing guard object name!!!\n")



class condition_var_t
{
public:
	condition_var_t(mutex_t& mutex_);
	~condition_var_t();
	
	bool wait();
	bool wait(int ms);

	bool notify();
	bool notifyAll();

	

private:
	mutex_t& m_mutex;

#ifdef WIN
	CONDITION_VARIABLE m_condVariable;
#else
	pthread_cond_t m_cond;
#endif

};


}//end namespace
#endif