/**
 *============================================================
 *  @file      pdumanip.hpp
 *  @brief     用于对数据包进行解包，或者把数据打包成数据包。pdu == Protocol Data Unit。
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_PDUMAINP_HPP_
#define LIBTAOMEEPP_PDUMAINP_HPP_

#include <cstring>
#include <ctype.h>


#include "byteswap.hpp"

namespace taomee {

/**
* @brief 把val转换字节序，并且打包到(uint8*)pkg + idx，然后增加idx的值，idx的增量为val变量占用的字节数。\n
*        比如val的类型是uint32，则idx的值增加4。
* @param pkg 用于保存数据包的内存块。
* @param val 需要打包到pkg里的值。
* @param idx 指示val应该被打包到pkg中偏移量为idx的内存中。
* @return void
*/
template <typename T>
inline void pack(void* pkg, T val, int& idx)
{
	*(reinterpret_cast<T*>(reinterpret_cast<uint8*>(pkg) + idx)) = bswap(val);
	idx += sizeof val;
}

/**
* @brief 把内存块val打包到(uint8*)pkg + idx，然后增加idx的值，idx的增量为len。
* @param pkg 用于保存数据包的内存块。
* @param val 需要打包到pkg里的内存块。
* @param len 内存块val的大小（byte）。
* @param idx 指示val应该被打包到pkg中偏移量为idx的内存中。
* @return void
*/
inline void pack(void* pkg, const void* val, std::size_t len, int& idx)
{
	memcpy(reinterpret_cast<uint8*>(pkg) + idx, val, len);
	idx += len;
}

template <typename T>
inline void pack2(void* pkg, T val, int& idx)
{
	*(reinterpret_cast<T*>(reinterpret_cast<uint8*>(pkg) + idx)) = val;
	idx += sizeof val;
}

inline void pack2(void* pkg, const void* val, std::size_t len, int& idx)
{
	memcpy(reinterpret_cast<uint8*>(pkg) + idx, val, len);
	idx += len;
}
/**
* @brief 不转换val的字节序，并且打包到(uint8*)pkg + idx，然后增加idx的值，idx的增量为val变量占用的字节数。\n
*        比如val的类型是uint32，则idx的值增加4。h == host。
* @param pkg 用于保存数据包的内存块。
* @param val 需要打包到pkg里的值。
* @param idx 指示val应该被打包到pkg中偏移量为idx的内存中。
* @return void
*/
template <typename T>
inline void pack_h(void* pkg, T val, int& idx)
{
	*(reinterpret_cast<T*>(reinterpret_cast<uint8*>(pkg) + idx)) = val;
	idx += sizeof val;
}

/**
 * @brief 将(uint8*)pkg + idx解包到val，并且转换字节序，然后增加idx的值，idx的增量为val变量占用的字节数。\n
 *        比如val的类型是uint32，则idx的值增加4。
 * @param pkg 需要解包的内存块。
 * @param val 解包后的数据保存到val里。
 * @param idx 指示应该把pkg中偏移量从idx开始的数据解包到val中。
 * @return void
 */
template <typename T>
inline void unpack(const void* pkg, T& val, int& idx)
{
	val = bswap(*(reinterpret_cast<const T*>(reinterpret_cast<const uint8*>(pkg) + idx)));
	idx += sizeof val;
}

/**
 * @brief 将(uint8*)pkg + idx解包到val，然后增加idx的值，idx的增量为len。
 * @param pkg 需要解包的内存块。
 * @param val 解包后的数据保存到内存块val里。
 * @param len 内存块val的大小（byte）。
 * @param idx 指示应该把pkg中偏移量从idx开始的数据解包到val中。
 * @return void
 */
inline void unpack(const void* pkg, void* val, std::size_t len, int& idx)
{
	memcpy(val, reinterpret_cast<const uint8*>(pkg) + idx, len);
	idx += len;
}
template <typename T>
inline void unpack2(const void* pkg, T& val, int& idx)
{
	val = (*(reinterpret_cast<const T*>(reinterpret_cast<const uint8*>(pkg) + idx)));
	idx += sizeof val;
}
inline void unpack2(const void* pkg, void* val, std::size_t len, int& idx)
{
	memcpy(val, reinterpret_cast<const uint8*>(pkg) + idx, len);
	idx += len;
}

/**
 * @brief 将(uint8*)pkg + idx解包到val，但不转换字节序，然后增加idx的值，idx的增量为val变量占用的字节数。\n
 *        比如val的类型是uint32，则idx的值增加4。
 * @param pkg 需要解包的内存块。
 * @param val 解包后的数据保存到val里。
 * @param idx 指示应该把pkg中偏移量从idx开始的数据解包到val中。
 * @return void
 */
template <typename T>
inline void unpack_h(const void* pkg, T& val, int& idx)
{
	val = *(reinterpret_cast<const T*>(reinterpret_cast<const uint8*>(pkg) + idx));
	idx += sizeof val;
}

} // namespace taomee

/**
* @brief 把字符first和second合并成十六进制数值。假设first=='A'，second=='B'，则char2hex的返回值为171，即十六进制的0xAB。
*
* @param first
* @param second
*
* @return first和second合并成的十六进制数值。
*/
static inline uint8
char2hex(char first, char second)
{
	first  = toupper(first);
	second = toupper(second);

	uint8 digit = (first >= 'A' ? (first - 'A') + 10 : (first - '0')) * 16;
	digit += (second >= 'A' ? (second - 'A') + 10 : (second - '0'));

	return digit;
}

/**
* @brief 把字符串形式的十六进制数srcstring转换成数值形式。假设srcstring=="ABCD"，
*        则((uint8*)desthex)[0]==0xAB，((uint8*)desthex)[1]==0xCD。
*
* @param srcstring 字符串形式的十六进制数。它的长度必须是偶数。
* @param srclen srcstring的长度，必须是偶数。
* @param desthex srcstring转换成数值形式后保存到desthex里。desthex指向的空间不能小于(srclen / 2)个字节。
*
* @return void
*/
static inline void
str2hex(const void* srcstring, size_t srclen, void* desthex)
{
	const char* src  = (const char*)srcstring;
	uint8* dest = (uint8*)desthex;

	size_t i;
	for (i = 0; i != srclen; i += 2) {
		dest[i / 2] = char2hex(src[i], src[i + 1]);
	}
}
#endif // LIBTAOMEEPP_PDUMAINP_HPP_
