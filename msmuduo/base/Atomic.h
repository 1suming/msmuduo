#ifndef _Atomic_h
#define _Atomic_h

#include <boost/noncopyable.hpp>

NS_BEGIN

namespace detail
{
	template<typename T>
	class AtomicIntegerT : boost::noncopyable
	{
	public:
		AtomicIntegerT()
			: value_(0)
		{
		}

		LONG get()
		{
		#ifdef LINUX 
				return __sync_val_compare_and_swap(&value_, 0, 0);

		#else 
				return value_;
		#endif
		}

		LONG getAndAdd(LONG x)
		{
			#ifdef LINUX 
				// in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
				return __sync_fetch_and_add(&value_, x);
			#else 
					return InterlockedExchangeAdd(&value_, x);
			#endif
		}

		LONG addAndGet(LONG x)
		{
			return getAndAdd(x) + x;
		}

		LONG incrementAndGet()
		{
			return addAndGet(1);
		}

		LONG decrementAndGet()
		{
			return addAndGet(-1);
		}

		void add(LONG x)
		{
			getAndAdd(x);
		}

		void increment()
		{
			incrementAndGet();
		}

		void decrement()
		{
			decrementAndGet();
		}

		LONG getAndSet(LONG newValue)
		{
			#ifdef LINUX
				// in gcc >= 4.7: __atomic_store_n(&value, newValue, __ATOMIC_SEQ_CST)
				return __sync_lock_test_and_set(&value_, newValue);
			#else 
				return InterlockedExchange(&value_, newValue);
			#endif
		}

	private:
		volatile LONG value_;
	};
}

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;


NS_END

#endif