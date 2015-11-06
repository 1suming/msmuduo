
#ifndef _lock_h_
#define _lock_h_

// ����windows��Ϊ�������ͻ�������ϣ�linux����Ϊ������
namespace ms
{
	class mutex_t
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

class lock_guard_t
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
	mutex_t& m_mutex; //һ��Ҫ������,����������ã�����ÿ���´�����һ��mutex_t.

};

#define lock_guard_t(x) printf("\nMissing guard object name!!!\n")


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