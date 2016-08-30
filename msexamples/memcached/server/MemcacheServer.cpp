#include"MemcacheServer.h"

#include<msmuduo/base/Atomic.h>
#include<msmuduo/base/Logging.h>
#include<msmuduo/net/EventLoop.h>


NS_USING;

ms::AtomicInt64 g_cas;

MemcacheServer::Options::Options()
{
	bzero(this, sizeof(*this));
}