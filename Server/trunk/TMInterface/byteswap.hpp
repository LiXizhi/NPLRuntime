/**
 *============================================================
 *  @file      byteswap.hpp
 *  @brief     用于进行字节序转换。如果原本是网络序，则转成本机序；如果原本是本机序，则转成网络序。\n
 *             用法：val = bswap(val);。可以对任意大小的整数类型进行字节序转换。
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */
#pragma once
#ifndef WIN32
extern "C" {
#include <stdint.h>   // C99
#include <byteswap.h> // Linux
}

namespace taomee {

inline int8 bswap(int8 x)
{
	return x;
}

inline uint8 bswap(uint8 x)
{
	return x;
}

inline int16 bswap(int16 x)
{
	return bswap_16(x);
}

inline uint16 bswap(uint16 x)
{
	return bswap_16(x);
}

inline int32 bswap(int32 x)
{
	return bswap_32(x);
}

inline uint32 bswap(uint32 x)
{
	return bswap_32(x);
}

inline int64 bswap(int64 x)
{
	return bswap_64(x);
}

inline uint64 bswap(uint64 x)
{
	return bswap_64(x);
}

#if __WORDSIZE == 32
inline long bswap(long x)
{
	return bswap_32(x);
}

inline unsigned long bswap(unsigned long x)
{
	return bswap_32(x);
}
#endif
}
#else
namespace taomee {

	inline int8 bswap(int8 x)
	{
		return x;
	}

	inline uint8 bswap(uint8 x)
	{
		return x;
	}

	inline int16 bswap(int16 x)
	{
		return 0;
	}

	inline uint16 bswap(uint16 x)
	{
		return 0;
	}

	inline int32 bswap(int32 x)
	{
		return 0;
	}

	inline uint32 bswap(uint32 x)
	{
		return 0;
	}

	inline int64 bswap(int64 x)
	{
		return 0;
	}

	inline uint64 bswap(uint64 x)
	{
		return 0;
	}

	

} // namespace taomee

#endif
