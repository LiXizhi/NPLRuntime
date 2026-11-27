//-----------------------------------------------------------------------------
// Class:	ParaWebSocketClient
// Desc:	Cross-platform RFC 6455 WebSocket client (ws:// and wss://)
//          using libcurl CONNECT_ONLY on native platforms and
//          emscripten_websocket on WebAssembly.
//-----------------------------------------------------------------------------
#include "ParaWebSocketClient.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <unordered_map>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <thread>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/websocket.h>
#elif !defined(HARMONY_OS)
#include <curl/curl.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#endif

namespace ParaEngine
{
	namespace
	{
		// Minimal self-contained RFC 3174 SHA-1 for WebSocket Sec-WebSocket-Accept
		void Sha1Digest(const uint8_t* data, size_t len, uint8_t out[20])
		{
			auto rol32 = [](uint32_t v, unsigned int bits) -> uint32_t {
				return (v << bits) | (v >> (32U - bits));
			};

			uint32_t h0 = 0x67452301U;
			uint32_t h1 = 0xEFCDAB89U;
			uint32_t h2 = 0x98BADCFEU;
			uint32_t h3 = 0x10325476U;
			uint32_t h4 = 0xC3D2E1F0U;

			const uint64_t bitLen = static_cast<uint64_t>(len) * 8ULL;
			std::vector<uint8_t> msg(data, data + len);
			msg.push_back(0x80U);
			while ((msg.size() % 64U) != 56U)
			{
				msg.push_back(0x00U);
			}
			for (int i = 7; i >= 0; --i)
			{
				msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFFU));
			}

			for (size_t chunkOffset = 0; chunkOffset < msg.size(); chunkOffset += 64U)
			{
				uint32_t w[80];
				for (int i = 0; i < 16; ++i)
				{
					const size_t p = chunkOffset + static_cast<size_t>(i) * 4U;
					w[i] = (static_cast<uint32_t>(msg[p]) << 24U) |
						(static_cast<uint32_t>(msg[p + 1]) << 16U) |
						(static_cast<uint32_t>(msg[p + 2]) << 8U) |
						static_cast<uint32_t>(msg[p + 3]);
				}
				for (int i = 16; i < 80; ++i)
				{
					w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1U);
				}

				uint32_t a = h0;
				uint32_t b = h1;
				uint32_t c = h2;
				uint32_t d = h3;
				uint32_t e = h4;

				for (int i = 0; i < 80; ++i)
				{
					uint32_t f = 0;
					uint32_t k = 0;
					if (i < 20)
					{
						f = (b & c) | ((~b) & d);
						k = 0x5A827999U;
					}
					else if (i < 40)
					{
						f = b ^ c ^ d;
						k = 0x6ED9EBA1U;
					}
					else if (i < 60)
					{
						f = (b & c) | (b & d) | (c & d);
						k = 0x8F1BBCDCU;
					}
					else
					{
						f = b ^ c ^ d;
						k = 0xCA62C1D6U;
					}
					const uint32_t temp = rol32(a, 5U) + f + e + k + w[i];
					e = d;
					d = c;
					c = rol32(b, 30U);
					b = a;
					a = temp;
				}

