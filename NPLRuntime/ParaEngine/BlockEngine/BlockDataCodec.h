#pragma once
#include <stdint.h>

namespace ParaEngine
{
	class CIntegerEncoder
	{
	public:
		template<typename VALUE_TYPE>
		static bool TryDecodeUInt32(CParaFile& source, VALUE_TYPE& value, int& nByteRead)
		{
			int b = 0;
			if (source.read(&b, 1) == 0)
				return false;
			++nByteRead;
			if ((b & 0x80) == 0)
			{
				// single-unsigned char
				value = (VALUE_TYPE)b;
				return true;
			}

			int shift = 7;

			value = (VALUE_TYPE)(b & 0x7F);
			bool keepGoing;
			int i = 0;
			do
			{
				if (source.read(&b, 1) == 0)
					return false;
				++nByteRead;

				i++;
				keepGoing = (b & 0x80) != 0;
				value |= ((VALUE_TYPE)(b & 0x7F)) << shift;
				shift += 7;
			} while (keepGoing && i < 4);
			if (keepGoing && i == 4)
			{
				return false;
			}
			return true;
		}

		template<typename VALUE_TYPE>
		static int EncodeUInt32(VALUE_TYPE value, BYTE* buffer, StringBuilder& stream)
		{
			int count = 0, index = 0;
			do
			{
				buffer[index++] = (unsigned char)((value & 0x7F) | 0x80);
				value >>= 7;
				count++;
			} while (value != 0);
			buffer[index - 1] &= 0x7F;
			stream.append((const char*)buffer, count);
			return count;
		}

		/** Author: LiXizhi
		// here we will use skip 1 algorithm to compress the data, since the result is sorted integer.
		// More advanced one is PForDelta algorithm, which is discussed in part here
		// http://stackoverflow.com/questions/283299/best-compression-algorithm-for-a-sequence-of-integers
		// Example:
		// to compress: 1-100, 110-160:
		// "skip 1" (assume start at zero as it makes things easier), "take 100", "skip 9", "take 51"; subtract 1 from each, giving (as decimals)
		// result: 0,99,8,50
		*/
		template<typename VALUE_TYPE>
		static int EncodeSkipOne(StringBuilder& stream, const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount == 0) return 0;
			unsigned char buffer[10];
			int last = -1, len = 0;
			for (int i = 0; i < nCount;)
			{
				int gap = data[i] - 2 - last, size = 0;
				while (++i < nCount && data[i] == data[i - 1] + 1)
					size++;
				last = data[i - 1];
				len += EncodeUInt32((VALUE_TYPE)gap, buffer, stream)
					+ EncodeUInt32((VALUE_TYPE)size, buffer, stream);
			}
			return len;
		}

		/** return true, if more than half of the input is continuous integer with step 1. if true, we prefer using EncodeSkipOne.*/
		template<typename VALUE_TYPE>
		static bool IsSkipOneBetter(const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount < 2) return false;

