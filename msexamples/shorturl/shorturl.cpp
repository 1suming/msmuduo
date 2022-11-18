#include"msmuduo/net/http/HttpRequest.h"
#include"msmuduo/net/http/HttpResponse.h"
#include"msmuduo/net/http/HttpServer.h"

#include"msmuduo/net/EventLoop.h"
#include"msmuduo/net/EventLoopThreadPool.h"
#include"msmuduo/base/Logging.h"

#include<map>

#ifdef LINUX 

	#include<sys/socket.h>
#endif 

NS_USING;


extern char favicon[555];
bool benchmark = false;

char favicon[555] = {
  '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',

};

std::map<string, string> redirections;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
	LOG_INFO << "Headers " << req.methodString() << " " << req.path();
	if (!benchmark)
	{
		const std::map<string, string>& headers = req.headers();
		for (std::map<string, string>::const_iterator it = headers.begin();
			it != headers.end();
			++it)
		{
			LOG_DEBUG << it->first << ": " << it->second;
		}
	}

	// TODO: support PUT and DELETE to create new redirections on-the-fly.


	std::map<string, string>::const_iterator it = redirections.find(req.path());
	if (it != redirections.end()) {
		resp->setStatusCode(HttpResponse::k301MovedPermanently);
		resp->setStatusMessage("Moved Permanently");
		resp->addHeader("Location", it->second);
		// resp->setCloseConnection(true);
	}
	else if (req.path() == "/")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/html");
		string now = Timestamp::now().toFormattedString();
		std::map<string, string>::const_iterator i = redirections.begin();
		string text;
		for (; i != redirections.end(); ++i)
		{
			text.append("<ul>" + i->first + " =&gt; " + i->second + "</ul>");
		}

		resp->setBody("<html><head><title>My tiny short url service</title></head>"
			"<body><h1>Known redirections</h1>"
			+ text +
			"Now is " + now +
			"</body></html>");
	}
	else if (req.path() == "/favicon.ico")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("image/png");
		resp->setBody(string(favicon, sizeof favicon));
	}
	else
	{
		resp->setStatusCode(HttpResponse::k404NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);
	}


}
int main(int argc,char* argv[])
{
	redirections["/1"] = "http://a.com";
	redirections["/2"] = "http://b.com";

	int numThreads = 0;
	if (argc > 1) {
		benchmark = true;
		Logger::setLogLevel(Logger::WARN);
		numThreads = atoi(argv[1]);

	}

#ifdef SO_REUSEPORT 
	LOG_WARN << "SO_REUSEPORT";
	EventLoop loop;
	EventLoopThreadPool threadPool(&loop, "shorturl");
	if (numThreads > 1) {
		threadpool.setThreadNum(numThreads);
	}
	else
	{
		numThreads = 1;
	}
	threadPool.start();
	std::vector<std::unique_ptr<HttpServer>> servers;
	for (int i = 0; i < numThreads; ++i)
	{
		servers.emplace_back(new HttpServer(threadPool.getNextLoop(),
			InetAddress(8000),
			"shorturl",
			TcpServer::kReusePort));
		servers.back()->setHttpCallback(onRequest);
		servers.back()->getLoop()->runInLoop(
			std::bind(&HttpServer::start, servers.back().get()));
	}
	loop.loop();


#else 
	LOG_WARN << "Normal";
	EventLoop loop;
	HttpServer server(&loop, InetAddress(8000), "shorturl");

	server.setHttpCallback(onRequest);
	server.setThreadNum(numThreads);
	server.start();
	loop.loop();

#endif 
}