				h0 += a;
				h1 += b;
				h2 += c;
				h3 += d;
				h4 += e;
			}

			const uint32_t hs[5] = { h0, h1, h2, h3, h4 };
			for (int i = 0; i < 5; ++i)
			{
				out[i * 4 + 0] = static_cast<uint8_t>((hs[i] >> 24U) & 0xFFU);
				out[i * 4 + 1] = static_cast<uint8_t>((hs[i] >> 16U) & 0xFFU);
				out[i * 4 + 2] = static_cast<uint8_t>((hs[i] >> 8U) & 0xFFU);
				out[i * 4 + 3] = static_cast<uint8_t>(hs[i] & 0xFFU);
			}
		}

		std::string ToLowerAscii(const std::string& input)
		{
			std::string out = input;
			std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			return out;
		}

		std::string TrimAscii(const std::string& s)
		{
			size_t b = 0;
			while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n'))
				++b;
			size_t e = s.size();
			while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n'))
				--e;
			return s.substr(b, e - b);
		}

		void FillRandomBytes(uint8_t* dst, size_t count)
		{
			thread_local std::mt19937 rng([]() {
				std::random_device rd;
				std::seed_seq seq{
					rd(),
					rd(),
					static_cast<unsigned int>(
						std::chrono::high_resolution_clock::now().time_since_epoch().count())
				};
				return std::mt19937(seq);
			}());

			size_t i = 0;
			while (i < count)
			{
				uint32_t r = rng();
				for (int b = 0; b < 4 && i < count; ++b, ++i)
				{
					dst[i] = static_cast<uint8_t>((r >> (b * 8)) & 0xFFU);
				}
			}
		}
	}

	namespace ParaWebSocketCodec
	{
		std::string Base64Encode(const uint8_t* data, size_t len)
		{
			static const char kTable[] =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			std::string out;
			out.reserve(((len + 2U) / 3U) * 4U);

			for (size_t i = 0; i < len; i += 3U)
			{
				const uint32_t b0 = data[i];
				const uint32_t b1 = (i + 1U < len) ? data[i + 1U] : 0U;
				const uint32_t b2 = (i + 2U < len) ? data[i + 2U] : 0U;
				const uint32_t triple = (b0 << 16U) | (b1 << 8U) | b2;

				out.push_back(kTable[(triple >> 18U) & 0x3FU]);
				out.push_back(kTable[(triple >> 12U) & 0x3FU]);
				out.push_back((i + 1U < len) ? kTable[(triple >> 6U) & 0x3FU] : '=');
				out.push_back((i + 2U < len) ? kTable[triple & 0x3FU] : '=');
			}
			return out;
		}

		std::string ComputeAcceptKey(const std::string& clientKeyBase64)
		{
			static const char kGuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			const std::string combined = clientKeyBase64 + kGuid;
			uint8_t sha1[20];
			Sha1Digest(reinterpret_cast<const uint8_t*>(combined.data()), combined.size(), sha1);
			return Base64Encode(sha1, sizeof(sha1));
		}

		bool ParseUrl(const std::string& url, ParsedWebSocketUrl& out, std::string& outError)
		{
			out = ParsedWebSocketUrl{};
			const size_t schemeEnd = url.find("://");
			if (schemeEnd == std::string::npos)
			{
				outError = "missing ws:// or wss:// scheme";
				return false;
			}

			const std::string scheme = ToLowerAscii(url.substr(0, schemeEnd));
			if (scheme == "wss")
			{
				out.secure = true;
				out.port = 443;
			}
			else if (scheme == "ws")
			{
				out.secure = false;
				out.port = 80;
			}
			else
			{
				outError = "unsupported WebSocket scheme: " + scheme;
				return false;
			}

			std::string rest = url.substr(schemeEnd + 3U);
			const size_t hashPos = rest.find('#');
			if (hashPos != std::string::npos)
			{
				rest.resize(hashPos);
			}

			size_t pathStart = rest.find_first_of("/?");
			std::string authority;
			if (pathStart == std::string::npos)
			{
				authority = rest;
				out.pathAndQuery = "/";
			}
			else
			{
				authority = rest.substr(0, pathStart);
				out.pathAndQuery = rest.substr(pathStart);
				if (!out.pathAndQuery.empty() && out.pathAndQuery[0] == '?')
				{
					out.pathAndQuery = "/" + out.pathAndQuery;
				}
			}

			if (authority.empty())
			{
				outError = "empty WebSocket host";
				return false;
			}

			std::string hostPart;
			std::string portPart;
			if (authority.front() == '[')
			{
				const size_t closeBracket = authority.find(']');
				if (closeBracket == std::string::npos)
				{
					outError = "malformed IPv6 host literal";
					return false;
				}
				hostPart = authority.substr(0, closeBracket + 1U);
				if (closeBracket + 1U < authority.size())
				{
					if (authority[closeBracket + 1U] != ':')
					{
						outError = "unexpected characters after IPv6 host literal";
						return false;
					}
					portPart = authority.substr(closeBracket + 2U);
				}
			}
			else
			{
				const size_t colonPos = authority.rfind(':');
				if (colonPos != std::string::npos)
				{
					hostPart = authority.substr(0, colonPos);
					portPart = authority.substr(colonPos + 1U);
				}
				else
				{
					hostPart = authority;
				}
			}

			if (hostPart.empty())
			{
				outError = "empty WebSocket host";
				return false;
			}
			out.host = hostPart;

			if (!portPart.empty())
			{
				char* endPtr = nullptr;
				const unsigned long parsedPort = std::strtoul(portPart.c_str(), &endPtr, 10);
				if (!endPtr || *endPtr != '\0' || parsedPort == 0UL || parsedPort > 65535UL)
				{
					outError = "invalid WebSocket port: " + portPart;
					return false;
				}
				out.port = static_cast<uint16_t>(parsedPort);
			}

			const uint16_t defaultPort = out.secure ? 443 : 80;
			if (out.port == defaultPort)
			{
				out.hostHeader = out.host;
			}
			else
			{
				out.hostHeader = out.host + ":" + std::to_string(out.port);
			}

			out.curlConnectUrl = std::string(out.secure ? "https://" : "http://") +
				out.host + ":" + std::to_string(out.port);
			return true;
		}

		void EncodeFrame(
			uint8_t opcode,
			const void* payload,
			size_t payloadLen,
			bool fin,
			bool masked,
			const uint8_t maskKey[4],
			std::string& outFrame)
		{
			outFrame.clear();
			outFrame.reserve(14U + payloadLen);

			const uint8_t b0 = static_cast<uint8_t>((fin ? 0x80U : 0x00U) | (opcode & 0x0FU));
			outFrame.push_back(static_cast<char>(b0));

			const uint8_t maskBit = masked ? 0x80U : 0x00U;
			if (payloadLen < 126U)
			{
				outFrame.push_back(static_cast<char>(maskBit | static_cast<uint8_t>(payloadLen)));
			}
			else if (payloadLen <= 65535U)
			{
				outFrame.push_back(static_cast<char>(maskBit | 126U));
				outFrame.push_back(static_cast<char>((payloadLen >> 8U) & 0xFFU));
				outFrame.push_back(static_cast<char>(payloadLen & 0xFFU));
			}
			else
			{
				outFrame.push_back(static_cast<char>(maskBit | 127U));
				const uint64_t len64 = static_cast<uint64_t>(payloadLen);
				for (int shift = 56; shift >= 0; shift -= 8)
				{
					outFrame.push_back(static_cast<char>((len64 >> shift) & 0xFFU));
				}
			}

			uint8_t effectiveMask[4] = { 0, 0, 0, 0 };
			if (masked)
			{
				if (maskKey)
				{
					std::memcpy(effectiveMask, maskKey, 4U);
				}
				else
				{
					FillRandomBytes(effectiveMask, 4U);
				}
				outFrame.append(reinterpret_cast<const char*>(effectiveMask), 4U);
			}

			if (payloadLen > 0U && payload != nullptr)
			{
				const size_t payloadOffset = outFrame.size();
				outFrame.resize(payloadOffset + payloadLen);
				const uint8_t* src = static_cast<const uint8_t*>(payload);
				uint8_t* dst = reinterpret_cast<uint8_t*>(&outFrame[payloadOffset]);
				if (masked)
				{
					for (size_t i = 0; i < payloadLen; ++i)
					{
						dst[i] = static_cast<uint8_t>(src[i] ^ effectiveMask[i & 3U]);
					}
				}
				else
				{
					std::memcpy(dst, src, payloadLen);
				}
			}
		}

		FrameStreamDecoder::FrameStreamDecoder(size_t maxMessageBytes)
			: m_maxMessageBytes(maxMessageBytes)
		{
		}

		void FrameStreamDecoder::AppendBytes(const char* bytes, size_t len)
		{
			if (bytes && len > 0U)
			{
				if (m_readOffset > 0U && m_readOffset == m_buffer.size())
				{
					m_buffer.clear();
					m_readOffset = 0U;
				}
				else if (m_readOffset >= 65536U && m_readOffset * 2U >= m_buffer.size())
				{
					m_buffer.erase(0U, m_readOffset);
					m_readOffset = 0U;
				}
				m_buffer.append(bytes, len);
			}
		}

		void FrameStreamDecoder::Reset()
		{
			m_buffer.clear();
			m_readOffset = 0U;
			m_inFragmentedMessage = false;
			m_fragmentOpcode = 0U;
			m_fragmentBuffer.clear();
		}

		bool FrameStreamDecoder::NextMessage(DecodedFrame& outFrame, std::string& outProtocolError)
		{
			outProtocolError.clear();

			while (true)
			{
				const size_t avail = m_buffer.size() - m_readOffset;
				if (avail < 2U)
				{
					return false;
				}

				const uint8_t* p = reinterpret_cast<const uint8_t*>(m_buffer.data() + m_readOffset);
				const bool fin = (p[0] & 0x80U) != 0U;
				const uint8_t rsv = static_cast<uint8_t>((p[0] >> 4U) & 0x07U);
				const uint8_t opcode = static_cast<uint8_t>(p[0] & 0x0FU);
				const bool masked = (p[1] & 0x80U) != 0U;
				uint64_t payloadLen = static_cast<uint64_t>(p[1] & 0x7FU);

				if (rsv != 0U)
				{
					outProtocolError = "unsupported WebSocket RSV bits";
					return false;
				}

				size_t headerLen = 2U;
				if (payloadLen == 126U)
				{
					if (avail < 4U)
						return false;
					payloadLen = (static_cast<uint64_t>(p[2]) << 8U) | static_cast<uint64_t>(p[3]);
					headerLen = 4U;
				}
				else if (payloadLen == 127U)
				{
					if (avail < 10U)
						return false;
					if ((p[2] & 0x80U) != 0U)
					{
						outProtocolError = "invalid 64-bit WebSocket payload length";
						return false;
					}
					payloadLen = 0U;
					for (int i = 2; i < 10; ++i)
					{
						payloadLen = (payloadLen << 8U) | static_cast<uint64_t>(p[i]);
					}
					headerLen = 10U;
				}

				const bool isControl = (opcode & 0x08U) != 0U;
				if (isControl && (!fin || payloadLen > 125U))
				{
					outProtocolError = "invalid WebSocket control frame";
					return false;
				}

				if (payloadLen > static_cast<uint64_t>(m_maxMessageBytes))
				{
					outProtocolError = "WebSocket frame exceeds maximum allowed message size";
					return false;
				}

				uint8_t maskKey[4] = { 0, 0, 0, 0 };
				if (masked)
				{
					if (avail < headerLen + 4U)
						return false;
					std::memcpy(maskKey, p + headerLen, 4U);
					headerLen += 4U;
				}

				const size_t totalFrameBytes = headerLen + static_cast<size_t>(payloadLen);
				if (avail < totalFrameBytes)
				{
					return false;
				}

				std::string framePayload;
				framePayload.resize(static_cast<size_t>(payloadLen));
				if (payloadLen > 0U)
				{
					const uint8_t* payloadSrc = p + headerLen;
					if (masked)
					{
						for (size_t i = 0; i < static_cast<size_t>(payloadLen); ++i)
						{
							framePayload[i] = static_cast<char>(payloadSrc[i] ^ maskKey[i & 3U]);
						}
					}
					else
					{
						std::memcpy(&framePayload[0], payloadSrc, static_cast<size_t>(payloadLen));
					}
				}

				m_readOffset += totalFrameBytes;
				if (m_readOffset == m_buffer.size())
				{
					m_buffer.clear();
					m_readOffset = 0U;
				}

				if (isControl)
				{
					outFrame.opcode = opcode;
					outFrame.fin = true;
					outFrame.payload = std::move(framePayload);
					return true;
				}

				if (opcode == kOpcodeContinuation)
				{
					if (!m_inFragmentedMessage)
					{
						outProtocolError = "unexpected WebSocket continuation frame";
						return false;
					}
					if (m_fragmentBuffer.size() + framePayload.size() > m_maxMessageBytes)
					{
						outProtocolError = "fragmented WebSocket message exceeds maximum allowed size";
						return false;
					}
					m_fragmentBuffer.append(framePayload);
					if (fin)
					{
						outFrame.opcode = m_fragmentOpcode;
						outFrame.fin = true;
						outFrame.payload = std::move(m_fragmentBuffer);
						m_fragmentBuffer.clear();
						m_inFragmentedMessage = false;
						m_fragmentOpcode = 0U;
						return true;
					}
					continue;
				}

				if (opcode != kOpcodeText && opcode != kOpcodeBinary)
				{
					outProtocolError = "unsupported WebSocket data opcode";
					return false;
				}

				if (m_inFragmentedMessage)
				{
					outProtocolError = "new WebSocket data frame started before fragmented message finished";
					return false;
				}

				if (fin)
				{
					outFrame.opcode = opcode;
					outFrame.fin = true;
					outFrame.payload = std::move(framePayload);
					return true;
				}

				m_inFragmentedMessage = true;
				m_fragmentOpcode = opcode;
				m_fragmentBuffer = std::move(framePayload);
			}
		}
	}

	namespace
	{
		struct ConnectionContext
		{
			int handle = 0;
			std::string url;
			std::string extraHeaders;
			std::atomic<int> state{ static_cast<int>(WebSocketReadyState::Connecting) };
			std::atomic<bool> stopRequested{ false };

			std::mutex mutex;
			std::deque<WebSocketEvent> rxEvents;
			std::deque<std::string> txFrames;
			bool closeFrameQueued = false;

#ifdef __EMSCRIPTEN__
			EMSCRIPTEN_WEBSOCKET_T emSocket = 0;
#endif

			void PushEvent(const std::string& type, std::string data, int code = 0)
			{
				std::lock_guard<std::mutex> lock(mutex);
				rxEvents.push_back(WebSocketEvent{ type, std::move(data), code });
			}

			void QueueEncodedFrame(uint8_t opcode, const void* payload, size_t len)
			{
				std::string frame;
				ParaWebSocketCodec::EncodeFrame(opcode, payload, len, true, true, nullptr, frame);
				std::lock_guard<std::mutex> lock(mutex);
				txFrames.push_back(std::move(frame));
			}
		};

#if !defined(__EMSCRIPTEN__) && !defined(HARMONY_OS)
		void EnsureCurlGlobalInitialized()
		{
			static std::once_flag s_once;
			std::call_once(s_once, []() {
				curl_global_init(CURL_GLOBAL_DEFAULT);
			});
		}

		curl_socket_t GetActiveCurlSocket(CURL* curl)
		{
			curl_socket_t sockfd = CURL_SOCKET_BAD;
#ifdef CURLINFO_ACTIVESOCKET
			if (curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd) == CURLE_OK &&
				sockfd != CURL_SOCKET_BAD)
			{
				return sockfd;
			}
#endif
			long legacySock = -1;
			if (curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &legacySock) == CURLE_OK &&
				legacySock >= 0)
			{
				return static_cast<curl_socket_t>(legacySock);
			}
			return CURL_SOCKET_BAD;
		}

		void WaitOnSocket(curl_socket_t sockfd, bool waitWrite, int timeoutMs)
		{
			if (sockfd == CURL_SOCKET_BAD || timeoutMs <= 0)
			{
#ifndef EMSCRIPTEN_SINGLE_THREAD
				std::this_thread::sleep_for(std::chrono::milliseconds((std::max)(1, timeoutMs)));
#endif
				return;
			}

#ifndef _WIN32
			if (sockfd < 0 || sockfd >= FD_SETSIZE)
			{
#ifndef EMSCRIPTEN_SINGLE_THREAD
				std::this_thread::sleep_for(std::chrono::milliseconds((std::max)(1, timeoutMs)));
#endif
				return;
			}
#endif

			fd_set readSet;
			fd_set writeSet;
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);
			FD_SET(sockfd, &readSet);
			if (waitWrite)
			{
				FD_SET(sockfd, &writeSet);
			}

			struct timeval tv;
			tv.tv_sec = timeoutMs / 1000;
			tv.tv_usec = (timeoutMs % 1000) * 1000;
			const int nfds =
