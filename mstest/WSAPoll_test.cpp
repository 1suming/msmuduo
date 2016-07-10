#include"globalsock.h"
#include<iostream>
#include<vector>
using namespace std;

#define BUFF_SIZE 50

#define PORT 9981

void pollTest()
{
	vector<struct pollfd> pollfds_;

	struct pollfd pollfd1;
	int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		cout << "socket::createNonblockingOrDie";
	}
	int listenfd = createServerSocket(PORT);
	pollfd1.fd = listenfd;
	pollfd1.events = POLLIN ;

	pollfds_.push_back(pollfd1);
	/*
	msdn WSAPOLL:https://msdn.microsoft.com/en-us/library/ms741669(VS.85).aspx
	Flag	Description
	POLLPRI	Priority data may be read without blocking. This flag is not supported by the Microsoft Winsock provider.
	POLLRDBAND	Priority band (out-of-band) data may be read without blocking.
	POLLRDNORM	Normal data may be read without blocking.
	POLLWRNORM	Normal data may be written without blocking.

	*/
	while (1){
		int numEvents = ::WSAPoll(&*pollfds_.begin(), pollfds_.size(), -1);//-1��ʾ��Զ�ȴ�
		//int numEvents = ::WSAPoll(&pollfd1, 1, -1);
		if (numEvents > 0)
		{
			for (int i = 0; i < pollfds_.size() && numEvents>0; i++)
			{
				int currFd = pollfds_[i].fd;
				if (pollfds_[i].revents > 0)
				{
					numEvents--;//ÿ���ҵ�������sock���ȥ1��for��numEvent>0���������õ��ж�

					int revents_ = pollfds_[i].revents;

					if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
					{
						cout << "POLLHUP" << endl;

					}
					if (revents_ & POLLNVAL)
					{
						cout << " Channel::handleEvent() POLLNVAL" << endl;
					}
					if (revents_ & (POLLERR | POLLNVAL))
					{
						cout << "POLLERR" << endl;
					}
					//if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
					if (revents_ & (POLLIN | POLLPRI))
					{
						cout << "POLLIN" << endl;

						//�����µ�����
						if (currFd == listenfd)
						{
							cout << "new connection\n";

							struct sockaddr_in peerAddr;
							int addrLen = sizeof(peerAddr);
							int connfd = ::accept(listenfd, (struct sockaddr*)&peerAddr, &addrLen);
							cout << "connfd:" << connfd << endl;

							struct pollfd pollfdNew;
							pollfdNew.fd = connfd;
							pollfdNew.events = POLLIN | POLLOUT;
							pollfdNew.revents = 0;

							pollfds_.push_back(pollfdNew);


						}


					}
					if (revents_ & POLLOUT)
					{
						//��������һֱ��עPOLLOUT����һֱ����busy loop,Ҳ���ǲ�ͣ�����
						cout << "POLLOUT fd:" << currFd << endl;

						//if (currFd != listenfd)//�������Ҳ�У�listenfd������POLLOUT�¼�
						{
							char buf[] = "hello,I'm server";
							int ret = send(currFd, buf, strlen(buf), 0);
							if (ret < 0)
							{
								cout << "send failed!!!" << endl;
							}
						}


					}



				}

			}
		}
		else if (numEvents == 0)
		{

		}
		else
		{
			int err = WSAGetLastError();
			cout << "error:" << err << endl;
			/*
			��WSAPoll
			���Ǳ�10022����
			WSAEINVAL                           (10022)             Invalid argument.
			�ṩ�˷Ƿ����������磬��ʹ��setsockopt()����ʱָ���˷Ƿ���level������һЩʵ���У���Ҳ�������׽��ֵĵ�ǰ״̬��أ����磬���׽���û��ʹ��listen()ʹ�䴦�ڼ���ʱ����accept()������

			
			����ҵ�ԭ���ˣ�listenfd��Ӧ����pollfd1.events = POLLIN  | POLLPRI ,��ΪPOLLPRI windows��֧��.

			*/

		}
	}
}
int main()
{
#ifdef WIN32 
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		cout << "WSAStartup failed";
		exit(0);
	}
#endif
	/*
	int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		cout<< "socket::createNonblockingOrDie";
	}
	cout << "sockfd" << sockfd << endl;
	*/
	 
	pollTest();



 

}