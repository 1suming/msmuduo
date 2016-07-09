
#include<iostream>
using namespace std;

#define BUFF_SIZE 50
int main()
{
#ifdef WIN32 
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		cout << "WSAStartup failed";
		exit(0);
	}

}