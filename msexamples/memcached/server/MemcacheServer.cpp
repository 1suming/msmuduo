#include"MemcacheServer.h"

#include<msmuduo/base/Atomic.h>
#include<msmuduo/base/Logging.h>
#include<msmuduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include<time.h>

using namespace ms;


AtomicInt64 g_cas;


MemcacheServer::Options::Options()
{
	bzero(this, sizeof(*this));
}

struct MemcacheServer::Stats
{
};

MemcacheServer::MemcacheServer(ms::EventLoop* loop, const Options& options)
	: loop_(loop),
	options_(options),
	startTime_(::time(NULL) - 1),
	server_(loop, InetAddress(options.tcpport), "muduo-memcached"),
	stats_(new Stats)
{
	server_.setConnectionCallback(
		boost::bind(&MemcacheServer::onConnection, this, _1));
}
MemcacheServer::~MemcacheServer()
{
}

void MemcacheServer::start()
{
	server_.start();
}

void MemcacheServer::stop()
{
	loop_->runAfter(3.0, boost::bind(&EventLoop::quit, loop_));
}

bool MemcacheServer::storeItem(const ItemPtr& item, const Item::UpdatePolicy policy, bool* exists)
{
	assert(item->neededBytes() == 0);
	mutex_t mutex =  shards_[item->hash() % kShards].mutex;
	ItemMap& items = shards_[item->hash() % kShards].items;

	lock_guard_t lock(mutex);
	ItemMap::const_iterator it = items.find(item);

	*exists = it != items.end();
	
	if (policy == Item::kSet)
	{
		item->setCas(g_cas.incrementAndGet());
		if (*exists)
		{
			items.erase(it); //注意，earse不会销毁指针
		}
		items.insert(item);
	}
	else
	{
		if (policy == Item::kAdd)
		{
			//如果 add 的 key 已经存在，则不会更新数据，之前的值将仍然保持相同，并且您将获得响应 NOT_STORED。
			if (*exists)
			{
				return false;
			}
			else
			{
				item->setCas(g_cas.incrementAndGet());
				items.insert(item);
			}

		}


	}

	return true;
}

ConstItemPtr MemcacheServer::getItem(const ConstItemPtr& key) const
{
	mutex_t mutex = shards_[key->hash() % kShards].mutex;
	const ItemMap& items = shards_[key->hash() % kShards].items;

	lock_guard_t lock(mutex);
	ItemMap::const_iterator it = items.find(key);
	return it != items.end() ? *it : ConstItemPtr();


}
bool MemcacheServer::deleteItem(const ConstItemPtr& key)
{
	mutex_t mutex = shards_[key->hash() % kShards].mutex;
    ItemMap& items = shards_[key->hash() % kShards].items;
	lock_guard_t lock(mutex);
	return items.erase(key) == 1;
}


void MemcacheServer::onConnection(const ms::TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		SessionPtr session(new Session(this, conn));
		lock_guard_t lock(mutex_);
		assert(sessions_.find(conn->name()) == sessions_.end());
		sessions_[conn->name()] = session;
		// assert(sessions_.size() == stats_.current_conns);
	}
	else
	{
		lock_guard_t  lock(mutex_);
		assert(sessions_.find(conn->name()) != sessions_.end());
		sessions_.erase(conn->name());
		// assert(sessions_.size() == stats_.current_conns);
	}
}