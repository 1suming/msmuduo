﻿#ifndef _Timestamp_h
#define _Timestamp_h

#include"copyable.h"
#include <boost/operators.hpp>
 
NS_BEGIN


class Timestamp : public copyable,
	public boost::less_than_comparable<Timestamp>
{
public:
	Timestamp()
		:microSecondsSinceEpoch_(0)
	{

	}
	///
	/// Constucts a Timestamp at specific time
	///
	/// @param microSecondsSinceEpoch
	explicit Timestamp(int64_t microSecondsSinceEpochArg)
		: microSecondsSinceEpoch_(microSecondsSinceEpochArg)
	{
	}

	void swap(Timestamp& that)
	{
		std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
	}

	// default copy/assignment/dtor are Okay

	string toString() const;
	string toFormattedString(bool showMicroseconds = true) const;

	bool valid() const { return microSecondsSinceEpoch_ > 0; }

	// for internal usage.
	int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
	time_t secondsSinceEpoch() const
	{
		return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	}

	///
	/// Get time of now.
	///
	static Timestamp now();
	static Timestamp invalid()
	{
		return Timestamp();
	}

	static Timestamp fromUnixTime(time_t t)
	{
		return fromUnixTime(t, 0);
	}

	static Timestamp fromUnixTime(time_t t, int microseconds)
	{
		return Timestamp(static_cast<int64_t>(t)* kMicroSecondsPerSecond + microseconds);
	}

	static const int kMicroSecondsPerSecond = 1000 * 1000; //microseconds:微秒 ，milliseconds:毫秒

private:
	int64_t microSecondsSinceEpoch_; //Epoch，新纪元 GMT的1970-01-01 00:00:00 +0000 (UTC)。

};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
	int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
	int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}




NS_END
#endif