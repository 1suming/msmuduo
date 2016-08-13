#ifndef _yqueue_hpp
#define _yqueue_hpp
#include"msmuduo/stdafx.h"

#include<stdlib.h>
#include<stddef.h>

NS_BEGIN
//  yqueue is an efficient queue implementation. The main goal is
//  to minimise number of allocations/deallocations needed. Thus yqueue
//  allocates/deallocates elements in batches of N.
//
//  yqueue allows one thread to use push/back function and another one 
//  to use pop/front functions. However, user must ensure that there's no
//  pop on the empty queue and that both threads don't access the same
//  element in unsynchronised manner.
//

/*
yqueue是一个高效的队列实现，主要目标是最小化内存alloc和dealloc。
yqueue线程安全性：允许一个thread pop&frfont,另一个push&pack，然后，用户必须保证不再empty的queue
上pop。 同时两个线程不能在不同步的情况下访问同一个元素

*/

//  T is the type of the object in the queue
//  N is granularity of the queue (how many pushes have to be done till
//  actual memory allocation is required)

/*

 */
template<typename T,int N> 
class yqueue_t
{
public:
	//Create the queue
	//!important:新建一个queue，back_chunk指向NULL表示没有队尾，与一般的queue不同.
	inline yqueue_t()
	{
		begin_chunk = (chunk_t*)malloc(sizeof(chunk_t));
		begin_pos = 0;

		back_chunk = NULL;
		back_pos = 0;

		end_chunk = begin_chunk;
		end_pos = 0;

	}
	//Destroy the queue
	inline ~yqueue_t()
	{
		while (true)
		{

			if (begin_chunk == end_chunk)
			{
				free(begin_chunk);
				break;
			}

			chunk_t* o = begin_chunk;
			begin_chunk = begin_chunk->next;
			free(o);
		}

		chunk_t *sc = spare_chunk.xchg(NULL);
		if (sc)
		{
			free(sc);
		}

	}

	//  Returns reference to the front element of the queue.
	//  If the queue is empty, behaviour is undefined.
	//这个front和pop结合使用
	inline T& front()
	{
		return begin_chunk->values[begin_pos];
	}

	//  Returns reference to the back element of the queue.
	//  If the queue is empty, behaviour is undefined.
	//这个back函数一般和push函数结合使用
	inline T& back()
	{
		return back_chunk->values[back_pos];

	}
	//Add an element to the back end of the queue
	inline void push()
	{
		/*
		从这里可以看出为什么要弄back_chunk和end_chunk这2个成员，具体有什么区别
		back_chunk表示队列为
		end_chunk表示push后的尾巴
		存在这种情况，back_pos位置恰好是n-1,push后endpos为N,就要新建一个chunk. 区分开back_pos和end_pos
		是为了不影响back(),这时候我们调用back应该是返回n-1,而不是endpos的0 .\

		push后end_pos和back_pos总是相差1
		
		*/
		back_chunk = end_chunk;
		back_pos = end_pos;

		//这个很重要，如果一个chunk_t满了后要新建一个chunk_t,没满就直接返回
		if (++end_pos != N)//如果end_pos的位置还没有到N,不必做其他的操作，直接返回
			return;

		chunk_t *sc = spare_chunk.xchg(NULL);
		if (sc)
		{
			end_chunk->next = sc;  
			sc->prev = end_chunk;
		}
		else
		{
			end_chunk->next = (chunk_t*)malloc(sizeof(chunk_t));

			end_chunk->next->prev = end_chunk;
		}
		//chunk_t *end_chunk; // push之后的chunk_t的结构指针  
		end_chunk = end_chunk->next; //这个很重要，如果一个chunk_t满了后要新建一个chunk_t,并设置end_chunk为最后一个
		end_pos = 0;

	}
	//  Removes element from the back end of the queue. In other words
	//  it rollbacks last push to the queue. Take care: Caller is
	//  responsible for destroying the object being unpushed.
	//  The caller must also guarantee that the queue isn't empty when
	//  unpush is called. It cannot be done automatically as the read
	//  side of the queue can be managed by different, completely
	//  unsynchronised thread.
	//从最后删除一个元素，调用者负责destroy这个弹出的元素.caller必须当调用时保证队列非空
	//因为这个不能自动做到，因为读取队列的一边 can be managed by different ,complete unsync thread
	inline void unpush()
	{
		//back_pos和end_pos不一定相等，看push函数
		if (back_pos)//不为0
		{
			--back_pos;
		}
		else
		{
			back_pos = N - 1;
			back_chunk = back_chunk->prev;

		}

		//  Now, move 'end' position backwards. Note that obsolete end chunk
		//  is not used as a spare chunk. The analysis shows that doing so
		//  would require free and atomic operation per chunk deallocated
		//  instead of a simple free.
		if (end_pos)
		{
			--end_pos;
		}
		else
		{
			end_pos = N - 1;
			end_chunk = end_chunk->prev;
			free(end_chunk->next);
			end_chunk->next = NULL;
		}


	}

	//Removes an element from the front end of the queue
	inline void pop()
	{
		//调用pos函数首先把begin_pos加1，如果最后begin_pos<N则直接返回
		if (++begin_pos == N)
		{
			chunk_t* o = begin_chunk;
			begin_chunk = begin_chunk->next;
			begin_chunk->prev = NULL;
			begin_pos = 0;

			//  'o' has been more recently used than spare_chunk,
			//  so for cache reasons we'll get rid of the spare and
			//  use 'o' as the spare.

			chunk_t* cs = spare_chunk.xchg(o);
			if (cs)
				free(cs);


		}
	}



private:
	//Individual memory chunk to hold N elements
	struct chunk_t{
		T values[N];
		chunk_t *prev;
		chunk_t *next;
	};
	//  Back position may point to invalid memory if the queue is empty,
	//  while begin & end positions are always valid. Begin position is
	//  accessed exclusively be queue reader (front/pop), while back and
	//  end positions are accessed exclusively by queue writer (back/push).
	chunk_t* begin_chunk; // 指向队头chunk_t的指针  
	int begin_pos;// 在队头chunk_t中队首元素在数组values中的索引位置  
	chunk_t* back_chunk;// 指向队尾chunk_t的指针  
	int back_pos;// 在队尾chunk_t中队尾元素在数组values中的索引位置  
	chunk_t* end_chunk;// push之后的chunk_t的结构指针   /./end_chunk就是拿来扩容的，总是指向链表的最后一个结
	int end_pos;// push之后的chunk_t的元素在数组values中的索引位置  

	//  People are likely to produce and consume at similar rates.  In
	//  this scenario holding onto the most recently freed chunk saves
	//  us from having to call malloc/free.
	//经常生产者和消费者速度不对称，在这种情况下保存经常freed chunk.避免频繁调用malloc
	atomic_ptr_t<chunk_t> spare_chunk;
	//spare_chunk，，扩容（写线程的事）和出队列（写线程的事）都会用到这个变量，所以这个变量是读写共享的，有同步的语义

	//  Disable copying of yqueue.
	yqueue_t(const yqueue_t&);
	const yqueue_t &operator = (const yqueue_t&);


};


NS_END

#endif