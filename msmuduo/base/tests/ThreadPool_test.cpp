#include"../../stdafx.h"
#include"../ThreadPool.h"
#include"../CountDownLatch.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include<iostream>
using namespace std;

using namespace ms;

void print()
{
	printf("tid=%d\n", CurrentThread::tid());

}
void printString(const std::string& str)
{
	printf("%s\n", str.c_str());
	sleep(100);
}
void test(int maxSize)
{
	cout << "Test ThreadPool with max queue size = " << maxSize<<endl;
	ThreadPool pool("MainThreadPool");
	pool.setMaxQueueSize(maxSize);
	pool.start(5);

	cout << "Adding" << endl;

	//pool.run(print);
	//pool.run(print);

	for (int i = 0; i < 10; i++)
	{
		cout << i << "-" << ends;
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", i);
		pool.run(boost::bind(printString, std::string(buf)));

	}
	cout << "Done" << endl;
	printf("\nqueueSize:%d\n", pool.queueSize());
	
	CountDownLatch latch(1);
	pool.run(boost::bind(&CountDownLatch::countDown,&latch));
	latch.wait();
	
	pool.stop();
	printf("\nqueueSize:%d\n", pool.queueSize());


}
int main()
{
	test(0);
	test(1);
	test(5);
	test(10);
	test(50);
}