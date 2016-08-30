#ifndef _pub_h
#define _pub_h


#include"pubsub.h"

#include<msmuduo/net/EventLoop.h>
#include<msmuduo/net/EventLoopThread.h>

#include <iostream>
#include <stdio.h>


NS_USING;
using namespace pubsub;


EventLoop* g_loop = NULL;
string g_topic;
string g_content;

void connection(PubSubClient* client)
{
	if (client->connected())
	{
		client->publish(g_topic, g_content);
		client->stop(); //stop的方法是disconnect
	}
	else
	{
		g_loop->quit();
	}
}
int main(int argc, char* argv[])
{
	if (argc == 4)
	{
		string hostport = argv[1];
		size_t colon = hostport.find(':');
		if (colon != string::npos)
		{
			string hostip = hostport.substr(0, colon);
			uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));

			g_topic = argv[2];
			g_content = argv[3];

			//如果content是'-'
			if (g_content == "-")
			{
				EventLoopThread loopThread;
				g_loop = loopThread.startLoop();

				PubSubClient client(g_loop, InetAddress(hostip, port), "Pub");
				client.start();


				string line;
				while (getline(std::cin, line)) //接收用户输入才publish
				{
					client.publish(g_topic, line);
				}
				client.stop();
				sleep(1);

			}
			else
			{
				EventLoop loop;
				g_loop = &loop;
				PubSubClient client(g_loop, InetAddress(hostip, port), "Pub");
				client.setConnectionCallback(connection);
				client.start();
				loop.loop();

			}
		}

	}
	else
	{
		printf("Usage: %s hub_ip:port topic content\n"
			"Read contents from stdin:\n"
			"  %s hub_ip:port topic -\n", argv[0], argv[0]);
	}

}
#endif