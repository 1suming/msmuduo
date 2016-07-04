#ifndef _Buffer_h
#define _Buffer_h



#include"../base/Copyable.h"
#include"../base/StringPiece.h"
#include"Endian.h"


#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>
//#include <unistd.h>  // ssize_t


NS_BEGIN


///A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///@code
///+--------------------+----------------------+----------------------+
///|prependable bytes   |  readable bytes      |  writable bytes      |
///|                    |     (Content)        |                      |
///|                    |                      |                      |
///|--------------------+----------------------+----------------------+

///|                    |                      |                      |
///0     <=           readerIndex       <=  writeIndex     <=         size
///@endcode

class Buffer :public copyable
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initSize=kInitialSize):
	{

	}

private:

	vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];

};

NS_END
#endif