#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONNECTION_NUM 1100000
/*
TCP SO_REUSEPORT 测试
*/

int main(int argc, char* argv[])
{
	char ip[16];
	int lisFd, conFd, port;
	struct sockaddr_in servAddr;
	pid_t pid;
	if (argc != 3)
	{
		printf("\n Usage: %s <server ip> <server port>\n", argv[0]);
		return 1;
	}
	pid = getpid();
	//从命令行获取解析Ip port
	strcpy(ip, argv[1]);
	port = atoi(argv[2]);

	//2.启动服务
	//2.1创建server socket
	if ((lisFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server %d Error : Could not create socket!\n", pid);
		return -1;
	}
	//2.2设置端口重用
	int val = 1;
	if (setsockopt(lisFd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0) {
		printf("\n Server %d Error : setsockopt failed!\n", pid);
		return 0;
	}
	//2.3 bind
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(ip);
	servAddr.sin_port = htons(port);
	if (bind(lisFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
		printf("\n Server %d Error : Bind Failed! \n", pid);
		return 0;
	}
	//2.4启动监听
	if (listen(lisFd, 1024) < 0) {
		printf("\n Server %d Error : Listen Failed \n", pid);
		return 0;
	}
	else
	{
		printf("\n Start server on %s:%d successed, pid is %d\n", ip, port, pid);
	}
	//3 接收连接
	int i = 0;
	int* sockets = (int*)malloc(sizeof(int) * MAX_CONNECTION_NUM);

	while (1)
	{
		//默认情况下socket是blocking的，即函数accept,recv/send,connect等，都是等函数执行结束之后才能够返回
		conFd = accept(lisFd, (struct sockaddr*)NULL, NULL);
		if (conFd > 0)
		{
			sockets[i++] = conFd;
			printf("Server %s %d (%d) accept success:%d\n", ip, port, pid, i);

		}
	}
}