#ifdef _WIN32
				0;
#else
				static_cast<int>(sockfd) + 1;
#endif
			select(nfds, &readSet, waitWrite ? &writeSet : nullptr, nullptr, &tv);
		}

		bool SendAllBytes(
			CURL* curl,
			curl_socket_t sockfd,
			const char* data,
			size_t totalLen,
			const std::atomic<bool>& stopRequested,
			int timeoutMs,
			std::string& outError)
		{
			size_t sentTotal = 0U;
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

			while (sentTotal < totalLen)
			{
				if (stopRequested.load(std::memory_order_acquire))
				{
					outError = "connection closed while sending";
					return false;
				}
				size_t sentNow = 0U;
				const CURLcode rc = curl_easy_send(curl, data + sentTotal, totalLen - sentTotal, &sentNow);
				if (rc == CURLE_OK)
				{
					sentTotal += sentNow;
					continue;
				}
				if (rc == CURLE_AGAIN)
				{
					if (std::chrono::steady_clock::now() >= deadline)
					{
						outError = "send timed out";
						return false;
					}
					WaitOnSocket(sockfd, true, 10);
					continue;
				}
				outError = std::string("curl_easy_send failed: ") + curl_easy_strerror(rc);
				return false;
			}
			return true;
		}

		bool ParseHttpUpgradeHeaders(
			const std::string& rawHeaders,
			const std::string& expectedAcceptKey,
			std::string& outError)
		{
			std::istringstream iss(rawHeaders);
			std::string statusLine;
			if (!std::getline(iss, statusLine))
			{
				outError = "empty HTTP upgrade response";
				return false;
			}
			statusLine = TrimAscii(statusLine);
			// Expect: HTTP/1.1 101 ...
			if (statusLine.find(" 101 ") == std::string::npos &&
				!(statusLine.size() >= 12U && statusLine.compare(statusLine.size() - 4U, 4U, " 101") == 0))
			{
				outError = "WebSocket upgrade rejected: " + statusLine;
				return false;
			}

			bool hasUpgradeWebsocket = false;
			bool hasAcceptMatch = false;
			std::string line;
			while (std::getline(iss, line))
			{
				line = TrimAscii(line);
				if (line.empty())
					continue;
				const size_t colon = line.find(':');
				if (colon == std::string::npos)
					continue;
				const std::string name = ToLowerAscii(TrimAscii(line.substr(0, colon)));
				const std::string val = TrimAscii(line.substr(colon + 1U));
				if (name == "upgrade" && ToLowerAscii(val).find("websocket") != std::string::npos)
				{
					hasUpgradeWebsocket = true;
				}
				else if (name == "sec-websocket-accept" && val == expectedAcceptKey)
				{
					hasAcceptMatch = true;
				}
			}

			if (!hasUpgradeWebsocket)
			{
				outError = "WebSocket upgrade response missing Upgrade: websocket header";
				return false;
			}
			if (!hasAcceptMatch)
			{
				outError = "WebSocket upgrade Sec-WebSocket-Accept mismatch";
				return false;
			}
			return true;
		}

		void RunCurlWebSocketWorker(std::shared_ptr<ConnectionContext> ctx)
		{
			EnsureCurlGlobalInitialized();

			ParsedWebSocketUrl parsed;
			std::string err;
			if (!ParaWebSocketCodec::ParseUrl(ctx->url, parsed, err))
			{
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", err, -1);
				ctx->PushEvent("close", err, 1006);
				return;
			}

			CURL* curl = curl_easy_init();
			if (!curl)
			{
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", "curl_easy_init failed", -1);
				ctx->PushEvent("close", "curl_easy_init failed", 1006);
				return;
			}

			struct CurlCleanupGuard
			{
				CURL* handle;
				~CurlCleanupGuard()
				{
					if (handle)
						curl_easy_cleanup(handle);
				}
			} guard{ curl };

			curl_easy_setopt(curl, CURLOPT_URL, parsed.curlConnectUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
			curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
			curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 10000L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#if defined(ANDROID)
			curl_easy_setopt(curl, CURLOPT_CAPATH, "/system/etc/security/cacerts");
#endif

			const CURLcode connectRc = curl_easy_perform(curl);
			if (connectRc != CURLE_OK)
			{
				const std::string connectErr =
					std::string("WebSocket connect failed: ") + curl_easy_strerror(connectRc);
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", connectErr, static_cast<int>(connectRc));
				ctx->PushEvent("close", connectErr, 1006);
				return;
			}

			if (ctx->stopRequested.load(std::memory_order_acquire))
			{
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				return;
			}

			const curl_socket_t sockfd = GetActiveCurlSocket(curl);

			uint8_t nonceBytes[16];
			FillRandomBytes(nonceBytes, sizeof(nonceBytes));
			const std::string clientKey = ParaWebSocketCodec::Base64Encode(nonceBytes, sizeof(nonceBytes));
			const std::string expectedAccept = ParaWebSocketCodec::ComputeAcceptKey(clientKey);

			std::string req;
			req.reserve(512U);
			req += "GET " + parsed.pathAndQuery + " HTTP/1.1\r\n";
			req += "Host: " + parsed.hostHeader + "\r\n";
			req += "Upgrade: websocket\r\n";
			req += "Connection: Upgrade\r\n";
			req += "Sec-WebSocket-Key: " + clientKey + "\r\n";
			req += "Sec-WebSocket-Version: 13\r\n";
			if (!ctx->extraHeaders.empty())
			{
				std::istringstream extraStream(ctx->extraHeaders);
				std::string hline;
				while (std::getline(extraStream, hline))
				{
					hline = TrimAscii(hline);
					if (!hline.empty() && hline.find(':') != std::string::npos)
					{
						req += hline + "\r\n";
					}
				}
			}
			req += "\r\n";

			if (!SendAllBytes(curl, sockfd, req.data(), req.size(), ctx->stopRequested, 10000, err))
			{
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", err, -1);
				ctx->PushEvent("close", err, 1006);
				return;
			}

			std::string handshakeBuf;
			handshakeBuf.reserve(1024U);
			size_t headerEndPos = std::string::npos;
			const auto handshakeDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
			char ioChunk[8192];

			while (headerEndPos == std::string::npos)
			{
				if (ctx->stopRequested.load(std::memory_order_acquire))
				{
					ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
					return;
				}
				size_t nRead = 0U;
				const CURLcode rc = curl_easy_recv(curl, ioChunk, sizeof(ioChunk), &nRead);
				if (rc == CURLE_OK)
				{
					if (nRead == 0U)
					{
						ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
						ctx->PushEvent("error", "peer closed socket during WebSocket upgrade", -1);
						ctx->PushEvent("close", "peer closed socket during WebSocket upgrade", 1006);
						return;
					}
					handshakeBuf.append(ioChunk, nRead);
					headerEndPos = handshakeBuf.find("\r\n\r\n");
					if (headerEndPos == std::string::npos && handshakeBuf.size() > 65536U)
					{
						ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
						ctx->PushEvent("error", "WebSocket upgrade headers exceeded 64 KB", -1);
						ctx->PushEvent("close", "WebSocket upgrade headers exceeded 64 KB", 1002);
						return;
					}
					continue;
				}
				if (rc == CURLE_AGAIN)
				{
					if (std::chrono::steady_clock::now() >= handshakeDeadline)
					{
						ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
						ctx->PushEvent("error", "WebSocket upgrade handshake timed out", -1);
						ctx->PushEvent("close", "WebSocket upgrade handshake timed out", 1006);
						return;
					}
					WaitOnSocket(sockfd, false, 15);
					continue;
				}

				const std::string recvErr =
					std::string("WebSocket upgrade read failed: ") + curl_easy_strerror(rc);
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", recvErr, static_cast<int>(rc));
				ctx->PushEvent("close", recvErr, 1006);
				return;
			}

			const std::string headerBlock = handshakeBuf.substr(0U, headerEndPos);
			if (!ParseHttpUpgradeHeaders(headerBlock, expectedAccept, err))
			{
				ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				ctx->PushEvent("error", err, -1);
				ctx->PushEvent("close", err, 1002);
				return;
			}

			ParaWebSocketCodec::FrameStreamDecoder decoder;
			const size_t leftoverOffset = headerEndPos + 4U;
			if (leftoverOffset < handshakeBuf.size())
			{
				decoder.AppendBytes(
					handshakeBuf.data() + leftoverOffset,
					handshakeBuf.size() - leftoverOffset);
			}

			ctx->state.store(static_cast<int>(WebSocketReadyState::Open), std::memory_order_release);
			ctx->PushEvent("open", "", 101);

			auto drainDecodedFrames = [&](bool& peerClosedOut) -> bool {
				ParaWebSocketCodec::DecodedFrame frame;
				std::string protoErr;
				while (decoder.NextMessage(frame, protoErr))
				{
					if (frame.opcode == ParaWebSocketCodec::kOpcodeText)
					{
						ctx->PushEvent("text", std::move(frame.payload), 0);
					}
					else if (frame.opcode == ParaWebSocketCodec::kOpcodeBinary)
					{
						ctx->PushEvent("binary", std::move(frame.payload), 0);
					}
					else if (frame.opcode == ParaWebSocketCodec::kOpcodePing)
					{
						ctx->QueueEncodedFrame(
							ParaWebSocketCodec::kOpcodePong,
							frame.payload.data(),
							frame.payload.size());
					}
					else if (frame.opcode == ParaWebSocketCodec::kOpcodePong)
					{
						// Keepalive pong acknowledged.
					}
					else if (frame.opcode == ParaWebSocketCodec::kOpcodeClose)
					{
						int closeCode = 1005;
						std::string closeReason;
						if (frame.payload.size() >= 2U)
						{
							closeCode = (static_cast<int>(static_cast<uint8_t>(frame.payload[0])) << 8) |
								static_cast<int>(static_cast<uint8_t>(frame.payload[1]));
							closeReason = frame.payload.substr(2U);
						}

						bool shouldEchoClose = false;
						{
							std::lock_guard<std::mutex> lock(ctx->mutex);
							if (!ctx->closeFrameQueued)
							{
								ctx->closeFrameQueued = true;
								shouldEchoClose = true;
							}
						}
						if (shouldEchoClose)
						{
							uint8_t replyPayload[2] = {
								static_cast<uint8_t>((closeCode >> 8) & 0xFF),
								static_cast<uint8_t>(closeCode & 0xFF)
							};
							std::string echoFrame;
							ParaWebSocketCodec::EncodeFrame(
								ParaWebSocketCodec::kOpcodeClose,
								frame.payload.size() >= 2U ? static_cast<const void*>(replyPayload) : nullptr,
								frame.payload.size() >= 2U ? 2U : 0U,
								true,
								true,
								nullptr,
								echoFrame);
							std::string sendIgnoreErr;
							SendAllBytes(
								curl,
								sockfd,
								echoFrame.data(),
								echoFrame.size(),
								ctx->stopRequested,
								500,
								sendIgnoreErr);
						}

						ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
						ctx->PushEvent("close", closeReason, closeCode);
						peerClosedOut = true;
						return true;
					}
				}

				if (!protoErr.empty())
				{
					ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
					ctx->PushEvent("error", protoErr, 1002);
					ctx->PushEvent("close", protoErr, 1002);
					peerClosedOut = true;
					return false;
				}
				return true;
			};

			bool peerClosed = false;
			drainDecodedFrames(peerClosed);

			while (!peerClosed)
			{
				// 1. Flush any outbound frames queued by Lua or Ping->Pong.
				std::deque<std::string> pendingTx;
				bool localCloseSent = false;
				{
					std::lock_guard<std::mutex> lock(ctx->mutex);
					if (!ctx->txFrames.empty())
					{
						pendingTx.swap(ctx->txFrames);
					}
					localCloseSent = ctx->closeFrameQueued;
				}

				for (const std::string& frameBytes : pendingTx)
				{
					std::atomic<bool> neverAbortSend{ false };
					if (!SendAllBytes(curl, sockfd, frameBytes.data(), frameBytes.size(), neverAbortSend, 8000, err))
					{
						ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
						ctx->PushEvent("error", err, -1);
						ctx->PushEvent("close", err, 1006);
						return;
					}
				}

				if (ctx->stopRequested.load(std::memory_order_acquire))
				{
					if (!localCloseSent)
					{
						uint8_t normalClose[2] = { 0x03U, 0xE8U }; // 1000
						std::string closeFrame;
						ParaWebSocketCodec::EncodeFrame(
							ParaWebSocketCodec::kOpcodeClose,
							normalClose,
							sizeof(normalClose),
							true,
							true,
							nullptr,
							closeFrame);
						std::atomic<bool> neverAbort{ false };
						std::string ignoreErr;
						SendAllBytes(curl, sockfd, closeFrame.data(), closeFrame.size(), neverAbort, 300, ignoreErr);
					}
					ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
					return;
				}

				// 2. Drain all currently available bytes from TLS/socket until CURLE_AGAIN.
				bool receivedAny = false;
				while (!peerClosed)
				{
					size_t nRead = 0U;
					const CURLcode rc = curl_easy_recv(curl, ioChunk, sizeof(ioChunk), &nRead);
					if (rc == CURLE_OK)
					{
						if (nRead == 0U)
						{
							ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
							ctx->PushEvent("close", "EOF", 1006);
							return;
						}
						receivedAny = true;
						decoder.AppendBytes(ioChunk, nRead);
						drainDecodedFrames(peerClosed);
						continue;
					}
					if (rc == CURLE_AGAIN)
					{
						break;
					}
					const std::string recvErr =
						std::string("curl_easy_recv failed: ") + curl_easy_strerror(rc);
					ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
					ctx->PushEvent("error", recvErr, static_cast<int>(rc));
					ctx->PushEvent("close", recvErr, 1006);
					return;
				}

				if (peerClosed)
					break;

				// 3. Brief wait if idle so outbound SendBinary calls wake within <= 5 ms.
				bool hasMoreTx = false;
				{
					std::lock_guard<std::mutex> lock(ctx->mutex);
					hasMoreTx = !ctx->txFrames.empty();
				}
				if (!hasMoreTx && !receivedAny && !ctx->stopRequested.load(std::memory_order_acquire))
				{
					WaitOnSocket(sockfd, false, 5);
				}
			}
		}
#endif // !defined(__EMSCRIPTEN__) && !defined(HARMONY_OS)

#ifdef __EMSCRIPTEN__
		EM_BOOL OnEmWsOpen(int /*eventType*/, const EmscriptenWebSocketOpenEvent* /*e*/, void* userData)
		{
			auto* rawPtr = static_cast<std::shared_ptr<ConnectionContext>*>(userData);
			if (rawPtr && *rawPtr)
			{
				(*rawPtr)->state.store(static_cast<int>(WebSocketReadyState::Open), std::memory_order_release);
				(*rawPtr)->PushEvent("open", "", 101);
			}
			return EM_TRUE;
		}

		EM_BOOL OnEmWsMessage(int /*eventType*/, const EmscriptenWebSocketMessageEvent* e, void* userData)
		{
			auto* rawPtr = static_cast<std::shared_ptr<ConnectionContext>*>(userData);
			if (rawPtr && *rawPtr && e)
			{
				if (e->isText)
				{
					const size_t textLen = (e->numBytes > 0 && e->data[e->numBytes - 1] == 0)
						? static_cast<size_t>(e->numBytes - 1)
						: static_cast<size_t>(e->numBytes);
					(*rawPtr)->PushEvent("text", std::string(reinterpret_cast<const char*>(e->data), textLen), 0);
				}
				else
				{
					(*rawPtr)->PushEvent(
						"binary",
						std::string(reinterpret_cast<const char*>(e->data), static_cast<size_t>(e->numBytes)),
						0);
				}
			}
			return EM_TRUE;
		}

		EM_BOOL OnEmWsError(int /*eventType*/, const EmscriptenWebSocketErrorEvent* /*e*/, void* userData)
		{
			auto* rawPtr = static_cast<std::shared_ptr<ConnectionContext>*>(userData);
			if (rawPtr && *rawPtr)
			{
				(*rawPtr)->PushEvent("error", "WebSocket error", -1);
			}
			return EM_TRUE;
		}

		EM_BOOL OnEmWsClose(int /*eventType*/, const EmscriptenWebSocketCloseEvent* e, void* userData)
		{
			auto* rawPtr = static_cast<std::shared_ptr<ConnectionContext>*>(userData);
			if (rawPtr && *rawPtr)
			{
				(*rawPtr)->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
				(*rawPtr)->PushEvent("close", e ? std::string(e->reason) : std::string(), e ? static_cast<int>(e->code) : 1005);
			}
			return EM_TRUE;
		}
#endif

		struct ManagerStorage
		{
			std::mutex tableMutex;
			int nextHandle = 1;
			std::unordered_map<int, std::shared_ptr<ConnectionContext>> connections;
#ifdef __EMSCRIPTEN__
			std::unordered_map<int, std::shared_ptr<ConnectionContext>*> emCallbackHolders;
#endif
		};

		ManagerStorage& GetStorage()
		{
			static ManagerStorage s_storage;
			return s_storage;
		}
	}

	ParaWebSocketClientManager& ParaWebSocketClientManager::GetSingleton()
	{
		static ParaWebSocketClientManager s_instance;
		return s_instance;
	}

	int ParaWebSocketClientManager::Connect(const std::string& url, const std::string& extraHeaders)
	{
		if (url.empty())
			return 0;

		auto ctx = std::make_shared<ConnectionContext>();
		ctx->url = url;
		ctx->extraHeaders = extraHeaders;

		auto& storage = GetStorage();
		int handle = 0;
		{
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			// Opportunistically reap leaked closed handles with empty event queues when table grows large.
			if (storage.connections.size() > 256U)
			{
				for (auto it = storage.connections.begin(); it != storage.connections.end();)
				{
					auto& c = it->second;
					bool canErase = false;
					if (c && c->state.load(std::memory_order_acquire) == static_cast<int>(WebSocketReadyState::Closed))
					{
						std::lock_guard<std::mutex> evLock(c->mutex);
						canErase = c->rxEvents.empty();
					}
					if (canErase)
						it = storage.connections.erase(it);
					else
						++it;
				}
			}

			handle = storage.nextHandle++;
			if (storage.nextHandle <= 0)
				storage.nextHandle = 1;
			ctx->handle = handle;
			storage.connections[handle] = ctx;
		}

#ifdef __EMSCRIPTEN__
		if (!emscripten_websocket_is_supported())
		{
			ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
			ctx->PushEvent("error", "Browser WebSocket not supported", -1);
			ctx->PushEvent("close", "Browser WebSocket not supported", 1006);
			return handle;
		}
		EmscriptenWebSocketCreateAttributes attr;
		emscripten_websocket_init_create_attributes(&attr);
		attr.url = ctx->url.c_str();
		attr.createOnMainThread = EM_TRUE;
		EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&attr);
		if (ws <= 0)
		{
			ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
			ctx->PushEvent("error", "emscripten_websocket_new failed", static_cast<int>(ws));
			ctx->PushEvent("close", "emscripten_websocket_new failed", 1006);
			return handle;
		}
		ctx->emSocket = ws;
		auto* holder = new std::shared_ptr<ConnectionContext>(ctx);
		{
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			storage.emCallbackHolders[handle] = holder;
		}
		emscripten_websocket_set_onopen_callback(ws, holder, OnEmWsOpen);
		emscripten_websocket_set_onmessage_callback(ws, holder, OnEmWsMessage);
		emscripten_websocket_set_onerror_callback(ws, holder, OnEmWsError);
		emscripten_websocket_set_onclose_callback(ws, holder, OnEmWsClose);
