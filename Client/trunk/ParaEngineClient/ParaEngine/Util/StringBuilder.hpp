//-----------------------------------------------------------------------------
// Class:	StringBuilderT
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.6.16
// Desc: 
// Portions of the code are:
// Copyright (C) 2005-2007 Feeling Software Inc.
// Copyright (C) 2005-2007 Sony Computer Entertainment America
//
// MIT License: http://www.opensource.org/licenses/mit-license.php
//-----------------------------------------------------------------------------
#include "math/ParaMath.h"

#include <limits>

#ifdef WIN32
#include <float.h>
#endif

#ifdef WIN32
#define ecvt _ecvt
#endif // WIN32
#ifdef ANDROID
#include <math.h>
#include <stdlib.h>

#define	NDIG	80

static char * cvt(double arg, int ndigits, int* decpt, int *sign, int eflag)
{
	int r2;
	double fi, fj;
	register char *p, *p1;
	static char buf[NDIG];

	if (ndigits < 0)
		ndigits = 0;
	if (ndigits >= NDIG - 1)
		ndigits = NDIG - 2;
	r2 = 0;
	*sign = 0;
	p = &buf[0];
	if (arg < 0) {
		*sign = 1;
		arg = -arg;
	}
	arg = modf(arg, &fi);
	p1 = &buf[NDIG];
	/*
	* Do integer part
	*/
	if (fi != 0) {
		p1 = &buf[NDIG];
		while (fi != 0) {
			fj = modf(fi / 10, &fi);
			*--p1 = (int)((fj + .03) * 10) + '0';
			r2++;
		}
		while (p1 < &buf[NDIG])
			*p++ = *p1++;
	}
	else if (arg > 0) {
		while ((fj = arg * 10) < 1) {
			arg = fj;
			r2--;
		}
	}
	p1 = &buf[ndigits];
	if (eflag == 0)
		p1 += r2;
	*decpt = r2;
	if (p1 < &buf[0]) {
		buf[0] = '\0';
		return(buf);
	}
	while (p <= p1 && p < &buf[NDIG]) {
		arg *= 10;
		arg = modf(arg, &fj);
		*p++ = (int)fj + '0';
	}
	if (p1 >= &buf[NDIG]) {
		buf[NDIG - 1] = '\0';
		return(buf);
	}
	p = p1;
	*p1 += 5;
	while (*p1 > '9') {
		*p1 = '0';
		if (p1 > buf)
			++*--p1;
		else {
			*p1 = '1';
			(*decpt)++;
			if (eflag == 0) {
				if (p > buf)
					*p = '0';
				p++;
			}
		}
	}
	*p = '\0';
	return(buf);
}

char * ecvt(double arg, int ndigits, int *decpt, int *sign)
{
	return cvt(arg, ndigits, decpt, sign, 1);
}

char * fcvt(double arg, int ndigits, int*decpt, int * sign)
{
	return cvt(arg, ndigits, decpt, sign, 0);
}
#endif
namespace ParaEngine
{

	template <class Char, class FloatType>
	void FloatToString(FloatType f, Char* sz)
	{
		Char* sBuffer = sz + 1;
		static const int digitCount = 6;
		int decimal, sign;

		// ecvt rounds the string for us: http://www.datafocus.com/docs/man3/ecvt.3.asp
		char* end = ecvt(f, digitCount, &decimal, &sign);

		if (sign != 0) (*sBuffer++) = '-';
		int count = digitCount;
		if (decimal > digitCount)
		{
			// We use the scientific notation: P.MeX
			(*sBuffer++) = (*end++); // P is one character.
			(*sBuffer++) = '.';

			// Mantissa (cleaned for zeroes)
			for (--count; count > 0; --count) if (end[count - 1] != '0') break;
			for (int i = 0; i < count; ++i) (*sBuffer++) = (*end++);
			if (sBuffer[-1] == '.') --sBuffer;

			// Exponent
			(*sBuffer++) = 'e';
			uint32 exponent = decimal - 1; // X
			if (exponent >= 10) (*sBuffer++) = (Char)('0' + (exponent / 10));
			(*sBuffer++) = (Char)('0' + (exponent % 10));
			(*sBuffer) = 0;
			return;
		}
		else if (decimal > 0)
		{
			// Simple number: A.B
			for (int i = 0; i < decimal; ++i) (*sBuffer++) = (*end++);
			if (decimal < digitCount) (*sBuffer++) = '.';
			count = digitCount - decimal;
		}
		else if (decimal < -digitCount)
		{
			// What case is this?
			decimal = count = 0;
		}
		else if (decimal < 0 || (decimal == 0 && *end != '0'))
		{
			// Tiny number: 0.Me-X
			(*sBuffer++) = '0'; (*sBuffer++) = '.';
			for (int i = 0; i < -decimal; ++i) (*sBuffer++) = '0';
			count = digitCount + decimal;
		}
		for (; count > 0; --count) if (end[count - 1] != '0') break;
		for (int i = 0; i < count; ++i) (*sBuffer++) = (*end++);
		if (decimal == 0 && count == 0) (*sBuffer++) = '0';
		if (sBuffer[-1] == '.') --sBuffer;
		(*sBuffer) = 0;
	}


	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::StringBuilderT(const string_type& sz)
	{
		this->m_buffer = NULL;
		this->m_size = 0;
		this->m_reserved = 0;

		append(sz.c_str(), sz.size());
	}

	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::StringBuilderT(const Char* sz)
	{
		this->m_buffer = NULL;
		this->m_size = 0;
		this->m_reserved = 0;

		size_t len = 0;
		for (const Char* p = sz; *p != 0; ++p) ++len;
		append(sz, len);
	}

	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::StringBuilderT(Char ch, size_t count)
	{
		this->m_buffer = NULL;
		this->m_size = 0;
		this->m_reserved = 0;

		reserve(count);
		for (size_t i = 0; i < count; ++i) m_buffer[m_size++] = ch;
	}

	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::StringBuilderT(size_t reservation)
	{
		this->m_buffer = NULL;
		this->m_size = 0;
		this->m_reserved = 0;
		reserve(reservation);
	}

	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::StringBuilderT()
	{
		this->m_buffer = NULL;
		this->m_size = 0;
		this->m_reserved = 0;
	}

