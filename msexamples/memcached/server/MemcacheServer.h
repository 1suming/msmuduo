#ifndef _MemcacheServer_h
#define _MemcacheServer_h

#include<boost/array.hpp>

#include<boost/noncopyable.hpp>

class MemcacheServer : boost::noncopyable
{
public:
	struct Options
	{
		Options();
		uint16_t tcpport;
		uint16_t udpport;
		uint16_t gperfport;
		int threads;

	};

	MemcacheServer(ms::EventLoop* loop, const Options&);

};


#endif