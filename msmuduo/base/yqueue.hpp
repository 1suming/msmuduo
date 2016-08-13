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
yqueue��һ����Ч�Ķ���ʵ�֣���ҪĿ������С���ڴ�alloc��dealloc��
yqueue�̰߳�ȫ�ԣ�����һ��thread pop&frfont,��һ��push&pack��Ȼ���û����뱣֤����empty��queue
��pop�� ͬʱ�����̲߳����ڲ�ͬ��������·���ͬһ��Ԫ��

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
	//!important:�½�һ��queue��back_chunkָ��NULL��ʾû�ж�β����һ���queue��ͬ.
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
	//���front��pop���ʹ��
	inline T& front()
	{
		return begin_chunk->values[begin_pos];
	}

	//  Returns reference to the back element of the queue.
	//  If the queue is empty, behaviour is undefined.
	//���back����һ���push�������ʹ��
	inline T& back()
	{
		return back_chunk->values[back_pos];

	}
	//Add an element to the back end of the queue
	inline void push()
	{
		/*
		��������Կ���ΪʲôҪŪback_chunk��end_chunk��2����Ա��������ʲô����
		back_chunk��ʾ����Ϊ
		end_chunk��ʾpush���β��
		�������������back_posλ��ǡ����n-1,push��endposΪN,��Ҫ�½�һ��chunk. ���ֿ�back_pos��end_pos
		��Ϊ�˲�Ӱ��back(),��ʱ�����ǵ���backӦ���Ƿ���n-1,������endpos��0 .\

		push��end_pos��back_pos�������1
		
		*/
		back_chunk = end_chunk;
		back_pos = end_pos;

		//�������Ҫ�����һ��chunk_t���˺�Ҫ�½�һ��chunk_t,û����ֱ�ӷ���
		if (++end_pos != N)//���end_pos��λ�û�û�е�N,�����������Ĳ�����ֱ�ӷ���
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
		//chunk_t *end_chunk; // push֮���chunk_t�Ľṹָ��  
		end_chunk = end_chunk->next; //�������Ҫ�����һ��chunk_t���˺�Ҫ�½�һ��chunk_t,������end_chunkΪ���һ��
		end_pos = 0;

	}
	//  Removes element from the back end of the queue. In other words
	//  it rollbacks last push to the queue. Take care: Caller is
	//  responsible for destroying the object being unpushed.
	//  The caller must also guarantee that the queue isn't empty when
	//  unpush is called. It cannot be done automatically as the read
	//  side of the queue can be managed by different, completely
	//  unsynchronised thread.
	//�����ɾ��һ��Ԫ�أ������߸���destroy���������Ԫ��.caller���뵱����ʱ��֤���зǿ�
	//��Ϊ��������Զ���������Ϊ��ȡ���е�һ�� can be managed by different ,complete unsync thread
	inline void unpush()
	{
		//back_pos��end_pos��һ����ȣ���push����
		if (back_pos)//��Ϊ0
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
		//����pos�������Ȱ�begin_pos��1��������begin_pos<N��ֱ�ӷ���
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
	chunk_t* begin_chunk; // ָ���ͷchunk_t��ָ��  
	int begin_pos;// �ڶ�ͷchunk_t�ж���Ԫ��������values�е�����λ��  
	chunk_t* back_chunk;// ָ���βchunk_t��ָ��  
	int back_pos;// �ڶ�βchunk_t�ж�βԪ��������values�е�����λ��  
	chunk_t* end_chunk;// push֮���chunk_t�Ľṹָ��   /./end_chunk�����������ݵģ�����ָ����������һ����
	int end_pos;// push֮���chunk_t��Ԫ��������values�е�����λ��  

	//  People are likely to produce and consume at similar rates.  In
	//  this scenario holding onto the most recently freed chunk saves
	//  us from having to call malloc/free.
	//���������ߺ��������ٶȲ��Գƣ�����������±��澭��freed chunk.����Ƶ������malloc
	atomic_ptr_t<chunk_t> spare_chunk;
	//spare_chunk�������ݣ�д�̵߳��£��ͳ����У�д�̵߳��£������õ����������������������Ƕ�д����ģ���ͬ��������

	//  Disable copying of yqueue.
	yqueue_t(const yqueue_t&);
	const yqueue_t &operator = (const yqueue_t&);


};


NS_END

#endif