#ifndef _LogStream_h
#define _LogStream_h
#include <boost/noncopyable.hpp>
#include"StringPiece.h"

NS_BEGIN

namespace detail
{
	const int kSmallBuffer = 4000;
	const int kLargeBuffer = 4000 * 1000;

	template<int SIZE>
	class FixedBuffer : boost::noncopyable
	{
	public:
		FixedBuffer() :cur_(data_)
		{
			setCookie(cookieStart);
		}
		~FixedBuffer()
		{
			setCookie(cookieEnd);

		}
		void append(const char* /*restrict*/ buf, size_t len)
		{
			//FIXME: append partially
			if (implicit_cast<size_t>(avail()) > len)
			{
				memcpy(cur_, buf, len);
				cur_ += len;

			}
		}

		const char* data() const { return data_; }
		int length() const { return static_cast<int>(cur_ - data_); }

		// write to data_ directly
		char* current() { return cur_; }
		int avail() const { return static_cast<int>(end() - cur_); }
		void add(size_t len) { cur_ += len; }

		void reset() { cur_ = data_; }
		void bzeroinit() { bzero(data_, sizeof data_); }

		// for used by GDB
		const char* debugString();
		void setCookie(void(*cookie)()) { cookie_ = cookie; }
		// for used by unit test
		string toString() const { return string(data_, length()); }
		StringPiece toStringPiece() const { return StringPiece(data_, length()); }

		



	private:

		const char* end() const { return data_ + sizeof data_; }
		//must be outline function for cookies
		static void cookieStart();
		static void cookieEnd();


		void(*cookie_)();


		char data_[SIZE];
		char *cur_;


	};
}//end namespace detail

class LogStream : boost::noncopyable
{
	typedef LogStream self;

public:
	typedef detail::FixedBuffer<detail::kSmallBuffer>  Buffer;

	self& operator<<(bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;

	}
	
	self& operator<<(char v)
	{
		buffer_.append(&v, 1);
		return *this;
	}

	// self& operator<<(signed char);
	// self& operator<<(unsigned char);


	self& operator<<(int16_t);
	self& operator<<(uint16_t);
	self& operator<<(int32_t);
	self& operator<<(uint32_t);
	self& operator<<(int64_t);
	self& operator<<(uint64_t);

	//pointer
	self& operator<<(const void*);

	self& operator<< (float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}
	// FIXME: replace this with Grisu3 by Florian Loitsch.
	self& operator<<(double v)
	{
		if (buffer_.avail() >= kMaxNumericSize)
		{
			int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v); //%g根据数值不同自动选择%f或者%e。%e格式在指数小于 - 4或者大于等于精度时使用
			buffer_.add(len);

		}
		return *this;

	}

	self& operator<<(const char* str)
	{
		if (str)
		{
			buffer_.append(str, strlen(str));
		}
		else
		{
			buffer_.append("(null)", 6);

		}
		return *this;
	}
	self& operator<<(const unsigned char* str)
	{
		return operator<<(reinterpret_cast<const char*>(str));
	}

	self& operator<<(const string& v)
	{
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

 

	self& operator<<(const StringPiece& v)
	{
		buffer_.append(v.data(), v.size());
		return *this;
	}

	self& operator<<(const Buffer& v)
	{
		*this << v.toStringPiece();
		return *this;
	}

	void append(const char* data, int len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void resetBuffer() { buffer_.reset(); }


 
	 
private:

	void staticCheck();

	template<typename T>
	void formatInteger(T);

	Buffer buffer_;

	static const int kMaxNumericSize = 32;

};

class Fmt // : boost::noncopyable
{
public:
	template<typename T>
	Fmt(const char* fmt, T val);

	const char* data() const { return buf_; }
	int length() const { return length_; }

private:
	char buf_[32];
	int length_;
};


inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
	s.append(fmt.data(), fmt.length());
	return s;

}
 

NS_END


#endif