#elif defined(HARMONY_OS) || defined(EMSCRIPTEN_SINGLE_THREAD)
		ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
		ctx->PushEvent("error", "WebSocket worker thread unavailable on this target", -1);
		ctx->PushEvent("close", "WebSocket worker thread unavailable on this target", 1006);
#else
		std::thread worker(&RunCurlWebSocketWorker, ctx);
		worker.detach();
#endif

		return handle;
	}

	bool ParaWebSocketClientManager::SendText(int handle, const std::string& text)
	{
		std::shared_ptr<ConnectionContext> ctx;
		{
			auto& storage = GetStorage();
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			auto it = storage.connections.find(handle);
			if (it == storage.connections.end())
				return false;
			ctx = it->second;
		}
		if (!ctx || ctx->state.load(std::memory_order_acquire) != static_cast<int>(WebSocketReadyState::Open))
			return false;

#ifdef __EMSCRIPTEN__
		if (ctx->emSocket <= 0)
			return false;
		return emscripten_websocket_send_utf8_text(ctx->emSocket, text.c_str()) == EMSCRIPTEN_RESULT_SUCCESS;
#else
		ctx->QueueEncodedFrame(ParaWebSocketCodec::kOpcodeText, text.data(), text.size());
		return true;
#endif
	}

	bool ParaWebSocketClientManager::SendBinary(int handle, const std::string& binaryData)
	{
		std::shared_ptr<ConnectionContext> ctx;
		{
			auto& storage = GetStorage();
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			auto it = storage.connections.find(handle);
			if (it == storage.connections.end())
				return false;
			ctx = it->second;
		}
		if (!ctx || ctx->state.load(std::memory_order_acquire) != static_cast<int>(WebSocketReadyState::Open))
			return false;

#ifdef __EMSCRIPTEN__
		if (ctx->emSocket <= 0)
			return false;
		return emscripten_websocket_send_binary(
			ctx->emSocket,
			const_cast<char*>(binaryData.data()),
			static_cast<uint32_t>(binaryData.size())) == EMSCRIPTEN_RESULT_SUCCESS;
#else
		ctx->QueueEncodedFrame(ParaWebSocketCodec::kOpcodeBinary, binaryData.data(), binaryData.size());
		return true;
#endif
	}

	int ParaWebSocketClientManager::GetState(int handle)
	{
		auto& storage = GetStorage();
		std::lock_guard<std::mutex> lock(storage.tableMutex);
		auto it = storage.connections.find(handle);
		if (it == storage.connections.end() || !it->second)
			return static_cast<int>(WebSocketReadyState::Closed);
		return it->second->state.load(std::memory_order_acquire);
	}

	bool ParaWebSocketClientManager::PollEvent(int handle, WebSocketEvent& outEvent)
	{
		std::shared_ptr<ConnectionContext> ctx;
		{
			auto& storage = GetStorage();
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			auto it = storage.connections.find(handle);
			if (it == storage.connections.end() || !it->second)
				return false;
			ctx = it->second;
		}

		std::lock_guard<std::mutex> lock(ctx->mutex);
		if (ctx->rxEvents.empty())
			return false;
		outEvent = std::move(ctx->rxEvents.front());
		ctx->rxEvents.pop_front();
		return true;
	}

	void ParaWebSocketClientManager::Close(int handle, int code, const std::string& reason)
	{
		std::shared_ptr<ConnectionContext> ctx;
#ifdef __EMSCRIPTEN__
		std::shared_ptr<ConnectionContext>* emHolder = nullptr;
#endif
		{
			auto& storage = GetStorage();
			std::lock_guard<std::mutex> lock(storage.tableMutex);
			auto it = storage.connections.find(handle);
			if (it != storage.connections.end())
			{
				ctx = it->second;
				storage.connections.erase(it);
			}
#ifdef __EMSCRIPTEN__
			auto hit = storage.emCallbackHolders.find(handle);
			if (hit != storage.emCallbackHolders.end())
			{
				emHolder = hit->second;
				storage.emCallbackHolders.erase(hit);
			}
#endif
		}

		if (!ctx)
			return;

#ifdef __EMSCRIPTEN__
		if (ctx->emSocket > 0)
		{
			emscripten_websocket_close(
				ctx->emSocket,
				static_cast<unsigned short>(code),
				reason.empty() ? nullptr : reason.c_str());
			emscripten_websocket_delete(ctx->emSocket);
			ctx->emSocket = 0;
		}
		ctx->state.store(static_cast<int>(WebSocketReadyState::Closed), std::memory_order_release);
		delete emHolder;
#else
		const int prevState = ctx->state.load(std::memory_order_acquire);
		if (prevState == static_cast<int>(WebSocketReadyState::Open))
		{
			ctx->state.store(static_cast<int>(WebSocketReadyState::Closing), std::memory_order_release);
			std::string closePayload;
			closePayload.push_back(static_cast<char>((code >> 8) & 0xFF));
			closePayload.push_back(static_cast<char>(code & 0xFF));
			if (!reason.empty())
			{
				const size_t maxReason = (std::min)(reason.size(), static_cast<size_t>(123U));
				closePayload.append(reason.data(), maxReason);
			}
			std::string closeFrame;
			ParaWebSocketCodec::EncodeFrame(
				ParaWebSocketCodec::kOpcodeClose,
				closePayload.data(),
				closePayload.size(),
				true,
				true,
				nullptr,
				closeFrame);
			std::lock_guard<std::mutex> lock(ctx->mutex);
			ctx->closeFrameQueued = true;
			ctx->txFrames.push_back(std::move(closeFrame));
		}
		ctx->stopRequested.store(true, std::memory_order_release);
#endif
	}
}
