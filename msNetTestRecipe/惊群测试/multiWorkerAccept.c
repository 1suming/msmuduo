#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define WORKER_COUNT 4

int worker_process(int listenfd, int i)
{
	while (1)
	{
		printf("I am work %d, my pid is %d, begin to accept connections \n", i,
			getpid());

		struct sockaddr_in client_info;
		socklen_t client_info_len = sizeof(struct sockaddr_in);
		int connfd = accept(listenfd, (struct sockaddr*)&client_info, &client_info_len);
		if (connfd != -1)
		{
			printf("worker %d accept success\n", i);
			printf("ip :%s \t", inet_ntoa(client_info.sin_addr));
			printf("port:%d\n", client_info.sin_port);

		}
		else
		{
			printf("worker %d accept failed", i);
		}
		close(connfd);
	}
}
int main(int argc,char* argv[]) {
	
	struct sockaddr_in address;
	bzero(&address, sizeof(address));

 	char ip[16];
	short port;

	if (argc != 3)
	{
		printf("\n Usage: %s <server ip> <server port>\n", argv[0]);
		
		strcpy(ip, "0.0.0.0");
		port = 6000;
	}
	else
	{
		strcpy(ip, argv[1]);
		port = atoi(argv[2]);

	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	ret = listen(listenfd, 5);
	int i;
	for ( i = 0; i < WORKER_COUNT; i++)
	{
		printf("Create Worker %d\n", i);
		pid_t pid = fork();
		//child process
		if (pid == 0)
		{
			worker_process(listenfd, i);
		}
		else if (pid < 0) {
			printf("fork error");
		}
	}
	//wait child proces
	int status;
	wait(&status);
	return 0;

}