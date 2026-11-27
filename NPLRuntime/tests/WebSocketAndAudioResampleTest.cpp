#include "WebSocket/ParaWebSocketClient.h"
#include "3dengine/AudioCaptureResampler.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using TestSocket = SOCKET;
static constexpr TestSocket kInvalidTestSocket = INVALID_SOCKET;
static void CloseTestSocket(TestSocket s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using TestSocket = int;
static constexpr TestSocket kInvalidTestSocket = -1;
static void CloseTestSocket(TestSocket s) { close(s); }
#endif

namespace
{
	bool TestUrlAndAcceptKey()
	{
		using namespace ParaEngine;
		// RFC 6455 Section 1.3 canonical test vector
		const std::string accept = ParaWebSocketCodec::ComputeAcceptKey("dGhlIHNhbXBsZSBub25jZQ==");
		if (accept != "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=")
		{
			std::fprintf(stderr, "ComputeAcceptKey mismatch: got '%s'\n", accept.c_str());
			return false;
		}

		ParsedWebSocketUrl parsed;
		std::string err;
		if (!ParaWebSocketCodec::ParseUrl(
				"wss://speechrtc.keepwork.com/speech?asr_format=pcm&asr_rate=16000",
				parsed,
				err))
		{
			std::fprintf(stderr, "ParseUrl wss failed: %s\n", err.c_str());
			return false;
		}
		if (!parsed.secure || parsed.host != "speechrtc.keepwork.com" || parsed.port != 443 ||
			parsed.hostHeader != "speechrtc.keepwork.com" ||
			parsed.pathAndQuery != "/speech?asr_format=pcm&asr_rate=16000" ||
			parsed.curlConnectUrl != "https://speechrtc.keepwork.com:443")
		{
			std::fprintf(stderr, "ParseUrl wss unexpected fields\n");
			return false;
		}

		if (!ParaWebSocketCodec::ParseUrl("ws://127.0.0.1:9001?foo=1", parsed, err) ||
			parsed.secure || parsed.port != 9001 || parsed.hostHeader != "127.0.0.1:9001" ||
			parsed.pathAndQuery != "/?foo=1")
		{
			std::fprintf(stderr, "ParseUrl ws custom port failed\n");
			return false;
		}
		return true;
	}

	bool TestFrameCodec()
	{
		using namespace ParaEngine;
		using namespace ParaEngine::ParaWebSocketCodec;

		FrameStreamDecoder decoder;
		const uint8_t mask[4] = { 0x37U, 0xFAU, 0x21U, 0x3DU };

		// 1. Binary payload containing embedded NULs and > 126 byte 16-bit length
		std::string pcmPayload(6400U, '\0');
		for (size_t i = 0; i < pcmPayload.size(); ++i)
		{
			pcmPayload[i] = static_cast<char>((i * 73U) & 0xFFU);
		}
		std::string wire;
		EncodeFrame(kOpcodeBinary, pcmPayload.data(), pcmPayload.size(), true, true, mask, wire);

		// Feed byte-by-byte to stress partial header & payload streaming
		for (char c : wire)
		{
			decoder.AppendBytes(&c, 1U);
		}
		DecodedFrame out;
		std::string err;
		if (!decoder.NextMessage(out, err) || !err.empty() ||
			out.opcode != kOpcodeBinary || out.payload != pcmPayload)
		{
			std::fprintf(stderr, "6400-byte masked binary decode failed: '%s'\n", err.c_str());
			return false;
		}

		// 2. Fragmented text interleaved with a Ping control frame (RFC 6455 Section 5.4)
		std::string f1, pingFrame, f2;
		EncodeFrame(kOpcodeText, "{\"type\":\"asr\",", 14U, false, false, nullptr, f1);
		EncodeFrame(kOpcodePing, "hb", 2U, true, false, nullptr, pingFrame);
		EncodeFrame(kOpcodeContinuation, "\"text\":\"hello\"}", 15U, true, false, nullptr, f2);
		decoder.AppendBytes(f1.data(), f1.size());
		decoder.AppendBytes(pingFrame.data(), pingFrame.size());
		decoder.AppendBytes(f2.data(), f2.size());

		if (!decoder.NextMessage(out, err) || out.opcode != kOpcodePing || out.payload != "hb")
		{
			std::fprintf(stderr, "Interleaved Ping frame failed\n");
			return false;
		}
		if (!decoder.NextMessage(out, err) || out.opcode != kOpcodeText ||
			out.payload != "{\"type\":\"asr\",\"text\":\"hello\"}")
		{
			std::fprintf(stderr, "Reassembled fragmented text failed: '%s'\n", out.payload.c_str());
			return false;
		}

		// 3. 64-bit length frame (> 65535 bytes)
		std::string largePayload(70000U, 'Z');
		largePayload[12345] = '\0';
		EncodeFrame(kOpcodeBinary, largePayload.data(), largePayload.size(), true, false, nullptr, wire);
		decoder.AppendBytes(wire.data(), wire.size());
		if (!decoder.NextMessage(out, err) || out.opcode != kOpcodeBinary || out.payload != largePayload)
		{
			std::fprintf(stderr, "70000-byte 64-bit length frame failed\n");
			return false;
		}

		return true;
	}

	bool TestAudioCaptureResampler()
	{
		using namespace ParaEngine;

		// 1. 16 kHz mono PCM16 passthrough
		{
			AudioCaptureResampler r;
			std::string raw(640U, '\0');
			for (size_t i = 0; i < 320U; ++i)
			{
				const int16_t s = static_cast<int16_t>((static_cast<int>(i) - 160) * 100);
				const uint16_t u = static_cast<uint16_t>(s);
				raw[i * 2U + 0U] = static_cast<char>(u & 0xFFU);
				raw[i * 2U + 1U] = static_cast<char>((u >> 8U) & 0xFFU);
			}
			std::string out;
			r.ProcessRawBytes(raw.data(), raw.size(), EAF_16BIT_MONO, 16000, 16000, out);
			if (out != raw)
			{
				std::fprintf(stderr, "16kHz mono passthrough mismatch\n");
				return false;
			}
		}

		// 2. 48 kHz stereo -> 16 kHz mono (exact 3:1 downsample + L/R average)
		{
			AudioCaptureResampler r;
			const int srcFrames = 4800; // 100 ms
			std::string raw(static_cast<size_t>(srcFrames) * 4U, '\0');
			for (int i = 0; i < srcFrames; ++i)
			{
				const int16_t left = 6000;
				const int16_t right = 2000; // mono average = 4000
				const uint16_t ul = static_cast<uint16_t>(left);
				const uint16_t ur = static_cast<uint16_t>(right);
				raw[static_cast<size_t>(i) * 4U + 0U] = static_cast<char>(ul & 0xFFU);
				raw[static_cast<size_t>(i) * 4U + 1U] = static_cast<char>((ul >> 8U) & 0xFFU);
				raw[static_cast<size_t>(i) * 4U + 2U] = static_cast<char>(ur & 0xFFU);
				raw[static_cast<size_t>(i) * 4U + 3U] = static_cast<char>((ur >> 8U) & 0xFFU);
			}
			std::string out;
			r.ProcessRawBytes(raw.data(), raw.size(), EAF_16BIT_STEREO, 48000, 16000, out);
			if (out.size() != 1600U * 2U)
			{
				std::fprintf(stderr, "48kHz stereo -> 16kHz mono expected 3200 bytes, got %zu\n", out.size());
				return false;
			}
			for (size_t i = 0; i < 1600U; ++i)
			{
				const uint16_t u = static_cast<uint16_t>(static_cast<uint8_t>(out[i * 2U])) |
					(static_cast<uint16_t>(static_cast<uint8_t>(out[i * 2U + 1U])) << 8U);
				if (static_cast<int16_t>(u) != 4000)
				{
					std::fprintf(stderr, "48kHz stereo average sample mismatch at %zu: %d\n", i, static_cast<int>(static_cast<int16_t>(u)));
					return false;
				}
			}
		}

		// 3. 44100 Hz -> 16000 Hz chunk-invariance (streaming 37-byte unaligned chunks == single buffer)
		{
			const size_t srcSamples = 44100U;
			std::string raw(srcSamples * 2U, '\0');
			for (size_t i = 0; i < srcSamples; ++i)
			{
				const double t = static_cast<double>(i) / 44100.0;
				const int16_t s = static_cast<int16_t>(std::round(12000.0 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * t)));
				const uint16_t u = static_cast<uint16_t>(s);
				raw[i * 2U + 0U] = static_cast<char>(u & 0xFFU);
				raw[i * 2U + 1U] = static_cast<char>((u >> 8U) & 0xFFU);
			}

			AudioCaptureResampler singleShot;
			std::string wholeOut;
			singleShot.ProcessRawBytes(raw.data(), raw.size(), EAF_16BIT_MONO, 44100, 16000, wholeOut);

			AudioCaptureResampler chunked;
			std::string streamedOut;
			size_t pos = 0U;
			while (pos < raw.size())
			{
				const size_t n = (std::min)(static_cast<size_t>(37U), raw.size() - pos);
				std::string part;
				chunked.ProcessRawBytes(raw.data() + pos, n, EAF_16BIT_MONO, 44100, 16000, part);
				streamedOut.append(part);
				pos += n;
			}

			if (wholeOut != streamedOut || wholeOut.size() != 16000U * 2U)
			{
				std::fprintf(
					stderr,
					"44.1kHz chunk-invariance failed: whole=%zu streamed=%zu\n",
					wholeOut.size(),
					streamedOut.size());
				return false;
			}
		}

		return true;
	}

	bool TestLoopbackSpeechWebSocket()
	{
		using namespace ParaEngine;
		using namespace ParaEngine::ParaWebSocketCodec;

#ifdef _WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

		TestSocket listenSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listenSock == kInvalidTestSocket)
		{
			std::fprintf(stderr, "Failed to create loopback listen socket\n");
			return false;
		}

		int yes = 1;
		::setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.sin_port = htons(0);
		if (::bind(listenSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
			::listen(listenSock, 1) != 0)
		{
			CloseTestSocket(listenSock);
			std::fprintf(stderr, "Failed to bind/listen loopback socket\n");
			return false;
		}

		socklen_t addrLen = sizeof(addr);
		::getsockname(listenSock, reinterpret_cast<sockaddr*>(&addr), &addrLen);
		const uint16_t port = ntohs(addr.sin_port);

		bool serverOk = false;
		std::thread serverThread([&]() {
			TestSocket clientSock = ::accept(listenSock, nullptr, nullptr);
			CloseTestSocket(listenSock);
			if (clientSock == kInvalidTestSocket)
				return;

			std::string req;
			char buf[4096];
			while (req.find("\r\n\r\n") == std::string::npos)
			{
				const int n = ::recv(clientSock, buf, sizeof(buf), 0);
				if (n <= 0)
				{
					CloseTestSocket(clientSock);
					return;
				}
				req.append(buf, static_cast<size_t>(n));
			}

			const std::string keyHeader = "Sec-WebSocket-Key: ";
			const size_t keyPos = req.find(keyHeader);
			if (keyPos == std::string::npos)
			{
				CloseTestSocket(clientSock);
				return;
			}
			const size_t keyEnd = req.find("\r\n", keyPos);
			const std::string key = req.substr(keyPos + keyHeader.size(), keyEnd - (keyPos + keyHeader.size()));
			const std::string acceptVal = ComputeAcceptKey(key);

			// Coalesce 101 response + immediate {"type":"ready"} frame in ONE send() to verify leftover byte preservation!
			std::string resp =
				"HTTP/1.1 101 Switching Protocols\r\n"
				"Upgrade: websocket\r\n"
				"Connection: Upgrade\r\n"
				"Sec-WebSocket-Accept: " + acceptVal + "\r\n\r\n";
			std::string readyFrame;
			const std::string readyJson = "{\"type\":\"ready\",\"asr_format\":\"pcm\",\"asr_rate\":16000}";
			EncodeFrame(kOpcodeText, readyJson.data(), readyJson.size(), true, false, nullptr, readyFrame);
			resp += readyFrame;
			::send(clientSock, resp.data(), static_cast<int>(resp.size()), 0);

			FrameStreamDecoder dec;
			size_t pcmBytesReceived = 0U;
			bool gotAsrEnd = false;

			while (!gotAsrEnd)
			{
				const int n = ::recv(clientSock, buf, sizeof(buf), 0);
				if (n <= 0)
					break;
				dec.AppendBytes(buf, static_cast<size_t>(n));

				DecodedFrame f;
				std::string protoErr;
				while (dec.NextMessage(f, protoErr))
				{
					if (f.opcode == kOpcodeBinary)
					{
						pcmBytesReceived += f.payload.size();
						const std::string partial =
							"{\"type\":\"asr\",\"event\":\"result\",\"text\":\"你好抱抱龙\"}";
						std::string pFrame;
						EncodeFrame(kOpcodeText, partial.data(), partial.size(), true, false, nullptr, pFrame);
						::send(clientSock, pFrame.data(), static_cast<int>(pFrame.size()), 0);
					}
					else if (f.opcode == kOpcodeText && f.payload == "{\"type\":\"asr_end\"}")
					{
						gotAsrEnd = true;
						const std::string doneJson =
							"{\"type\":\"asr\",\"event\":\"done\",\"text\":\"你好抱抱龙，帮我盖房子\"}";
						std::string dFrame;
						EncodeFrame(kOpcodeText, doneJson.data(), doneJson.size(), true, false, nullptr, dFrame);
						::send(clientSock, dFrame.data(), static_cast<int>(dFrame.size()), 0);
						break;
					}
				}
			}

			serverOk = (pcmBytesReceived == 3200U && gotAsrEnd);
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			CloseTestSocket(clientSock);
		});

		auto& mgr = ParaWebSocketClientManager::GetSingleton();
		const std::string url = "ws://127.0.0.1:" + std::to_string(port) + "/speech?asr_format=pcm&asr_rate=16000";
		const int handle = mgr.Connect(url);

		bool sawOpen = false;
		bool sawReady = false;
		bool sawPartial = false;
		bool sawDone = false;
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

		while (!sawDone && std::chrono::steady_clock::now() < deadline)
		{
			WebSocketEvent ev;
			while (mgr.PollEvent(handle, ev))
			{
				if (ev.type == "open")
				{
					sawOpen = true;
				}
				else if (ev.type == "text")
				{
					if (ev.data.find("\"type\":\"ready\"") != std::string::npos)
					{
						sawReady = true;
						std::string pcm(3200U, '\0');
						pcm[0] = '\x00';
						pcm[1] = '\x10';
						mgr.SendBinary(handle, pcm);
					}
					else if (ev.data.find("\"event\":\"result\"") != std::string::npos)
					{
						sawPartial = true;
						mgr.SendText(handle, "{\"type\":\"asr_end\"}");
					}
					else if (ev.data.find("\"event\":\"done\"") != std::string::npos)
					{
						sawDone = true;
					}
				}
				else if (ev.type == "error")
				{
					std::fprintf(stderr, "Loopback client error: %s\n", ev.data.c_str());
				}
			}
			if (!sawDone)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		}

		mgr.Close(handle);
		serverThread.join();

		if (!sawOpen || !sawReady || !sawPartial || !sawDone || !serverOk)
		{
			std::fprintf(
				stderr,
				"Loopback speech flow failed: open=%d ready=%d partial=%d done=%d serverOk=%d\n",
				sawOpen,
				sawReady,
				sawPartial,
				sawDone,
				serverOk);
			return false;
		}
		return true;
	}

	bool TestLiveKeepworkWss(const char* liveUrl)
	{
		using namespace ParaEngine;
		auto& mgr = ParaWebSocketClientManager::GetSingleton();
		const int handle = mgr.Connect(liveUrl);

		bool sawReady = false;
		bool sawDone = false;
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);

		while (!sawDone && std::chrono::steady_clock::now() < deadline)
		{
			WebSocketEvent ev;
			while (mgr.PollEvent(handle, ev))
			{
				if (ev.type == "text")
				{
					std::printf("[live wss] %s\n", ev.data.c_str());
					if (!sawReady && ev.data.find("\"type\":\"ready\"") != std::string::npos)
					{
						sawReady = true;
						// Send 0.5s of 16 kHz mono silence (8000 samples = 16000 bytes) in 6400-byte frames + asr_end
						std::string chunk1(6400U, '\0');
						std::string chunk2(6400U, '\0');
						std::string chunk3(3200U, '\0');
						mgr.SendBinary(handle, chunk1);
						mgr.SendBinary(handle, chunk2);
						mgr.SendBinary(handle, chunk3);
						mgr.SendText(handle, "{\"type\":\"asr_end\"}");
					}
					else if (ev.data.find("\"event\":\"done\"") != std::string::npos)
					{
						sawDone = true;
					}
				}
				else if (ev.type == "error")
				{
					std::fprintf(stderr, "[live wss error] %s\n", ev.data.c_str());
					mgr.Close(handle);
					return false;
				}
			}
			if (!sawDone)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		mgr.Close(handle);
		return sawReady && sawDone;
	}
}

int main(int argc, char** argv)
{
	if (!TestUrlAndAcceptKey())
		return 1;
	if (!TestFrameCodec())
		return 2;
	if (!TestAudioCaptureResampler())
		return 3;
	if (!TestLoopbackSpeechWebSocket())
		return 4;

	if (argc > 1 && std::strcmp(argv[1], "--live-keepwork") == 0)
	{
		const char* url = (argc > 2)
			? argv[2]
			: "wss://speechrtc.keepwork.com/speech?asr_format=pcm&asr_rate=16000";
		if (!TestLiveKeepworkWss(url))
			return 5;
	}

	std::printf("WebSocketAndAudioResampleTest PASSED\n");
	return 0;
}