	template <typename UserAllocator>
	StringBuilderT<UserAllocator>::~StringBuilderT()
	{
		reserve(0);
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::enlarge(size_t minimum)
	{
		reserve(max(m_reserved + minimum, 2 * m_reserved));
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::clear()
	{
		m_size = 0;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::resize(size_t new_size)
	{
		if (new_size > m_reserved)
		{
			reserve(new_size);
		}
		m_size = new_size;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(Char c)
	{
		if (m_size + 1 >= m_reserved) enlarge(2);

		m_buffer[m_size++] = c;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(const string_type& sz) { append(sz.c_str()); }
	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(const Char* sz)
	{
		if (sz == NULL) return;

		// This is optimized for SMALL strings.
		for (; *sz != 0; ++sz)
		{
			if (m_size >= m_reserved) enlarge(32);
			m_buffer[m_size++] = *sz;
		}
	}
	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(const Char* sz, size_t len)
	{
		if (sz == NULL) return;

		if ((m_size + len) >= m_reserved)
		{
			enlarge(max((size_t)32, m_size + len + 1));
		}
		memcpy(m_buffer + m_size, sz, len);
		m_size += len;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::WriteAt(int nIndex, const Char* sz, size_t len)
	{
		if (sz == NULL || len == 0) return;
		
		if ((len + nIndex) > m_size)
		{
			if ((len + nIndex) >= m_reserved)
			{
				enlarge(max((size_t)32, nIndex + len + 1));
			}
			m_size = nIndex + len;
		}
		memcpy(m_buffer + nIndex, sz, len);
	}


	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(const StringBuilderT& b)
	{
		if (m_size + b.m_size >= m_reserved) enlarge(32 + m_size + b.m_size - m_reserved);
		memcpy(m_buffer + m_size, b.m_buffer, b.m_size * sizeof(Char));
		m_size += b.m_size;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(float f)
	{
#ifdef WIN32
		// use <float.h> _isnan method to detect the 1.#IND00 NaN.
		if (f != std::numeric_limits<float>::infinity() && f != -std::numeric_limits<float>::infinity() && f != std::numeric_limits<float>::quiet_NaN() && f != std::numeric_limits<float>::signaling_NaN() && !_isnan((double)f))
#else
		if (f != std::numeric_limits<float>::infinity() && f != -std::numeric_limits<float>::infinity() && f != std::numeric_limits<float>::quiet_NaN() && f != std::numeric_limits<float>::signaling_NaN())
#endif
		{
			if (Math::IsEquivalent(f, 0.0f, std::numeric_limits<float>::epsilon())) append((Char)'0');
			else
			{
				Char sz[128];
				FloatToString(f, sz);
				append(sz + 1);
			}
		}
		else if (f == std::numeric_limits<float>::infinity())
		{
			append((Char)'I'); append((Char)'N'); append((Char)'F');
		}
		else if (f == -std::numeric_limits<float>::infinity())
		{
			append((Char)'-'); append((Char)'I'); append((Char)'N'); append((Char)'F');
		}
		else
		{
			append((Char)'N'); append((Char)'a'); append((Char)'N');
		}
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(double f)
	{
#ifdef WIN32
		// use <float.h> _isnan method to detect the .#IND00 NaN.
		if (f != std::numeric_limits<float>::infinity() && f != -std::numeric_limits<float>::infinity() && f != std::numeric_limits<float>::quiet_NaN() && f != std::numeric_limits<float>::signaling_NaN() && !_isnan(f))
#else
		if (f != std::numeric_limits<float>::infinity() && f != -std::numeric_limits<float>::infinity() && f != std::numeric_limits<float>::quiet_NaN() && f != std::numeric_limits<float>::signaling_NaN())
#endif
		{
			if (Math::IsEquivalent(f, 0.0, std::numeric_limits<double>::epsilon())) append((Char)'0');
			else
			{
				Char sz[128];
				FloatToString(f, sz);
				append(sz + 1);
			}
		}
		else if (f == std::numeric_limits<double>::infinity())
		{
			append((Char)'I'); append((Char)'N'); append((Char)'F');
		}
		else if (f == -std::numeric_limits<double>::infinity())
		{
			append((Char)'-'); append((Char)'I'); append((Char)'N'); append((Char)'F');
		}
		else
		{
			append((Char)'N'); append((Char)'a'); append((Char)'N');
		}
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::appendLine(const Char* sz)
	{
		append(sz);
		append((Char)'\n');
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::appendHex(uint8 i)
	{
		uint8 top = (i & 0xF0) >> 4;
		uint8 bot = i & 0xF;
		if (top <= 0x9) append((Char)('0' + top));
		else append((Char)('A' + (top - 0xA)));
		if (bot <= 0x9) append((Char)('0' + bot));
		else append((Char)('A' + (bot - 0xA)));
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::remove(int32 start)
	{
		if ((int32)m_size > start && start >= 0) m_size = start;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::remove(int32 start, int32 end)
	{
		int32 diff = end - start;
		if ((int32)m_size >= end && start >= 0 && diff > 0)
		{
			const Char* stop = m_buffer + m_size - diff;
			for (Char* p = m_buffer + start; p != stop; ++p)
			{
				*p = *(p + diff);
			}
			m_size -= diff;
		}
	}

	template <typename UserAllocator>
	const typename StringBuilderT<UserAllocator>::Char* StringBuilderT<UserAllocator>::c_str() const
	{
		StringBuilderT<UserAllocator>* ncThis = const_cast< StringBuilderT<UserAllocator>* >(this);
		if (m_size + 1 > m_reserved) ncThis->enlarge(1);
		ncThis->m_buffer[m_size] = 0;
		return m_buffer;
	}

	template <typename UserAllocator>
	int32 StringBuilderT<UserAllocator>::index(Char c) const
	{
		if (m_buffer != NULL && m_size > 0)
		{
			const Char* end = m_buffer + m_size + 1;
			for (const Char* p = m_buffer; p != end; ++p)
			{
				if (*p == c) return (int32)(p - m_buffer);
			}
		}
		return -1;
	}

	template <typename UserAllocator>
	int32 StringBuilderT<UserAllocator>::rindex(Char c) const
	{
		if (m_buffer != NULL && m_size > 0)
		{
			for (const Char* p = m_buffer + m_size - 1; p != m_buffer; --p)
			{
				if (*p == c) return (int32)(p - m_buffer);
			}
		}
		return -1;
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(uint32 i)
	{
		char sz[128];
		snprintf(sz, 128, "%u", (unsigned int)i);
		append(sz);
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(uint64 i)
	{
		char sz[128];
		snprintf(sz, 128, "%u", (unsigned int)i);
		append(sz);
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::append(int32 i)
	{
		char sz[128];
		snprintf(sz, 128, "%i", (int)i);
		append(sz);
	}

	template <typename UserAllocator>
	void StringBuilderT<UserAllocator>::reserve(size_t _length)
	{
		assert(m_size <= m_reserved);
		if (_length == 0)
		{
			if (m_buffer != 0)
			{
				user_allocator::deallocate(m_buffer, m_reserved);
				m_buffer = 0;
			}
			m_size = m_reserved = 0;
		}
		else if (_length < m_reserved)
		{
			if (m_size>_length)
				m_size = _length;
		}
		else if (_length > m_reserved)
		{
			// rounded to exponent of 2, for buffer smaller than 2048
			int n = max(5, Math::log2_ceil((int)_length)); // the smallest buffer is 32 bytes
			if (n<11)
				_length = (size_t)(0x1 << n);
			else
			{
				// always round to multiple of 2 even for size bigger than 2048
				_length = (size_t)(0x1 << n);
			}

			Char* b = static_cast<Char*>(user_allocator::allocate(_length));
			memcpy(b, m_buffer, m_size * sizeof(Char));
			if (m_buffer != 0)
			{
				user_allocator::deallocate(m_buffer, m_reserved);
			}
			m_buffer = b;
			m_reserved = _length;
		}
	}
} // ParaEngine

