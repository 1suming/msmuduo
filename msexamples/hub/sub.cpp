 

#include"pubsub.h"

#include<msmuduo/net/EventLoop.h>
 
#include <boost/bind.hpp>
#include <vector>
#include <stdio.h>



#include <iostream>
#include <stdio.h>

NS_USING;
using namespace pubsub;
 
EventLoop* g_loop = NULL;
std::vector<string> g_topics;

void subscription(const string& topic, const string& content, Timestamp)
{
	printf("%s: %s\n", topic.c_str(), content.c_str());
}

void connection(PubSubClient* client)
{
	if (client->connected())
	{
		//一旦连上hub，发订阅的消息
		for (std::vector<string>::iterator it = g_topics.begin();
			it != g_topics.end(); it++)
		{
			client->subscribe(*it, subscription);

		}
	}
	else
	{
		g_loop->quit();
	}
}
int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		string hostport = argv[1];
		size_t colon = hostport.find(':');
		if (colon != string::npos)
		{
			string hostip = hostport.substr(0, colon);
			uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));

			for (int i = 2; i < argc; i++)
			{
				g_topics.push_back(argv[i]);
			}

			EventLoop loop;
			g_loop = &loop;


			PubSubClient client(&loop, InetAddress(hostip, port), "sub");
			client.setConnectionCallback(connection);
			client.start();
			loop.loop();

		}
		else
		{
			printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
		}


	}
	else
	{
		printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
	}
}