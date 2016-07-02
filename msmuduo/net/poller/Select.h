#ifndef _select_h
#define _select_h

NS_BEGIN

/*
WSAAsyncSelect function
https://msdn.microsoft.com/en-us/library/ms741540(VS.85).aspx

int WSAAsyncSelect(
_In_ SOCKET       s, //A descriptor that identifies the socket for which event notification is required.
_In_ HWND         hWnd, //A handle that identifies the window that will receive a message when a network event occurs.
_In_ unsigned int wMsg,//A message to be received when a network event occurs.
_In_ long         lEvent //A bitmask that specifies a combination of network events in which the application is interested.

);

*/
class Select
{
 
};




 

NS_END
#endif