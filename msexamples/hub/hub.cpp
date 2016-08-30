#include"codec.h"

#include<msmuduo/base/Logging.h>
#include<msmuduo/net/TcpServer.h>
#include<msmuduo/net/EventLoop.h>
 
#include<boost/bind.hpp>


#include <map>
#include <set>
#include <stdio.h>

using namespace ms;

CURR_NS_BEGIN
 
//订阅者合集
typedef std::set<string> ConnectionSubscription;


/*
一个Topic保存 订阅者的集合.
*/
class Topic : public ms::copyable
{
public:
	Topic(const string& topic) :
		topic_(topic)
	{

	}

	void add(const TcpConnectionPtr& conn)
	{
		audiences_.insert(conn);
		if (lastPubTime_.valid())//这样做，一有sub连上来，就发之前的消息给他
		{
			conn->send(makeMessage());
		}
	}
	void remove(const TcpConnectionPtr& conn)
	{
		audiences_.erase(conn);
	}
	//发给所有的订阅者
	void publish(const string& content, Timestamp time)
	{
		content_ = content;
		lastPubTime_ = time;
		string message = makeMessage();
		for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin();
			it != audiences_.end(); it++)
		{
			(*it)->send(message);
		}
	}
private:
	string makeMessage()
	{
		return "pub " + topic_ + "\r\n" + content_ + "\r\n";
	}
private:
	string topic_;
	string content_;
	Timestamp lastPubTime_;
	std::set<TcpConnectionPtr> audiences_;


};

class PubSubServer :boost::noncopyable
{
public:
	PubSubServer(ms::EventLoop* loop, const ms::InetAddress& listenAddr) :
		loop_(loop),
		server_(loop, listenAddr, "PubSubServer")
	{
		server_.setConnectionCallback(
			boost::bind(&PubSubServer::onConnection, this, _1));
		server_.setMessageCallback(
			boost::bind(&PubSubServer::onMessage, this, _1, _2, _3));

		//看了看的方便，注释掉下面
		//loop_->runEvery(1.0, boost::bind(&PubSubServer::timePubish, this));


	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		//不管客户端连上来的是sub还是pub，都首先创建一个ConnectionSubscription，虽然对sub来说，没有必要
		if (conn->connected())
		{
			conn->setContext(ConnectionSubscription()); //保存一个clinet订阅的主题


		}
		else
		{
			//连接断开，把这个client订阅的topic里面都取消
			const ConnectionSubscription& connSub
				= boost::any_cast<const ConnectionSubscription&>(conn->getContext());
			// subtle: doUnsubscribe will erase *it, so increase before calling.
			for (ConnectionSubscription::const_iterator it = connSub.begin();
				it != connSub.end();)
			{
				doUnsubscribe(conn, *it++);
			}

		}
	}
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
	{
		ParseResult result = kSuccess;
		while (result == kSuccess) //while循环，当result一直为kSuccess
		{
			string cmd;
			string topic;
			string content;

			result = parseMessage(buf, &cmd, &topic, &content);
			if (result == kSuccess)
			{
				if (cmd == "pub")
				{
					doPublish(conn->name(), topic, content, receiveTime);
				}
				else if (cmd == "sub")
				{
					LOG_INFO << conn->name() << " subscribes " << topic;
					doSubscribe(conn, topic);


				}
				else if (cmd == "unsub")
				{
					doUnsubscribe(conn, topic);

				}
				else
				{
					conn->shutdown();
					result = kError;
				}
			}
			else if (result == kError)
			{
				conn->shutdown();
			}
		}
	}


	void doPublish(const string& source,
		const string& topic,
		const string& content,
		Timestamp time)
	{
		getTopic(topic).publish(content, time);
	}

	Topic& getTopic(const string& topic)
	{
		std::map<string, Topic>::iterator it = topics_.find(topic);
		if (it == topics_.end())
		{
			it = topics_.insert(make_pair(topic, Topic(topic))).first; //pair<iterator,bool> insert (const value_type& val);

		}

		return it->second;
	}
	//TODO:这个函数没有处理客户端sub的topic不存在的情况
	void doSubscribe(const TcpConnectionPtr& conn, const string& topic)
	{
		//getMutableContext返回的是：boost::any*
		//为什么any_cast用的是ConnectionSubscripton，而不是指针？？？看boost:any_cast原型
		/*
		 template<typename ValueType>
		 ValueType * any_cast(any * operand)
		 */
		LOG_OK << conn->name() << " sub " << topic;
		ConnectionSubscription* connSub =
			boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
		 
		
		connSub->insert(topic);//保存这个客户端订阅的主题
		getTopic(topic).add(conn);


 	}
	void doUnsubscribe(const TcpConnectionPtr& conn, const string& topic)
	{
		LOG_OK << conn->name() << " unsub " << topic;

		getTopic(topic).remove(conn);

		// topic could be the one to be destroyed, so don't use it after erasing.
		//客户端不订阅某个topic，就销毁
		ConnectionSubscription* connSub = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());

		connSub->erase(topic);

	}

	void timePubish()
	{
		Timestamp now = Timestamp::now();
		doPublish("internal", "utc_time", now.toFormattedString(), now);
	}
private:
	EventLoop* loop_;
	TcpServer server_;

	//保存topic字符串到Topic的映射
	std::map<string, Topic> topics_;
	

};
CURR_NS_END

CURR_NS_USING;

int main(int argc, char* argv[])
{
	ms::Logger::setLogLevel(Logger::TRACE);
	if (argc > 1)
	{
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		EventLoop loop;
		if (argc > 2)
		{
			//int inspectPort = atoi(argv[2]);
		}
		PubSubServer server(&loop, InetAddress(port));
		server.start();
		loop.loop();
 

	}
	else
	{
		printf("Usage: %s pubsub_port \n", argv[0]);
	}
}