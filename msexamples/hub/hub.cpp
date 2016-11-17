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
 
//�����ߺϼ�
typedef std::set<string> ConnectionSubscription;


/*
һ��Topic���� �����ߵļ���.
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
		if (lastPubTime_.valid())//��������һ��sub���������ͷ�֮ǰ����Ϣ����
		{
			conn->send(makeMessage());
		}
	}
	void remove(const TcpConnectionPtr& conn)
	{
		audiences_.erase(conn);
	}
	//�������еĶ�����
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

		//���˿��ķ��㣬ע�͵�����
		//loop_->runEvery(1.0, boost::bind(&PubSubServer::timePubish, this));


	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		//���ܿͻ�������������sub����pub�������ȴ���һ��ConnectionSubscription����Ȼ��sub��˵��û�б�Ҫ
		if (conn->connected())
		{
			conn->setContext(ConnectionSubscription()); //����һ��clinet���ĵ�����


		}
		else
		{
			//���ӶϿ��������client���ĵ�topic���涼ȡ��
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
		while (result == kSuccess) //whileѭ������resultһֱΪkSuccess
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
	//TODO:�������û�д����ͻ���sub��topic�����ڵ����
	void doSubscribe(const TcpConnectionPtr& conn, const string& topic)
	{
		//getMutableContext���ص��ǣ�boost::any*
		//Ϊʲôany_cast�õ���ConnectionSubscripton��������ָ�룿������boost:any_castԭ��
		/*
		 template<typename ValueType>
		 ValueType * any_cast(any * operand)
		 */
		LOG_OK << conn->name() << " sub " << topic;
		ConnectionSubscription* connSub =
			boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
		 
		
		connSub->insert(topic);//��������ͻ��˶��ĵ�����
		getTopic(topic).add(conn);


 	}
	void doUnsubscribe(const TcpConnectionPtr& conn, const string& topic)
	{
		LOG_OK << conn->name() << " unsub " << topic;

		getTopic(topic).remove(conn);

		// topic could be the one to be destroyed, so don't use it after erasing.
		//�ͻ��˲�����ĳ��topic��������
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

	//����topic�ַ�����Topic��ӳ��
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