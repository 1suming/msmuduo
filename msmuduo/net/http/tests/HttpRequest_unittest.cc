#include"msmuduo/stdafx.h"
#include"msmuduo/net/http/HttpServer.h"
#include"msmuduo/net/http/HttpRequest.h"
#include"msmuduo/net/http/HttpResponse.h"
#include"msmuduo/net/http/HttpContext.h"
#include"msmuduo/net/EventLoop.h"
#include"msmuduo/base/Logging.h"

#include<iostream>
#include<map>



#define BOOST_TEST_MAIN  //一定要有,且要放在unit_tset.hpp的前面，否则:(unit_test_main.obj) : error LNK2019: 无法解析的外部符号 "class boost::unit_test::test_suite * __

#include <boost/test/unit_test.hpp>


NS_USING;

BOOST_AUTO_TEST_CASE(testParseRequestAllInOne)
{
	HttpContext context;
	Buffer input;
	input.append("GET /index.html HTTP/1.1\r\n"
		"Host: www.chenshuo.com\r\n"
		"\r\n");

	BOOST_CHECK(context.parseRequest(&input, Timestamp::now()));
	BOOST_CHECK(context.gotAll());
	const HttpRequest& request = context.request();
	BOOST_CHECK_EQUAL(request.method(), HttpRequest::kGet);
	BOOST_CHECK_EQUAL(request.path(), string("/index.html"));
	BOOST_CHECK_EQUAL(request.getVersion(), HttpRequest::kHttp11);
	BOOST_CHECK_EQUAL(request.getHeader("Host"), string("www.chenshuo.com"));
	BOOST_CHECK_EQUAL(request.getHeader("User-Agent"), string(""));
}

BOOST_AUTO_TEST_CASE(testParseRequestInTwoPieces)
{
	string all("GET /index.html HTTP/1.1\r\n"
		"Host: www.chenshuo.com\r\n"
		"\r\n");

	for (size_t sz1 = 0; sz1 < all.size(); ++sz1)
	{
		HttpContext context;
		Buffer input;
		input.append(all.c_str(), sz1);
		BOOST_CHECK(context.parseRequest(&input, Timestamp::now()));
		BOOST_CHECK(!context.gotAll());

		size_t sz2 = all.size() - sz1;
		input.append(all.c_str() + sz1, sz2);
		BOOST_CHECK(context.parseRequest(&input, Timestamp::now()));
		BOOST_CHECK(context.gotAll());
		const HttpRequest& request = context.request();
		BOOST_CHECK_EQUAL(request.method(), HttpRequest::kGet);
		BOOST_CHECK_EQUAL(request.path(), string("/index.html"));
		BOOST_CHECK_EQUAL(request.getVersion(), HttpRequest::kHttp11);
		BOOST_CHECK_EQUAL(request.getHeader("Host"), string("www.chenshuo.com"));
		BOOST_CHECK_EQUAL(request.getHeader("User-Agent"), string(""));
	}
}

BOOST_AUTO_TEST_CASE(testParseRequestEmptyHeaderValue)
{
	HttpContext context;
	Buffer input;
	input.append("GET /index.html HTTP/1.1\r\n"
		"Host: www.chenshuo.com\r\n"
		"User-Agent:\r\n"
		"Accept-Encoding: \r\n"
		"\r\n");

	BOOST_CHECK(context.parseRequest(&input, Timestamp::now()));
	BOOST_CHECK(context.gotAll());
	const HttpRequest& request = context.request();
	BOOST_CHECK_EQUAL(request.method(), HttpRequest::kGet);
	BOOST_CHECK_EQUAL(request.path(), string("/index.html"));
	BOOST_CHECK_EQUAL(request.getVersion(), HttpRequest::kHttp11);
	BOOST_CHECK_EQUAL(request.getHeader("Host"), string("www.chenshuo.com"));
	BOOST_CHECK_EQUAL(request.getHeader("User-Agent"), string(""));
	BOOST_CHECK_EQUAL(request.getHeader("Accept-Encoding"), string(""));
}