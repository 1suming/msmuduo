#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVER_ADDRESS "0.0.0.0"
#define SERVER_PORT 8000
#define WORKER_COUNT 4
#define MAXEVENTS 64


static int create_and_bind_socket() {
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr);
	server_address.sin_port = htons(SERVER_PORT);
	bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	return fd;
}

static int make_non_blocking_socket(int fd)
{
	int flags = fcntl(fd, F_GETFL);//获取文件flags
	if (flags == -1) {
		perror("fcntl error");
		return -1;
	}
	flags |= O_NONBLOCK;
	int s = fcntl(fd, F_SETFL, flags);
	if (s == -1) {
		perror("fcntl set error");
		return -1;
	}
	return 0;

}

int worker_process2(int listenfd, int epoll_fd, struct epoll_event* events, int k)
{
	while (1)
	{
		int n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		printf("Worker %d pid:%d get value from epoll_wait \n", k, getpid());
		int i;
		for (i = 0; i < n; i++)
		{
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN))
				)
			{
				printf("%d\n", i);
				fprintf(stderr, "epoll err\n");
				close(events[i].data.fd);
				continue;
			}
			else if (listenfd == events[i].data.fd)
			{
				struct sockaddr in_addr;
				socklen_t in_len;
				int in_fd;
				in_len = sizeof(in_addr);
				in_fd = accept(listenfd, &in_addr, &in_len);
				if (in_fd == -1) {
					printf("worker %d accept failed\n", k);
					break;
				}
				printf("worker %d accept success\n", k);
				close(in_fd);

			}
		}
	}
	return 0;
}
int worker_process(int listenfd, int epoll_fd, struct epoll_event* events,
	int k) {
	while (1) {
		int n;
		n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		printf("Worker %d pid is %d get value from epoll_wait\n", k, getpid());
		for (int i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN))) {
				printf("%d\n", i);
				fprintf(stderr, "epoll err\n");
				close(events[i].data.fd);
				continue;
			}
			else if (listenfd == events[i].data.fd) {
				struct sockaddr in_addr;
				socklen_t in_len;
				int in_fd;
				in_len = sizeof(in_addr);
				in_fd = accept(listenfd, &in_addr, &in_len);
				if (in_fd == -1) {
					printf("worker %d accept failed\n", k);
					break;
				}
				printf("worker %d accept success\n", k);
				close(in_fd);
			}
		}
	}

	return 0;
}
int main(int argc, char* argv[]) {

	int listen_fd, s;
	int epoll_fd;
	struct epoll_event event;
	struct epoll_event* events;
	listen_fd = create_and_bind_socket();
	if (listen_fd == -1) {
		abort();
	}
	s = make_non_blocking_socket(listen_fd);
	if (s == -1) {
		abort();
	}

	epoll_fd = epoll_create(MAXEVENTS);
	if (epoll_fd == -1) {
		printf("error epoll_create");
		abort();
	}
	event.data.fd = listen_fd;
	event.events = EPOLLIN;
	s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

	events = (struct epoll_event*)malloc(MAXEVENTS * sizeof(struct epoll_event));

	int i;
	for (i = 0; i < WORKER_COUNT; i++)
	{
		printf("create worker %d\n", i);
		int pid = fork();
		if (pid == 0) {
			worker_process(listen_fd, epoll_fd, events, i);
		}
	}
	int status;
	wait(&status);
	free(events);
	close(listen_fd);
	return 0;

}