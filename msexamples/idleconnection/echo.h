#ifndef _echo_h
#define _echo_h

#include"msmuduo/net/TcpServer.h"

#include<boost/circular_buffer.hpp>
#include<boost/unordered_set.hpp>
#include<boost/version.hpp>

#if BOOST_VERSION < 104700
namespace boost
{
	template <typename T>
	inline size_t hash_value(const boost::shared_ptr<T>& x)
	{
		return boost::hash_value(x.get()); //T* get() const;
	}
}
#endif
//RFC 862
NS_USING;
class EchoServer
{
public:
	EchoServer(EventLoop* loop,
		const InetAddress& listenAddr,
		int idleSeconds);

	void start();


private:
	void onConnection(const TcpConnectionPtr& conn);

	void onMessage(const TcpConnectionPtr& conn,
		Buffer* buf, Timestamp time);


	void onTimer();

	void dumpConnectionBuckets() const;


	typedef boost::weak_ptr<TcpConnection> WeakTcpConnectionPtr;

	struct Entry : public copyable
	{
		explicit Entry(const WeakTcpConnectionPtr& weakConn) :
		weakConn_(weakConn)
		{

		}

		~Entry()
		{
			TcpConnectionPtr conn = weakConn_.lock();
			if (conn)
			{
				conn->shutdown();

			}
		}

		WeakTcpConnectionPtr weakConn_;
	};

	typedef boost::shared_ptr<Entry> EntryPtr;
	typedef boost::weak_ptr<Entry> WeakEntryPtr;

	typedef boost::unordered_set<EntryPtr> Bucket;
	typedef boost::circular_buffer<Bucket> WeakConnectionList;



	TcpServer server_;

	WeakConnectionList connectionBuckets_;
};


#endif