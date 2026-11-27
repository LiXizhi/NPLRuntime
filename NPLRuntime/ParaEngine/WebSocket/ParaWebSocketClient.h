#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace ParaEngine
{
	enum class WebSocketReadyState : int
	{
		Closed = 0,
		Connecting = 1,
		Open = 2,
		Closing = 3,
	};

	struct WebSocketEvent
	{
		std::string type; // "open", "text", "binary", "close", "error"
		std::string data;
		int code = 0;
	};

	struct ParsedWebSocketUrl
	{
		bool secure = false;
		std::string host;
		uint16_t port = 80;
		std::string pathAndQuery;
		std::string hostHeader;
		std::string curlConnectUrl;
	};

	namespace ParaWebSocketCodec
	{
		constexpr uint8_t kOpcodeContinuation = 0x0;
		constexpr uint8_t kOpcodeText = 0x1;
		constexpr uint8_t kOpcodeBinary = 0x2;
		constexpr uint8_t kOpcodeClose = 0x8;
		constexpr uint8_t kOpcodePing = 0x9;
		constexpr uint8_t kOpcodePong = 0xA;

		bool ParseUrl(const std::string& url, ParsedWebSocketUrl& out, std::string& outError);
		std::string Base64Encode(const uint8_t* data, size_t len);
		std::string ComputeAcceptKey(const std::string& clientKeyBase64);

		void EncodeFrame(
			uint8_t opcode,
			const void* payload,
			size_t payloadLen,
			bool fin,
			bool masked,
			const uint8_t maskKey[4],
			std::string& outFrame);

		struct DecodedFrame
		{
			uint8_t opcode = 0;
			bool fin = true;
			std::string payload;
		};

		class FrameStreamDecoder
		{
		public:
			explicit FrameStreamDecoder(size_t maxMessageBytes = 16 * 1024 * 1024);

			void AppendBytes(const char* bytes, size_t len);

			/**
			 * Pops next complete message or control frame.
			 * Data fragments (FIN=0 + continuation 0x0) are reassembled transparently
			 * while allowing interleaved control frames (Ping/Pong/Close) per RFC 6455.
			 * @return true if outFrame was populated; false if more bytes are needed or on protocol error.
			 */
			bool NextMessage(DecodedFrame& outFrame, std::string& outProtocolError);

			void Reset();

		private:
			size_t m_maxMessageBytes;
			std::string m_buffer;
			size_t m_readOffset = 0;
			bool m_inFragmentedMessage = false;
			uint8_t m_fragmentOpcode = 0;
			std::string m_fragmentBuffer;
		};
	}

	class ParaWebSocketClientManager
	{
	public:
		static ParaWebSocketClientManager& GetSingleton();

		int Connect(const std::string& url, const std::string& extraHeaders = "");
		bool SendText(int handle, const std::string& text);
		bool SendBinary(int handle, const std::string& binaryData);
		int GetState(int handle);
		bool PollEvent(int handle, WebSocketEvent& outEvent);
		void Close(int handle, int code = 1000, const std::string& reason = "");

	private:
		ParaWebSocketClientManager() = default;
		~ParaWebSocketClientManager() = default;
		ParaWebSocketClientManager(const ParaWebSocketClientManager&) = delete;
		ParaWebSocketClientManager& operator=(const ParaWebSocketClientManager&) = delete;
	};
}