			int nSkipCount = 0;
			int nHalfCount = nCount >> 1;
			for (int i = 1; i < nCount && nSkipCount <= nHalfCount; ++i)
			{
				if (data[i] == (data[i - 1] + 1))
					nSkipCount++;
			}
			return nSkipCount >= nHalfCount;
		}

		template<typename VALUE_TYPE>
		static void DecodeSkipOne(CParaFile& stream, std::vector<VALUE_TYPE>& list, int nStreamSize = 0xfffffff)
		{
			VALUE_TYPE skip, take;
			int last = 0;
			int nByteRead = 0;
			while (nByteRead<nStreamSize && TryDecodeUInt32(stream, skip, nByteRead)
				&& TryDecodeUInt32(stream, take, nByteRead))
			{
				last += (int)skip + 1;
				for (VALUE_TYPE i = 0; i <= take; i++) {
					list.push_back(last++);
				}
			}
		}

		template<typename VALUE_TYPE>
		static int EncodeIntDeltaArray(StringBuilder& stream, const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount == 0) return 0;
			int len = 0;
			unsigned char buffer[10];
			len += EncodeUInt32((VALUE_TYPE)(data[0]), buffer, stream);

			for (int i = 1; i < nCount; ++i)
			{
				len += EncodeUInt32((VALUE_TYPE)(data[i] - data[i - 1]), buffer, stream);
			}
			return len;
		}

		template<typename VALUE_TYPE>
		static void DecodeIntDeltaArray(CParaFile& stream, std::vector<VALUE_TYPE>& list, int nStreamSize = 0xfffffff)
		{
			VALUE_TYPE lastValue, delta;
			int last = 0;
			int nByteRead = 0;

			if (nByteRead<nStreamSize && TryDecodeUInt32(stream, lastValue, nByteRead))
			{
				list.push_back(lastValue);
				while (nByteRead<nStreamSize && TryDecodeUInt32(stream, delta, nByteRead))
				{
					lastValue += delta;
					list.push_back(lastValue);
				}
			}
		}

		/** Author: LiXizhi
		if the input array contains, many data of the same value.
		1,1,1,2,2,2,2,3,3,3,3 is saved as 1,3,2,4,3,4 (value, count, value, count, ... value)
		*/
		template<typename VALUE_TYPE>
		static int EncodeSameInteger(StringBuilder& stream, const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount == 0) return 0;
			unsigned char buffer[10];
			int len = 0;
			int last = data[0];
			uint32_t size = 0;
			len += EncodeUInt32(last, buffer, stream);

			for (int i = 0; i < nCount;)
			{
				size = 0;

				VALUE_TYPE cur_last = last;
				while (++i < nCount && (cur_last = data[i]) == last)
					size++;

				len += EncodeUInt32(size, buffer, stream);
				if (last != cur_last)
				{
					last = cur_last;
					len += EncodeUInt32(last, buffer, stream);
				}
			}
			return len;
		}

		template<typename VALUE_TYPE>
		static void DecodeSameInteger(CParaFile& stream, std::vector<VALUE_TYPE>& list, int nStreamSize = 0xfffffff)
		{
			VALUE_TYPE last;
			uint32_t size = 0;
			int nByteRead = 0;
			while (nByteRead<nStreamSize && TryDecodeUInt32(stream, last, nByteRead))
			{
				list.push_back(last);
				size = 0;
				if (nByteRead<nStreamSize && TryDecodeUInt32(stream, size, nByteRead))
				{
					for (uint32_t i = 0; i < size; i++) {
						list.push_back(last);
					}
				}
			}
		}

		template<typename VALUE_TYPE>
		static bool IsSameIntegerBetter(const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount == 0)
				return false;
			VALUE_TYPE last = data[0];
			int len = 0;
			uint32_t size = 0;
			for (int i = 0; i < nCount && len<nCount;)
			{
				size = 0;
				VALUE_TYPE cur_last = last;
				while (++i < nCount && (cur_last = data[i]) == last)
					size++;
				last = cur_last;
				len += 2;
			}
			return len<nCount;
		}

		template<typename VALUE_TYPE>
		static int EncodeIntArray(StringBuilder& stream, const std::vector<VALUE_TYPE>& data)
		{
			int nCount = data.size();
			if (nCount == 0) return 0;
			int len = 0;
			unsigned char buffer[10];
			for (int i = 0; i < nCount; ++i)
			{
				len += EncodeUInt32(data[i], buffer, stream);
			}
			return len;
		}

		template<typename VALUE_TYPE>
		static void DecodeIntArray(CParaFile& stream, std::vector<VALUE_TYPE>& list, int nStreamSize = 0xfffffff)
		{
			VALUE_TYPE value;
			int last = 0;
			int nByteRead = 0;
			while (nByteRead<nStreamSize && TryDecodeUInt32(stream, value, nByteRead))
			{
				list.push_back(value);
			}
		}
	};

	/** Author: LiXizhi
		if the input array contains, many data of the same value.
		1,1,1,2,2,2,2,3,3,3,3 is saved as 1,3,2,4,3,4 (value, count, value, count, ... value)
	*/
	template<typename VALUE_TYPE>
	class CSameIntegerEncoder
	{
	public:
		CSameIntegerEncoder(StringBuilder* pStream) :m_pStream(pStream), m_bIsFirst(true), m_nLastValueCount(0), m_nLastValue(0){};

		void Reset(){
			m_bIsFirst = true;
			m_nLastValue = 0;
			m_nLastValueCount = 0;
		}
		void Append(VALUE_TYPE nValue)
		{
			if (!m_bIsFirst)
			{
				if (m_nLastValue == nValue)
				{
					m_nLastValueCount++;
				}
				else
				{
					Finalize();
					m_nLastValue = nValue;
					m_nLastValueCount = 1;
				}
			}
			else
			{
				m_nLastValue = nValue;
				m_nLastValueCount = 1;
				m_bIsFirst = false;
			}
		}

		/* @param nCount: must be larger than 1*/
		void Append(VALUE_TYPE nValue, int nCount)
		{
			assert(nCount >= 1);
			if (!m_bIsFirst)
			{
				if (m_nLastValue == nValue)
				{
					m_nLastValueCount += nCount;
				}
				else
				{
					Finalize();
					m_nLastValue = nValue;
					m_nLastValueCount = nCount;
				}
			}
			else
			{
				m_nLastValue = nValue;
				m_nLastValueCount = nCount;
				m_bIsFirst = false;
			}
		}

		/** call this function when last element is added. */
		void Finalize()
		{
			if (m_nLastValueCount > 0)
			{
				unsigned char buffer[10];
				CIntegerEncoder::EncodeUInt32(m_nLastValue, buffer, *m_pStream);
				CIntegerEncoder::EncodeUInt32(m_nLastValueCount - 1, buffer, *m_pStream);
			}
			else
			{
				OUTPUT_LOG("Error: invalid call to finalize with nothing to finalize\n");
				assert(false);
			}
		}
	public:
		StringBuilder* m_pStream;
		int m_nLastValueCount;
		VALUE_TYPE m_nLastValue;
		bool m_bIsFirst;
	};

	template<typename VALUE_TYPE>
	class CSameIntegerDecoder
	{
	public:
		static void DecodeSameIntegerOfCount(CParaFile& stream, std::vector<VALUE_TYPE>& list, int nIntegerCount = 0xfffffff)
		{
			VALUE_TYPE last;
			uint32_t size = 0;
			int nIntegerRead = 0;
			int nByteRead = 0;
			while (nIntegerRead < nIntegerCount && CIntegerEncoder::TryDecodeUInt32(stream, last, nByteRead))
			{
				list.push_back(last);
				size = 0;
				if (CIntegerEncoder::TryDecodeUInt32(stream, size, nByteRead))
				{
					for (uint32_t i = 0; i < size; i++) {
						list.push_back(last);
					}
				}
				nIntegerRead += (size+1);
			}
		}
		static void SkipDecodeSameIntegerOfCount(CParaFile& stream, int nIntegerCount = 0xfffffff)
		{
			VALUE_TYPE last;
			uint32_t size = 0;
			int nIntegerRead = 0;
			int nByteRead = 0;
			while (nIntegerRead < nIntegerCount && CIntegerEncoder::TryDecodeUInt32(stream, last, nByteRead))
			{
				size = 0;
				CIntegerEncoder::TryDecodeUInt32(stream, size, nByteRead);
				nIntegerRead += (size + 1);
			}
		}
	};
}