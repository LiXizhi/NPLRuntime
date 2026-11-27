#pragma once

#include "IParaAudioEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ParaEngine
{
	/**
	 * Stateful streaming converter from any ParaAudioFormats + hardware capture rate
	 * (e.g. 16000/22050/32000/44100/48000 Hz, 8/16-bit, mono/stereo) into normalized
	 * mono 16-bit signed little-endian PCM (PCM16LE) with continuous fractional phase
	 * across arbitrary chunk boundaries.
	 */
	class AudioCaptureResampler
	{
	public:
		void Reset()
		{
			m_remainderBytes.clear();
			m_monoSamples.clear();
			m_srcPosNum = 0;
		}

		static size_t BytesPerFrame(ParaAudioFormats format)
		{
			switch (format)
			{
			case EAF_8BIT_MONO:
				return 1U;
			case EAF_8BIT_STEREO:
				return 2U;
			case EAF_16BIT_MONO:
				return 2U;
			case EAF_16BIT_STEREO:
				return 4U;
			default:
				return 2U;
			}
		}

		static int ChannelsForFormat(ParaAudioFormats format)
		{
			return (format == EAF_8BIT_STEREO || format == EAF_16BIT_STEREO) ? 2 : 1;
		}

		static int BitsPerSampleForFormat(ParaAudioFormats format)
		{
			return (format == EAF_8BIT_MONO || format == EAF_8BIT_STEREO) ? 8 : 16;
		}

		void ProcessRawBytes(
			const char* rawBytes,
			size_t rawSize,
			ParaAudioFormats format,
			int srcSampleRate,
			int dstSampleRate,
			std::string& outPcm16le)
		{
			outPcm16le.clear();
			if (!rawBytes || rawSize == 0U || srcSampleRate <= 0 || dstSampleRate <= 0)
			{
				return;
			}

			const size_t frameBytes = BytesPerFrame(format);

			const char* workPtr = rawBytes;
			size_t workSize = rawSize;
			std::string combinedStorage;
			if (!m_remainderBytes.empty())
			{
				combinedStorage.reserve(m_remainderBytes.size() + rawSize);
				combinedStorage.append(m_remainderBytes);
				combinedStorage.append(rawBytes, rawSize);
				m_remainderBytes.clear();
				workPtr = combinedStorage.data();
				workSize = combinedStorage.size();
			}

			const size_t frameCount = workSize / frameBytes;
			const size_t consumedBytes = frameCount * frameBytes;
			if (consumedBytes < workSize)
			{
				m_remainderBytes.assign(workPtr + consumedBytes, workSize - consumedBytes);
			}
			if (frameCount == 0U)
			{
				return;
			}

			// Zero-copy fast path for 16-bit mono at exact target rate (e.g. 16000 Hz)
			if (format == EAF_16BIT_MONO && srcSampleRate == dstSampleRate && m_monoSamples.empty())
			{
				outPcm16le.assign(workPtr, consumedBytes);
				return;
			}

			m_monoSamples.reserve(m_monoSamples.size() + frameCount);
			const uint8_t* p = reinterpret_cast<const uint8_t*>(workPtr);
			for (size_t i = 0; i < frameCount; ++i)
			{
				float mono = 0.0f;
				switch (format)
				{
				case EAF_8BIT_MONO:
				{
					mono = static_cast<float>((static_cast<int>(p[i]) - 128) * 256);
					break;
				}
				case EAF_8BIT_STEREO:
				{
					const int l = (static_cast<int>(p[i * 2U + 0U]) - 128) * 256;
					const int r = (static_cast<int>(p[i * 2U + 1U]) - 128) * 256;
					mono = 0.5f * static_cast<float>(l + r);
					break;
				}
				case EAF_16BIT_MONO:
				{
					const uint16_t u = static_cast<uint16_t>(p[i * 2U + 0U]) |
						(static_cast<uint16_t>(p[i * 2U + 1U]) << 8U);
					mono = static_cast<float>(static_cast<int16_t>(u));
					break;
				}
				case EAF_16BIT_STEREO:
				default:
				{
					const uint16_t ul = static_cast<uint16_t>(p[i * 4U + 0U]) |
						(static_cast<uint16_t>(p[i * 4U + 1U]) << 8U);
					const uint16_t ur = static_cast<uint16_t>(p[i * 4U + 2U]) |
						(static_cast<uint16_t>(p[i * 4U + 3U]) << 8U);
					mono = 0.5f * (static_cast<float>(static_cast<int16_t>(ul)) +
						static_cast<float>(static_cast<int16_t>(ur)));
					break;
				}
				}
				m_monoSamples.push_back(mono);
			}

			auto appendSample16le = [&](float s) {
				const float clamped = (std::max)(-32768.0f, (std::min)(32767.0f, std::round(s)));
				const uint16_t u = static_cast<uint16_t>(static_cast<int16_t>(clamped));
				outPcm16le.push_back(static_cast<char>(u & 0xFFU));
				outPcm16le.push_back(static_cast<char>((u >> 8U) & 0xFFU));
			};

			if (srcSampleRate == dstSampleRate)
			{
				outPcm16le.reserve(m_monoSamples.size() * 2U);
				for (float s : m_monoSamples)
				{
					appendSample16le(s);
				}
				m_monoSamples.clear();
				m_srcPosNum = 0;
				return;
			}

			const int64_t srcRate64 = static_cast<int64_t>(srcSampleRate);
			const int64_t dstRate64 = static_cast<int64_t>(dstSampleRate);
			const double invDstRate = 1.0 / static_cast<double>(dstSampleRate);
			const double step = static_cast<double>(srcSampleRate) * invDstRate;
			const size_t approxOutSamples = static_cast<size_t>(
				(static_cast<uint64_t>(m_monoSamples.size()) * static_cast<uint64_t>(dstSampleRate)) /
				static_cast<uint64_t>(srcSampleRate)) + 2U;
			outPcm16le.reserve(approxOutSamples * 2U);

			if (srcRate64 >= dstRate64)
			{
				// Exact rational phase (in 1/dstSampleRate units) + continuous box-integral downsampling
				const int64_t availNum = static_cast<int64_t>(m_monoSamples.size()) * dstRate64;
				while (m_srcPosNum + srcRate64 <= availNum)
				{
					const int64_t aNum = m_srcPosNum;
					const int64_t bNum = m_srcPosNum + srcRate64;
					const size_t i0 = static_cast<size_t>(aNum / dstRate64);
					const size_t i1 = static_cast<size_t>(bNum / dstRate64);
					const double a = static_cast<double>(aNum) * invDstRate;
					const double b = static_cast<double>(bNum) * invDstRate;

					double weightedSum = 0.0;
					if (i0 == i1)
					{
						weightedSum = static_cast<double>(m_monoSamples[i0]) * (b - a);
					}
					else
					{
						weightedSum += static_cast<double>(m_monoSamples[i0]) * (static_cast<double>(i0 + 1U) - a);
						for (size_t k = i0 + 1U; k < i1; ++k)
						{
							weightedSum += static_cast<double>(m_monoSamples[k]);
						}
						const int64_t tailRem = bNum - static_cast<int64_t>(i1) * dstRate64;
						if (tailRem > 0 && i1 < m_monoSamples.size())
						{
							weightedSum += static_cast<double>(m_monoSamples[i1]) * (static_cast<double>(tailRem) * invDstRate);
						}
					}

					appendSample16le(static_cast<float>(weightedSum / step));
					m_srcPosNum = bNum;
				}

				const size_t consumedSamples = static_cast<size_t>(m_srcPosNum / dstRate64);
				if (consumedSamples > 0U)
				{
					m_monoSamples.erase(
						m_monoSamples.begin(),
						m_monoSamples.begin() + static_cast<std::ptrdiff_t>(consumedSamples));
					m_srcPosNum -= static_cast<int64_t>(consumedSamples) * dstRate64;
				}
			}
			else
			{
				// Exact rational phase linear interpolation upsampling (e.g. 8000 Hz -> 16000 Hz)
				while (m_srcPosNum + dstRate64 < static_cast<int64_t>(m_monoSamples.size()) * dstRate64)
				{
					const size_t i0 = static_cast<size_t>(m_srcPosNum / dstRate64);
					const float frac = static_cast<float>(
						static_cast<double>(m_srcPosNum - static_cast<int64_t>(i0) * dstRate64) * invDstRate);
					const float s = m_monoSamples[i0] * (1.0f - frac) + m_monoSamples[i0 + 1U] * frac;
					appendSample16le(s);
					m_srcPosNum += srcRate64;
				}

				const size_t consumedSamples = static_cast<size_t>(m_srcPosNum / dstRate64);
				if (consumedSamples > 0U)
				{
					m_monoSamples.erase(
						m_monoSamples.begin(),
						m_monoSamples.begin() + static_cast<std::ptrdiff_t>(consumedSamples));
					m_srcPosNum -= static_cast<int64_t>(consumedSamples) * dstRate64;
				}
			}
		}

	private:
		std::string m_remainderBytes;
		std::vector<float> m_monoSamples;
		int64_t m_srcPosNum = 0; // Fractional source sample index multiplied by dstSampleRate
	};
}
