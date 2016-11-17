#ifndef _MemcacheServer_h
#define _MemcacheServer_h

#include"Item.h"
#include"Session.h"



#include"msmuduo/base/lock.h"
#include"msmuduo/net/TcpServer.h"


#include<boost/array.hpp>

#include<boost/noncopyable.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

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
	~MemcacheServer();

	void setThreadNum(int threads) { server_.setThreadNum(threads); }
	void start();
	void stop();

	time_t startTime() const { return startTime_; }

	bool storeItem(const ItemPtr& item, Item::UpdatePolicy policy, bool* exists);
	ConstItemPtr getItem(const ConstItemPtr& key) const;
	bool deleteItem(const ConstItemPtr& key);




private:
	void onConnection(const ms::TcpConnectionPtr& conn);

	struct Stats;

	ms::EventLoop* loop_;
	Options options_;
	const time_t startTime_;

	mutable ms::mutex_t mutex_;
	boost::unordered_map<string, SessionPtr> sessions_;

	//a complicated solution to save memory
	struct Hash
	{
		size_t operator()(const ConstItemPtr& x) const
		{
			return x->hash();
		}
	};
	struct Equal
	{
		bool operator() (const ConstItemPtr& x, const ConstItemPtr& y) const
		{
			return x->key() == y->key();
		}
	};

	typedef boost::unordered_set<ConstItemPtr, Hash, Equal> ItemMap;


	struct MapWithLock
	{
		ItemMap items;
		mutable ms::mutex_t mutex;
	};

	const static int kShards = 4096;

	boost::array<MapWithLock, kShards> shards_;

	// NOT guarded by mutex_, but here because server_ has to destructs before
	// sessions_
	ms::TcpServer server_;
	boost::scoped_ptr<Stats> stats_;
};


#endif